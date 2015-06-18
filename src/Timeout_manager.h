/* 
 * File:   Timeout_manager.h
 * Author: yerlyf
 *
 * Created on October 7, 2014, 4:18 PM
 */

#pragma once

#include "Call_manager.h"
#include "Thread.h"

class Timeout_manager : public Thread {
public:
    Timeout_manager(Call_manager *call_manager);

    virtual ~Timeout_manager();
private:
    Call_manager *call_manager;
    void *run();

};
/* TIMEOUT_MANAGER_H */

