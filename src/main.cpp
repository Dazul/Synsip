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

#include <cstdlib>
#include <syslog.h>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <string.h>
#include <pthread.h> //for threading , link with lpthread
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>


#include "Network_manager.h"
#include "config.h"
#include "Message_manager.h"
#include "Synthesis_manager.h"
#include "Call_manager.h"
#include "Timeout_manager.h"
#include "Config_parser.h"

using namespace std;

Synthesis_manager *synthesis_manager;
Call_manager *call_manager;
Message_manager *message_manager;
Network_manager *network_manager;
Network_manager *network_manager_local;
Timeout_manager *timeout_manager;

void sigHandler(int sig) {
    delete network_manager;
    delete network_manager_local;
}

/**
 * Get configuration in configuration file
 * @param config
 */
void read_config(synsip_config *config) {
	//Example of hardcoded config. Not real one.
    config->listen_port = 7801;
    strcpy(config->registrar, "192.168.0.121");
    strcpy(config->user, "91");
    strcpy(config->password, "secret");
    config->max_calls = 4;
    strcpy(config->script_path, "/home/user/SynSipTest");
    strcpy(config->scriptfr_name, "annonce.sh");
    strcpy(config->scriptde_name, "annonce.sh");
    config->sip_port = 5061; // default port
}

/**
 * The first method call
 * 
 */
int main(int argc, char** argv) {
    int endValue = 0;
    cout << "start" << endl;

    openlog(NULL, 0, LOG_USER);
    syslog(LOG_INFO, "Program start");

    // Configuration
    synsip_config *config;
    config = (synsip_config*) malloc(sizeof (synsip_config));
    //read_config(config);
    Config_parser parser;
    parser.parse_config(*config, argv[1]);
    
    exit(0);



    syslog(LOG_INFO, "*** New instanace starting");

    // Create the synthesis manager
    synthesis_manager = new Synthesis_manager();
    if (!synthesis_manager->init(config)) {
        syslog(LOG_ERR, "Cannot initialize the synthesis manager");
        return EXIT_FAILURE;
    }


    // Create the call mananger
    call_manager = new Call_manager();
    if (!call_manager->init(config)) {
        syslog(LOG_ERR, "Cannot initialize Call manager");
        return EXIT_FAILURE;
    }

    // Create the timeout manager
    timeout_manager = new Timeout_manager(call_manager);
    timeout_manager->start();

    // Create the message manager
    message_manager = new Message_manager(synthesis_manager, call_manager);

    // Create the network manager from automate
    network_manager = new Network_manager(message_manager);

    if (!message_manager->init(config)) {
        syslog(LOG_ERR, "Cannot start message thread");
        return EXIT_FAILURE;
    }

    // if two network create
    if (network_manager->create_connection(config->listen_port)) {
		syslog(LOG_INFO, "Networks create");
    } else {
        sleep(2); // Wait for the other process
        delete network_manager;
        syslog(LOG_ERR, "Cannot create network connection");
        endValue = EXIT_FAILURE;
    }
    if (endValue == EXIT_FAILURE) {
        free(config);
        return endValue;
    }
    
    network_manager->join();

    syslog(LOG_INFO, "Program ended");
    free(config);


    closelog();
    return endValue;
}



