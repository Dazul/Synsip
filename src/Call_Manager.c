/*
* Synsip, automated calling machine working with text to speech synthesis
*
* Copyright (C) 2014  Luis Domingues
* 
* This file is part of Synsip.
* 
* Synsip is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
* 
* Synsip is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with Synsip.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Call_Manager.h"
#include "queue.h"

#include <pjsua-lib/pjsua.h>
#include <pjmedia.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#define THIS_FILE	"Call_Manager"

#define SIP_DOMAIN	"192.168.1.190"
#define SIP_USER	"91"
#define SIP_PASSWD	"secret"
#define BUFFER_SIZE 255

struct pjsua_player_eof_data
{
    pj_pool_t       *pool;
   	pjsua_call_id 	call_id;
    pjsua_player_id player_id;
};

FILE *stream;
sem_t wait_start_call;
sem_t wait_destroy_player;
pthread_t destroy_thread;

/* Util to display the error message for the specified error code  */
static int app_perror( const char *sender, const char *title, 
		       pj_status_t status)
{
    char errmsg[PJ_ERR_MSG_SIZE];

    pj_strerror(status, errmsg, sizeof(errmsg));

    PJ_LOG(2,(sender, "%s: %s [code=%d]", title, errmsg, status));
    return 1;
}

/* Function who destroy the players, in another thread. */
static void *destroy_players(void *args)
{
	void *msg;
	pj_thread_t *pj_thread;
	pj_thread_desc desc;
	pj_status_t status;
	status = pj_thread_register("Destroy thread", desc, &pj_thread);
	if(status != PJ_SUCCESS){
		app_perror(THIS_FILE, "Unable to register thread", status);
	}
	while(1)
	{
		sem_wait(&wait_destroy_player);
		msg = remove_element();
    	struct pjsua_player_eof_data *eof_data = (struct pjsua_player_eof_data *)msg;
		status = pjsua_call_hangup(eof_data->call_id, 0, NULL, NULL);
    	status = pjsua_player_destroy(eof_data->player_id);
    	pj_pool_release(eof_data->pool);
    	free(msg);
	}
}

/* Put the player_id on a queue to be destroy by another thread */
static PJ_DEF(pj_status_t) on_pjsua_wav_file_end_callback(pjmedia_port* media_port, void* args)
{
	pj_status_t status;
	struct pjsua_player_eof_data *arg_queue = malloc(sizeof(struct pjsua_player_eof_data));
	struct pjsua_player_eof_data *eof_data = (struct pjsua_player_eof_data *)args;
	*arg_queue = *eof_data;
	add_element(arg_queue);
	sem_post(&wait_destroy_player);
    return -1;
}

/* Play the file after the call */
static int play_file(pjsua_call_id call_id,  char *msgFile)
{
    if(msgFile == NULL){
        return -1;
    }
    pj_str_t file;
    file.ptr = msgFile;
    file.slen = strlen(msgFile);

    pjsua_player_id player_id;
    pj_status_t status;
    status = pjsua_player_create(&file, PJMEDIA_FILE_NO_LOOP, &player_id);
    if (status != PJ_SUCCESS){
        app_perror(THIS_FILE, "Unable to create", status);
        return status;
    }

    pjmedia_port *player_media_port;

    status = pjsua_player_get_port(player_id, &player_media_port);
    if (status != PJ_SUCCESS)
    {
        app_perror(THIS_FILE, "Unable to get port", status);
        return status;
    }

    pj_pool_t *pool = pjsua_pool_create("my_eof_data", 512, 512);
    struct pjsua_player_eof_data *eof_data = PJ_POOL_ZALLOC_T(pool, struct pjsua_player_eof_data);
    eof_data->pool = pool;
    eof_data->player_id = player_id;
	eof_data->call_id = call_id;
	
    pjmedia_wav_player_set_eof_cb(player_media_port, eof_data, &on_pjsua_wav_file_end_callback);

    status = pjsua_conf_connect(pjsua_player_get_conf_port(player_id), pjsua_call_get_conf_port(call_id));

    pjsua_player_set_pos(player_id, 0);

    if (status != PJ_SUCCESS)
    {
        app_perror(THIS_FILE, "Unable to conect", status);
        return status;
    }        

    return status;
}

/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
			     pjsip_rx_data *rdata)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &ci);

    PJ_LOG(1,(THIS_FILE, "Incoming call from %.*s!!",
			 (int)ci.remote_info.slen,
			 ci.remote_info.ptr));
    pjsua_conf_port_id conf_port = pjsua_call_get_conf_port(call_id);

    /* Automatically answer incoming calls with 200/OK */
    pjsua_call_answer(call_id, 200, NULL, NULL);
    
    play_file(call_id, NULL);

}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &ci);
    PJ_LOG(1,(THIS_FILE, "Call %d state=%.*s", call_id,
			 (int)ci.state_text.slen,
			 ci.state_text.ptr));
}

/* Callback called by the library when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);
    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
		// When media is active, release the player for the file.
		sem_post(&wait_start_call);
    }
}

/* Display error and exit application */
static void error_exit(const char *title, pj_status_t status)
{
    pjsua_perror(THIS_FILE, title, status);
    pjsua_destroy();
    exit(1);
}

/* Make a call */
void make_call(char *msgFile, char *num)
{
    struct timespec timeout;
    pjsua_call_id call_id;
    pj_str_t uri_arg;
    
    uri_arg.ptr = num;
    uri_arg.slen = strlen(num);

    pjsua_call_make_call(pjsua_acc_get_default(), &uri_arg, NULL, NULL, NULL, &call_id);
    
    pjsua_call_info ci;

    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 20;
    timeout.tv_nsec = 0;
    sem_timedwait(&wait_start_call, &timeout);
    if(errno == ETIMEDOUT){
    	PJ_LOG(1,(THIS_FILE, "Time out!"));
    	pjsua_call_hangup(call_id, 0, NULL, NULL);
    	return;
    }
    sleep(1);

    play_file(call_id, msgFile);
}

int init_call_manager(int file)
{
    pjsua_acc_id acc_id;
    pj_status_t status;

    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status);

    /* Init pjsua */
    {
	    pjsua_config cfg;
	    pjsua_logging_config log_cfg;

	    pjsua_config_default(&cfg);
	    cfg.cb.on_incoming_call = &on_incoming_call;
	    cfg.cb.on_call_media_state = &on_call_media_state;
	    cfg.cb.on_call_state = &on_call_state;
	    cfg.thread_cnt = 2;

	    pjsua_logging_config_default(&log_cfg);
	    log_cfg.console_level = 1;

	    status = pjsua_init(&cfg, &log_cfg, NULL);
	    if (status != PJ_SUCCESS) error_exit("Error in pjsua_init()", status);
    }

    /* Add UDP transport. */
    {
	    pjsua_transport_config cfg;

	    pjsua_transport_config_default(&cfg);
	    cfg.port = 5060;
	    status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
	    if (status != PJ_SUCCESS) error_exit("Error creating transport", status);
    }
    
    status = pjsua_set_null_snd_dev();
    if (status != PJ_SUCCESS){
         error_exit("Error creating null device", status);
    }

    /* Initialization is done, now start pjsua */
    status = pjsua_start();
    if (status != PJ_SUCCESS) error_exit("Error starting pjsua", status);

    /* Register to SIP server by creating SIP account. */
    {
	    pjsua_acc_config cfg;

	    pjsua_acc_config_default(&cfg);
	    cfg.id = pj_str("sip:" SIP_USER "@" SIP_DOMAIN);
	    cfg.reg_uri = pj_str("sip:" SIP_DOMAIN);
	    cfg.cred_count = 1;
	    cfg.cred_info[0].realm = pj_str("*");
	    //cfg.cred_info[0].scheme = pj_str("*");
	    cfg.cred_info[0].username = pj_str(SIP_USER);
	    cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
	    cfg.cred_info[0].data = pj_str(SIP_PASSWD);

	    status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
	    if (status != PJ_SUCCESS) error_exit("Error adding account", status);
    }
    //sleep(5);
    //make_call("sip:81@192.168.0.2");
    init_queue(50);
    sem_init(&wait_start_call, 0, 0);
    sem_init(&wait_destroy_player, 0, 0);
    pthread_create(&destroy_thread, NULL, &destroy_players, NULL);
    /* Wait until user press "q" to quit. */
    stream = fdopen (file, "r");
    char bufMsg[BUFFER_SIZE];
    char bufNum[BUFFER_SIZE];
    for (;;) {
        char c;
        int i = 0;
        int isNum = 0;
        while((c=getc(stream)) != '\n'){
            if(c == '|'){
                isNum = 1;
                bufMsg[i] = '\0';
                i = 0;
                continue;
            }
            if(isNum){
                bufNum[i++] = c;
            } else {
                bufMsg[i++] = c;
            }
            if((i+1) == BUFFER_SIZE){
                printf("Buffer size exceded\n");
                break;
            }
        }
        bufNum[i] = '\0';
        if(bufMsg[0] == 'q'){
            printf("Receive a q\n");
            break;
        }
        /*printf("*********************************************************\n");
        printf("Phone number: %s\n", bufNum);
        printf("Message: %s\n", bufMsg);*/
//        continue;
        make_call(bufMsg, bufNum);
    }
    pthread_cancel(destroy_thread);
    destroy_queue();
    fclose (stream);
    pjsua_destroy();
    return 0;
}
