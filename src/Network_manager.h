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

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#include "Thread.h"
#include "config.h"
#include "Message_manager.h"

class Network_manager : public Thread {
public:
    Network_manager(Message_manager *message_manager);
    virtual ~Network_manager();

    bool create_connection(int port);
    bool wait_connection();

private:
    void *run();
    void transfer_message(char message[]);

    int server_socket, client_socket, sockadd_size;
    struct sockaddr_in server, client;
    
    Message_manager *message_manager;
};

/* NETWORK_MANAGER_H */
