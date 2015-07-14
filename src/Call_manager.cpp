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

#include "Call_manager.h"


#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <condition_variable>
#include <mutex>
#include <sys/syslog.h>
#include <errno.h>
#include <queue>
#include <iostream>
#include <map>
#include <vector>

#include <pjsua-lib/pjsua.h>
#include <unistd.h>
#include <algorithm>

#define TIME_WAIT 10
#define TIMEOUT 15 // 15 sec. to wait response
#define TIMEOUT_CONF 50 // 45 sec. to play audio file

#define LOG_LEVEL 0
#define MAX_SIM_CALL 4
#define MAX_SIZE 255

#define STAT_DISPO 0
#define STAT_MAKECALL 1
#define STAT_EARLY 2
#define STAT_CALLING 3
#define STAT_CONFIRME 4
#define STAT_MEDIA_ACTIVE 5
#define STAT_DISCONNECTED 6
#define STAT_TIMEOUT 10

using namespace std;

sem_t wait_call_status_broadcast;
sem_t wait_call_status_one;
sem_t wait_max_calls;
sem_t wait_destroy_player;
sem_t play_audio_file;

mutex mtx_ann_queue;
condition_variable cond_new_annonce;
queue<str_annonce> annonce_queue;


vector<mycall_info> mycall_info_tab(MAX_SIZE);

map<int, mycall_info> call_mstate; // "machine state"
int cid_number[MAX_SIZE]; // call_id <=> phone number
mutex mtx_state_access;

time_t now;

Call_manager::Call_manager() {
    sem_init(&wait_call_status_broadcast, 0, 0);
    sem_init(&wait_call_status_one, 0, 0);
    sem_init(&wait_destroy_player, 0, 0);
    sem_init(&play_audio_file, 0, 0);

    printf("Call_manager create\n");
}

int SplitNumber(vector<string>& vecteur, string chaine, char separateur) {
    vecteur.clear();

    string::size_type stTemp = chaine.find(separateur);

    while (stTemp != string::npos) {
        vecteur.push_back(chaine.substr(0, stTemp));
        chaine = chaine.substr(stTemp + 1);
        stTemp = chaine.find(separateur);
    }

    vecteur.push_back(chaine);

    return vecteur.size();
}

/* When incoming call*/
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata) {
    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &ci);

    /*answer*/
    pjsua_call_answer(call_id, 603, NULL, NULL); // reject incoming call (code 603)
    syslog(LOG_INFO, "Try to call this server form %s", ci.remote_contact);
}

void delete_file(char* file) {
    char commandefile[127];
    sprintf(commandefile, "rm -f %s", file); // if file already remove (broadcast) use -f
    if (system(commandefile) == -1) {

        syslog(LOG_ERR, "Cannot delete the audio file : %s", strerror(errno));
    }
}

void change_call_stat(pjsua_call_id call_id, int status) {
    now = time(0);
    mtx_state_access.lock();
    int number = cid_number[call_id % MAX_SIZE];
    mycall_info mci = call_mstate[number];
    mycall_info newmci = mci;
    switch (status) {
        case STAT_EARLY:
            newmci.call_status = STAT_EARLY; // update status
            newmci.timeout = now + TIMEOUT; // update timeout
            break;
        case STAT_CONFIRME:
            newmci.call_status = STAT_CONFIRME; // update status
            newmci.timeout = now + TIMEOUT_CONF; // update timeout
            break;
        case STAT_DISCONNECTED:
            newmci.call_status = STAT_DISPO;
            printf("Status befor disconnected : %d\n", mci.call_status);
            sem_post(&wait_max_calls);
            int semVal;
        	sem_getvalue(&wait_max_calls, &semVal);
            if (mci.call_status < STAT_CONFIRME || mci.call_status == STAT_TIMEOUT) {
                char err[50];
                syslog(LOG_INFO, "Cannot wait connection");
            }
            break;
    }
    call_mstate.at(number) = newmci;
    mtx_state_access.unlock();
}

/**
 * call when media status change
 * @param call_id
 */
void change_media_state(pjsua_call_id call_id) {
    now = time(0);
    mtx_state_access.lock();
    int number = cid_number[call_id % MAX_SIZE];
    mycall_info mci = call_mstate[number];
    mycall_info newmci = mci;
    newmci.call_status = STAT_MEDIA_ACTIVE; // update status
    newmci.timeout = now + TIMEOUT_CONF;
    mci.hp_manager->play_audio_file(call_id, mci.audio_file);

    call_mstate.at(number) = newmci;
    mtx_state_access.unlock();
}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e) {
    pjsua_call_info ci;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &ci);

    syslog(LOG_INFO, "HP %s state change : %s\n", call_mstate[cid_number[call_id]].number, ci.state_text.ptr);


    if (ci.state == PJSIP_INV_STATE_EARLY) {
        change_call_stat(call_id, STAT_EARLY);
    } else if (ci.state == PJSIP_INV_STATE_CONFIRMED) {
        change_call_stat(call_id, STAT_CONFIRME);
    } else if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        change_call_stat(call_id, STAT_DISCONNECTED);
    }

}

static void on_call_media_state(pjsua_call_id call_id) {
    pjsua_call_info ci;
    pjsua_call_get_info(call_id, &ci);
    pjsua_conf_connect(ci.conf_slot, 0);
    pjsua_conf_connect(0, ci.conf_slot);

    syslog(LOG_INFO, "Call id %d call media state change : %s\n", call_id, ci.state_text.ptr);

    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
        change_media_state(call_id);
    } else if (ci.media_status == PJSUA_CALL_MEDIA_ERROR) {
        syslog(LOG_ERR, "Media error");
    }
}

bool Call_manager::init(synsip_config *config) {
    this->config = config;
	sem_init(&wait_max_calls, 0, config->max_calls);
    if (this->start() == -1) {

        return false;
    }

    return true;
}

bool Call_manager::wait() {
    if (this->join() == -1) {

        return false;
    }
    return true;
}

bool Call_manager::make_call(str_annonce annonce) {
    unique_lock<mutex> mlock(mtx_ann_queue);
    annonce_queue.push(annonce);
    mlock.unlock();
    cond_new_annonce.notify_one();

    return true;
}

void* Call_manager::run() {
    while (true) {
        printf("init call manager\n");
        pjsua_acc_id acc_id;
        pj_status_t status;

        /* create pjusa */
        status = pjsua_create();
        if (status != PJ_SUCCESS) {
            syslog(LOG_ERR, "Error in pjsua_create() : %d", status);
            return NULL;
        }
        /* Init pjsua */
        {
            pjsua_config cfg;
            pjsua_logging_config log_cfg;

            pjsua_config_default(&cfg);
            cfg.cb.on_incoming_call = &on_incoming_call;
            cfg.cb.on_call_media_state = &on_call_media_state;
            cfg.cb.on_call_state = &on_call_state;
            cfg.thread_cnt = 4;

            pjsua_logging_config_default(&log_cfg);
            log_cfg.console_level = LOG_LEVEL;

            status = pjsua_init(&cfg, &log_cfg, NULL);
            if (status != PJ_SUCCESS) {
                syslog(LOG_ERR, "Error pjsua_init() : %d", status);
                return NULL;
            }
        }
        /* Add UPD transport */
        {
            pjsua_transport_config cfg;
            pjsua_transport_config_default(&cfg);
            cfg.port = config->sip_port;
            status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
            if (status != PJ_SUCCESS) {
                syslog(LOG_ERR, "Error creating pjsua transport : %d", status);
                return NULL;
            }
        }
        /* Null audio device */
        status = pjsua_set_null_snd_dev();
        if (status != PJ_SUCCESS) {
            syslog(LOG_ERR, "Error set null sound device : %d", status);
            return NULL;
        }
        /* Initialization is done, now start pjsua */
        status = pjsua_start();
        if (status != PJ_SUCCESS) {
            syslog(LOG_ERR, "Error starting pjsua : %d", status);
            return NULL;
        }

        /* Register to SIP server by creating SIP account */
        {
            pjsua_acc_config cfg;

            pjsua_acc_config_default(&cfg);

            char id[80];
            char reg[80];
            memset(id, 0, 80);
            memset(reg, 0, 80);
            strcat(id, "sip:");
            strcat(reg, "sip:");
            strcat(id, config->user);
            strcat(id, "@");
            strcat(id, config->registrar); //strcat(id, config->registrar);
            strcat(reg, config->registrar);

            cfg.id = pj_str(id);
            cfg.reg_uri = pj_str(reg);
            cfg.cred_count = 1;
            cfg.cred_info[0].realm = pj_str((char*) "*");
            //cfg.cred_info[0].scheme = pj_str((char *)"digest");
            cfg.cred_info[0].username = pj_str(config->user);
            cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
            cfg.cred_info[0].data = pj_str(config->password);

            status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
            if (status != PJ_SUCCESS) {
                syslog(LOG_ERR, "Error adding account : %d", status);
                return NULL;
            } else {
                syslog(LOG_INFO, "Account create, id : %d", acc_id);
            }
        }

        syslog(LOG_INFO, "Pjsua account create, account id : %d\n", acc_id);

        sleep(1); // wait account registered


        /**
         * Do really the annonce
         * @return 
         */
        while (true) {
            sleep(1);
            unique_lock<mutex> mlock(mtx_ann_queue);
            while (annonce_queue.empty()) {
                cond_new_annonce.wait(mlock);
            }
            syslog(LOG_INFO, "New annonce incoming");
            str_annonce annonce = annonce_queue.front();
            annonce_queue.pop();
            mlock.unlock();
            
            printf("********************Check Registration\n");
            if(pjsua_acc_is_valid(acc_id) == 0){
            	pjsua_acc_set_registration(acc_id, 1); // verify if account is register
            }
            printf("********************Registration checked\n");

            manage_individual_call(annonce, acc_id);
        }
    }

    sleep(1);
    pjsua_destroy();
}

/**
 * 
 * @param annonce
 * @param acc_id
 */
void Call_manager::manage_individual_call(str_annonce annonce, pjsua_acc_id acc_id) {

	syslog(LOG_INFO, "Making new call");
    now = time(0);
    char *number = annonce.phone_number;
    mycall_info mci;
    mci.hp_manager = new HP_manager(acc_id, this->config);
    strcpy(mci.audio_file, annonce.audio_file);
    mci.bd_id = annonce.bd_id;
    mci.call_status = STAT_MAKECALL;
    mci.original_annonce = annonce;
    mci.timeout = now + TIMEOUT;
    strcpy(mci.number, number);

    // if no state exist for this number
    int n = atoi(number);
    mtx_state_access.lock();
    if (call_mstate.find(n) == call_mstate.end()) {
    	syslog(LOG_INFO, "Number not in call_mstate %d\n", n);
        printf("Number not in call_mstate %d\n", n);
        pjsua_call_id call_id = mci.hp_manager->call(mci.number);
        if (call_id == -1) {
            syslog(LOG_INFO, "Error HP");

        } else {
            mci.call_id = call_id;
            call_mstate[n] = mci;
            cid_number[call_id % MAX_SIZE] = n;
        }
    } else {
    	syslog(LOG_INFO, "Number already in call_mstate %d\n", n);
        printf("Number already in call_mstate %d\n", n);
        mycall_info old_mci = call_mstate[n];
		delete old_mci.hp_manager;
        if (old_mci.call_status == STAT_DISPO) {
            pjsua_call_id call_id = mci.hp_manager->call(mci.number);
            if (call_id == -1) {
                syslog(LOG_INFO, "Error HP, call id = -1.");
            } else {
                mci.call_id = call_id;
                call_mstate.at(n) = mci;
                cid_number[call_id % MAX_SIZE] = n;
            }
        } else {
            // can wait here before retry call the hp
            syslog(LOG_INFO, "Error HP, STATE is not dispo.");
        }
    }
    mtx_state_access.unlock();
}

void Call_manager::timeout(int t_now) {
    mtx_state_access.lock();
    for (auto& x : call_mstate) {
        if (x.second.call_status == STAT_MAKECALL || x.second.call_status == STAT_CALLING || x.second.call_status == STAT_EARLY || x.second.call_status == STAT_CONFIRME || x.second.call_status == STAT_MEDIA_ACTIVE) { // timeout
            if (x.second.timeout <= t_now) {
                syslog(LOG_INFO, "Timeout %d, %d\n", x.second.timeout, t_now);
                if (end_a_call(x.second.call_id)) {
                    printf("Hangup timeout\n");
                } else {
                    printf("Cannot hangup timeout\n");
                }
            }
        }

    }
    mtx_state_access.unlock();
}

/**
 * muss only call by lock mtx_state_access method timeout
 * @param call_id
 */
bool Call_manager::end_a_call(pjsua_call_id call_id) {
	syslog(LOG_INFO, "Ending call\n");
    int number = cid_number[call_id % MAX_SIZE];
    mycall_info mci = call_mstate[number];
    mci.call_status = STAT_TIMEOUT;
    call_mstate.at(number) = mci;
    mci.hp_manager->hangup_hp(call_id);
    //sem_post(&wait_max_calls);
    return true;
}

Call_manager::~Call_manager() {
    printf("Destroy\n");
    pjsua_destroy();
}

