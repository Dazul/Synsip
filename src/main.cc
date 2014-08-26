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
* along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include "Network_manager.h"

using namespace std;

extern "C"{
    int init_call_manager(int file);
    void call_manager_destroy();
}

Network_manager *network;

void sigHandler(int sig){
    printf("Interupt Signal received Parent\n");
    delete network;
}

int main(int argc, char** argv) {
    cout << "Start program" << endl;
    pid_t pid;
    int mypipe[2];

    if (pipe (mypipe)){
      fprintf (stderr, "Pipe failed.\n");
      return EXIT_FAILURE;
    }
    
    pid = fork();
    if(pid == 0)
    {
    //    Child
        close (mypipe[1]);
        signal(SIGINT, SIG_IGN);
        init_call_manager(mypipe[0]);
    } else {
        // create the network class
        signal(SIGINT, sigHandler);
        close (mypipe[0]);
        network = new Network_manager(mypipe[1]);

        // create the connection stream
        if (network->createConnection()) {

            if (!network->waitMessage()) {
                cout << "Waiting message error" << endl;
            }
            /*
            string msg = "Voie, une, entrée du train, Amtrak. Ce train dessert, Chablais-City, la Prairie. Départ 12 heure 34.";

            char *message = new char[msg.size() + 1];
            copy(msg.begin(), msg.end(), message);

            synthesize.synthesired(message);

            */
            //if (network.closeConnection()) cout << "Connection close" << endl;
        } else {
            cout << "Cannot create connection" << endl;
        }
        int returnStatus;
        waitpid(pid, &returnStatus, 0);
        cout << "Child end status: " << returnStatus << endl;
    }
    return 0;
}


