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

#pragma once

#include <array>
#include <pjsua-lib/pjsua.h>

#include "config.h"
#include "Thread.h"
#include "HP_manager.h"

/*
struct pjsua_player_eof_data {
    pj_pool_t *pool;
    pjsua_call_id call_id;
    pjsua_player_id player_id;
};
*/

struct str_annonce{
    char phone_number[120];
    char audio_file[10];
    int bd_id;
};

struct mycall_info{
    pjsua_call_id call_id;
    int call_status;
    char audio_file[10];
    bool broadast;
    int bd_id;
    char number[30];
    pjsua_call_id other_call_id[15]; // Max broadcast call
    int nbr_of_other_call_id;
    str_annonce original_annonce;
    int timeout;
    HP_manager *hp_manager;
};

class Call_manager : public Thread {
public:
    Call_manager();
    
    bool init(synsip_config *config); 
    bool wait();
    bool make_call(str_annonce);
    void timeout(int t_now); // time now
    
    virtual ~Call_manager();
private:
    void *run();
    synsip_config *config;
    bool end_a_call(pjsua_call_id call_id);
    void manage_individual_call(str_annonce annonce, pjsua_acc_id);
    
};

/* CALL_MANAGER_H */

