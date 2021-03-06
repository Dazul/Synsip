/*
* Synsip, automated calling machine working with text to speech synthesis
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

#include "Thread.h"
#include "config.h"
#include "Message_manager.h"

class Files_manager : public Thread {
public:
    Files_manager(Message_manager *message_manager, synsip_config* config);
    virtual ~Files_manager();
	bool start_read_reqs();

private:
    void *run();
    void transfer_message(string message);
    synsip_config* config;

    char request_files_path[50];
    
    Message_manager *message_manager;
};

/* FILES_MANAGER_H */
