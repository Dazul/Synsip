/* 
 * File:   Timeout_manager.h
 * Author: yerlyf
 *
 * Created on October 7, 2014, 4:18 PM
 */

#pragma once

#include "Thread.h"

class Freeze_Monitor : public Thread {
public:
    Freeze_Monitor();

    virtual ~Freeze_Monitor();
private:
    void *run();
};
/* TIMEOUT_MANAGER_H */

