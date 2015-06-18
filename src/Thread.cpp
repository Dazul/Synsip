/*
* Synsip, automated calling machine working with text to speech synthesis
* 
* Copyright (C) 2014  EIA-FR (https://eia-fr.ch/)
* author: Fabien Yerly
* inspired by: http://vichargrave.com/java-style-thread-class-in-c/
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

#include "Thread.h"

using namespace std;

Thread::Thread() : m_tid(0), m_running(0), m_detached(0) {
}

Thread::~Thread() {
    if (m_running == 1 && m_detached == 0) {
        pthread_detach(m_tid);
    }
    if (m_running == 1) {
        pthread_cancel(m_tid);
    }
}

static void* runThread(void* arg);

int Thread::start() {
    int result=-1;
    result = pthread_create(&m_tid, NULL, runThread, this);
    if (result == 0) {
        m_running = 1;
    }
    return result;
}

static void* runThread(void* arg) {
    return ((Thread*) arg)->run();
}

int Thread::join() {
    int result = -1;
    if (m_running == 1) {
        result = pthread_join(m_tid, NULL);
        if (result == 0) {
            m_detached = 1;
        }
    }
    return result;
}

int Thread::detach() {
    int result = -1;
    if(m_running == 1 && m_detached == 0){
        result = pthread_detach(m_tid);
        if(result == 0){
            m_detached = 1;
        }
    }
    return result;
}

pthread_t Thread::self(){
    return m_tid;
}


