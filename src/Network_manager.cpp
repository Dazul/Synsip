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

#include "Network_manager.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

void* Network_manager::run() {
    
    while(true){
    	//Listen
    	listen(socket_desc, 3);
	
    	//Accept and incoming connection
    	syslog(LOG_INFO, "Waiting for incoming connections...");
    	c = sizeof (struct sockaddr_in);

		client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t*) & c);
		syslog(LOG_INFO, "Connection accepted from %s", inet_ntoa(client.sin_addr));

		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( thread_id , NULL);

		//puts(inet_ntoa(client.sin_addr));


		if (client_sock < 0) {
		    syslog(LOG_ERR, "accept failed");
		    return NULL;
		}

		char returnMessage[] = {"Connection to server ready\n"};
		write(client_sock, returnMessage, strlen(returnMessage));

		int read_size;
		char automate_message[256];
		memset(automate_message, 0, 256);
		//Receive a message from client
		while (true) {

		    read_size = read(client_sock, automate_message, sizeof(automate_message));
		    if (read_size > 0) {
		        //call the message generator
		        message_manager->generateMessage(read_size, automate_message);
				/*cout << "Received message: " << automate_message << endl;
				if(strcmp(automate_message, "exit") == 0){
					cout << "Detach" << endl;
		        	this->detach();
		    	}*/
			
		        //clear the message buffer
		        memset(automate_message, 0, 256);
		    }
		    if (read_size == 0) {
    			syslog(LOG_INFO, "Client disconnected");
		        break;
		    } else if (read_size == -1) {
		        //puts("No message");
		        return NULL;
		    }

		}
	}
    return NULL;
}

Network_manager::Network_manager(int file, synsip_config config) {
    //TODO Debug
    //cout << "Network create" << endl;
    port = 7801;
    canReceivce = true;
    message_manager = new Message_manager(file);
}


bool Network_manager::createConnection() {
    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        //printf("Could not create socket");
    }
    //TODO Debug
    //puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    //Bind
    if (bind(socket_desc, (struct sockaddr *) &server, sizeof (server)) < 0) {
        //print the error message
        syslog(LOG_ERR, "bind failed. Error");
        return false;
    }
    //TODO Debug
    //puts("bind done");

    return true;
}

bool Network_manager::waitMessage() {
    this->start();
    return true;

}

bool Network_manager::closeConnection() {
    close(socket_desc);
    close(client_sock);
    close(c);
    return true;
}

Network_manager::~Network_manager() {
    syslog(LOG_ERR, "Deleting Network Manager");
    closeConnection();
    delete message_manager;
}

