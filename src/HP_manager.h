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

#ifndef HP_MANAGER_H
#define	HP_MANAGER_H

#include <pjsua-lib/pjsua.h>
#include "Thread.h"

struct pjsua_player_eof_data {
    pj_pool_t *pool;
    pjsua_call_id call_id;
    pjsua_player_id player_id;
};

class HP_manager : public Thread {
public:
    HP_manager(pjsua_acc_id acc_id);
    pjsua_call_id call(char* number);
    bool play_audio_file(pjsua_call_id call_id, char* audio_file);
    
    bool hangup_hp(pjsua_call_id call_id);
    bool hangup_hp_all();
    virtual ~HP_manager();
    
private:
    pjsua_acc_id acc_id;
    int play_file(pjsua_call_id call_id, char* audfile);
    void *run();
};

#endif	/* HP_MANAGER_H */
