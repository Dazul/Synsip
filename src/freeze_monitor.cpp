/* 
 * File:   Timeout_manager.cpp
 * Author: yerlyf
 * 
 * Created on October 7, 2014, 4:18 PM
 */

#include <unistd.h>

#include "freeze_monitor.h"

#define TIMESTEP 60

Freeze_Monitor::Freeze_Monitor() {
}

void *Freeze_Monitor::run() {
    sleep(TIMESTEP);
    //Dangerouse, but If we are here, the process freezed.
    exit(-1);
}

Freeze_Monitor::~Freeze_Monitor() {
}

