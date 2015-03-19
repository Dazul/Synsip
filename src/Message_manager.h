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

#ifndef MESSAGE_MANAGER_H
#define	MESSAGE_MANAGER_H

#include "Thread.h"
#include "config.h"
#include <vector>
#include <string>

#include "Synthesis_manager.h"
#include "Database_manager.h"
#include "Call_manager.h"

class Message_manager : public Thread {
public:
    Message_manager(Synthesis_manager *synthesis_manager, Database_manager *database_manager, Call_manager *call_manager);
    
    bool init(synsip_config *config);
    void analyse_message(char *message);
    
    virtual ~Message_manager();
private:
    void *run();
    bool generate_annonce(char *message);
    synsip_config *config;
    
    Synthesis_manager *synthesis_manager;
    Database_manager *database_manager;
    Call_manager *call_manager;

    int id;
    str_message str_msg;
 
};

#endif	/* MESSAGE_MANAGER_H */

