/* 
 * File:   main.cpp
 * Author: Fabien Yerly
 *
 * Created on September 20, 2014, 9:03 PM
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
    config->listen_port_local = 6800;
    strcpy(config->registrar, "192.168.1.127");
    strcpy(config->user, "91");
    strcpy(config->password, "secret");
    config->max_calls = 4;
    strcpy(config->script_path, "/home/synsip");
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
    read_config(config);



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
    // Create the  network manager from local
    network_manager_local = new Network_manager(message_manager);

    if (!message_manager->init(config)) {
        syslog(LOG_ERR, "Cannot start message thread");
        return EXIT_FAILURE;
    }

    // if two network create
    if (network_manager->create_connection(config->listen_port)) {
        if (network_manager_local->create_connection(config->listen_port_local)) {
            if (!network_manager->wait_connection()) {
                syslog(LOG_INFO, "Cannot wait connection");
            }

            if (!network_manager_local->wait_connection()) {
                syslog(LOG_INFO, "Cannot wait connection local");
            }
        } else {
            sleep(2); // Wait for the other process
            delete network_manager_local;
            delete network_manager;
            syslog(LOG_ERR, "Cannot create network connection");
            endValue = EXIT_FAILURE;
        }
    } else {
        sleep(2); // Wait for the other process
        delete network_manager_local;
        delete network_manager;
        syslog(LOG_ERR, "Cannot create network connection");
        endValue = EXIT_FAILURE;
    }
    if (endValue == EXIT_FAILURE) {
        free(config);
        return endValue;
    }
    syslog(LOG_INFO, "Networks create");

    network_manager->join();

    syslog(LOG_INFO, "Program ended");
    free(config);


    closelog();
    return endValue;
}



