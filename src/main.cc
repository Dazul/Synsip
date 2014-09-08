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

#include <iostream>
#include <ctype.h>
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
#include <syslog.h>
#include "Network_manager.h"

using namespace std;

extern "C"{
    int init_call_manager(int file);
}

Network_manager *network;

void sigHandler(int sig){
    delete network;
}

void print_license() {
	cout << "Synsip 0.1.0\n"
"Copyright (C) 2014  EIA-FR (https://eia-fr.ch/)\n"
"Copyright (C) 2014  Luis Domingues\n"
"This is free software; see the source for copying conditions.  There is NO\n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"Written by Fabien Yerly and Luis Domingues\n";
}

int main(int argc, char** argv) {
	
	int c;
	while ((c = getopt (argc, argv, "v")) != -1)
	{
		switch (c) {
			case 'v':
				print_license();
				return 0;
				break;
			default:
				continue;
		}
	}
	
    //cout << "Start program" << endl;
    //TODO Debug
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
        openlog(NULL, LOG_PID, LOG_USER);
        syslog(LOG_INFO, "***** New instance starting");
        network = new Network_manager(mypipe[1]);

        // create the connection stream
        if (network->createConnection()) {

            if (!network->waitMessage()) {
            	//TODO Debug
                //cout << "Waiting message error" << endl;
            }
        } else {
        	//TODO Debug
            //cout << "Cannot create connection" << endl;
        }
        int returnStatus;
        waitpid(pid, &returnStatus, 0);
        //TODO Debug
        //cout << "Child end status: " << returnStatus << endl;
        syslog(LOG_INFO, "Application ended");
        closelog();
    }
    return 0;
}


