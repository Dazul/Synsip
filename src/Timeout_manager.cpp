/* 
 * File:   Timeout_manager.cpp
 * Author: yerlyf
 * 
 * Created on October 7, 2014, 4:18 PM
 */

#include <unistd.h>

#include "Timeout_manager.h"
#include "Call_manager.h"

#define TIMESTEP 1

Timeout_manager::Timeout_manager(Call_manager *call_manager) {
    this->call_manager = call_manager;
}

void *Timeout_manager::run() {
    time_t now;
    while(true){
        sleep(TIMESTEP);
        now = time(0);
        call_manager->timeout(now);
    }
}

Timeout_manager::~Timeout_manager() {
}

