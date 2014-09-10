/*
* Synsip, automated calling machine working with text to speech synthesis
* 
* Copyright (C) 2014  EIA-FR (https://eia-fr.ch/)
* author: Fabien Yerly
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

#ifndef NETWORK_H
#define	NETWORK_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <syslog.h>

#include "Thread.h"
#include "Message_manager.h"

class Network_manager : public Thread{
public:
    Network_manager(int file, synsip_config config);

    virtual ~Network_manager();
    void* run();

    bool createConnection();
    bool waitMessage();
    bool closeConnection();

    bool canReceivce;

private:
    int socket_desc, client_sock, c;
    struct sockaddr_in server, client;
    int port;
    synsip_config config;
    Message_manager *message_manager;
};

#endif	/* NETWORK_H */

