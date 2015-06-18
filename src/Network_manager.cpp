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

#include "Network_manager.h"
#include <syslog.h>
#include <errno.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

#define MSG_SIZE 512 // the max message size (512 char)

Network_manager::Network_manager(Message_manager *message_manager) {
    cout << "Network_manager create" << endl;
    this->message_manager = message_manager;
}

/**
 * Create the network connection
 * @return true if socket and bind ready
 *         false else
 */
bool Network_manager::create_connection(int port) {
    // Create the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        syslog(LOG_ERR, "Server socket error : %s", strerror(errno));
        return false;
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    //Bind the socket
    if (bind(server_socket, (struct sockaddr *) &server, sizeof (server)) < 0) {
        //Print the error message
        syslog(LOG_ERR, "Bind error : %s", strerror(errno));
        return false;
    }

    return true;
}

/**
 * Start and join the thread to wait a connection
 * @return true if start and join
 *         false else
 */
bool Network_manager::wait_connection() {
    if (this->start() < 0) {
        return false;
    }
    cout << "Network_manager wait connection" << endl;
    return true;
}

/**
 * The thread. Listen on connection, accept connection
 * @return 
 */
void* Network_manager::run() {
    while (true) {
        // Listen of connection
        int result = listen(server_socket, 1); // 3 connection (one for automate, one for the web interface and one for the delayed message)
        if (result == -1) {
            syslog(LOG_ERR, "Listen error : %s", strerror(errno));
            return NULL;
        }

        sockadd_size = sizeof (struct sockaddr_in);
        // Accept the incoming connection
        client_socket = accept(server_socket, (struct sockaddr *) &client, (socklen_t*) & sockadd_size);
        if (client_socket == -1) {
            syslog(LOG_ERR, "Client socket error : %s", strerror(errno));
            return NULL;
        }
        syslog(LOG_INFO, "Client connected : %s", inet_ntoa(client.sin_addr));

        int read_size;
        char message[MSG_SIZE]; // message
        while (true) {
            read_size = read(client_socket, message, sizeof (message));
            if (read_size > 0) {
                // Transfert message
                this->transfer_message(message);
                // Clear the message buffer
                memset(message, 0, MSG_SIZE);
            }
            if (read_size == 0) {
                syslog(LOG_INFO, "Client disconnected : %s", inet_ntoa(client.sin_addr));
                close(client_socket);
                this->detach(); // close this thread
                this->wait_connection(); // wait new connection
            }
            if (read_size == -1) {
                syslog(LOG_ERR, "Client message error : %s", strerror(errno));
            }
        }
    }
    return NULL;

}

void Network_manager::transfer_message(char* message) {
    message_manager->analyse_message(message);
}

Network_manager::~Network_manager() {
    syslog(LOG_ERR, "Network closed");
}

