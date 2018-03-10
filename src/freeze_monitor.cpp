/* 
 * File:   Timeout_manager.cpp
 * Author: yerlyf
 * 
 * Created on October 7, 2014, 4:18 PM
 */

#include <unistd.h>

#include "freeze_monitor.h"
#include <stdlib.h>
#include <syslog.h>
#define TIMESTEP 60

Freeze_Monitor::Freeze_Monitor() {
}

void *Freeze_Monitor::run() {
    sleep(TIMESTEP);
    //Dangerouse, but If we are here, the process freezed.
    exit(EXIT_FAILURE);
    //syslog(LOG_INFO, "Process died.");
}

Freeze_Monitor::~Freeze_Monitor() {
}

