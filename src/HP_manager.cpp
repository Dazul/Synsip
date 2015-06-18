/*
* Synsip, automated calling machine working with text to speech synthesis
* 
* Copyright (C) 2014  EIA-FR (https://eia-fr.ch/)
* author: Fabien Yerly
* 
* Copyright (C) 2014-2015  Luis Domingues
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

#include <semaphore.h>
#include <arpa/nameser_compat.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <iostream>
#include "HP_manager.h"
#include "Call_manager.h"
#include "config.h"

sem_t wait_file_end;
sem_t wait_to_play_file;

HP_manager::HP_manager(pjsua_acc_id acc_id, synsip_config *config) {
	this->config = config;
    this->acc_id = acc_id;

    sem_init(&wait_file_end, 0, 0);
    sem_init(&wait_to_play_file, 0, 0);
    printf("HP_manager create\n");

}

pjsua_call_id HP_manager::call(char* num) {
    printf("HP_manager call %s\n", num);

    pj_status_t status;
    pjsua_call_id call_id = -1;
    pj_str_t uri_arg;
    char num_uri[30];
	memset(num_uri, 0, 30);
    strcat(num_uri, "sip:");
    strcat(num_uri, num);
    strcat(num_uri, "@");
    strcat(num_uri, config->registrar);
    uri_arg.ptr = num_uri;
    uri_arg.slen = strlen(num_uri);
    // make the call

    status = pjsua_call_make_call(acc_id, &uri_arg, 0, NULL, NULL, &call_id);

    if (status != PJ_SUCCESS) {
        char buffer[255];
        pj_str_t error = pjsip_strerror(status, buffer, 255);
        printf("HP %s, does not answer. %s\n", num, buffer);
        return -1;
    }
    return call_id;
}

static PJ_DEF(pj_status_t) on_pjsua_wav_file_end_callback(pjmedia_port* media_port, void* args) {
    pj_status_t status;
    struct pjsua_player_eof_data *eof_data = (struct pjsua_player_eof_data *) args;
    status = pjsua_call_hangup(eof_data->call_id, 0, NULL, NULL);
    status = pjsua_player_destroy(eof_data->player_id);
    pj_pool_release(eof_data->pool);

    printf("call id %d, end play file\n", eof_data->call_id);
    return 1;
}

/* Play the file */
int HP_manager::play_file(pjsua_call_id call_id, char* audfile) { //
    //printf("play_file name : %s\n", audfile);
    if (audfile == NULL) {
        return -1;
    }
    pj_str_t file;
    char audfile_full[255];
	memset(audfile_full, 0, 255);
    strcat(audfile_full, config->script_path);
    strcat(audfile_full, audfile);
    file.ptr = audfile_full;
    file.slen = strlen(audfile_full);

    pjsua_player_id player_id;
    pj_status_t status;
    status = pjsua_player_create(&file, PJMEDIA_FILE_NO_LOOP, &player_id);
    if (status != PJ_SUCCESS) {
    	char buffer[255];
        pj_str_t error = pjsip_strerror(status, buffer, 255);
        printf("Unable to create. %s\n", buffer);
        return status;
    }

    pjmedia_port *player_media_port;

    status = pjsua_player_get_port(player_id, &player_media_port);
    if (status != PJ_SUCCESS) {
        printf("Unable to get port");
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

    if (status != PJ_SUCCESS) {
        printf("Unable to connect");
        return status;
    }

    return status;
}

bool HP_manager::play_audio_file(pjsua_call_id call_id, char* audio_file) {
    int status = play_file(call_id, audio_file);
    printf("HP_manager, Play audio file on id %d, file %s, status %d\n", call_id, audio_file, status);
    return true;
}

bool HP_manager::hangup_hp(pjsua_call_id call_id) {
    pj_thread_t *thread;
    pj_thread_desc desc;
    if (!pj_thread_is_registered()) {
        pj_thread_register(NULL, desc, &thread);
    }
    pj_status_t status = pjsua_call_hangup(call_id, 0, NULL, NULL);
    pj_thread_destroy(thread);
    if (status != PJ_SUCCESS) {
        syslog(LOG_ERR, "Cannot hangup call %d", call_id);
        return false;
    }
    return true;
}

bool HP_manager::hangup_hp_all() {
    pjsua_call_hangup_all();
    return true;
}

void *HP_manager::run() {
    bool r = true;
    while (r) {
        //play_file(audio_file);

        r = false;
    }
}

HP_manager::~HP_manager() {
}

