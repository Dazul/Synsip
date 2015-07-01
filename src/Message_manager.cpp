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
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/syslog.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sstream>

#include "config.h"
#include "Message_manager.h"



using namespace std;

queue<string> msg_queue;
mutex mymutex;
condition_variable mycond;

Message_manager::Message_manager(Synthesis_manager *synthesis_manager, Call_manager *call_manager) {
    this->synthesis_manager = synthesis_manager;
    this->call_manager = call_manager;

    cout << "Message_manager create" << endl;
}

bool Message_manager::init(synsip_config* config) {

    this->config = config;

    if (this->start() == -1) {
        return false;
    }

    return true;
}

void Message_manager::analyse_message(string message) {
    // convert to queue char
    unique_lock<mutex> mlock(mymutex);
    msg_queue.push(message);
    mlock.unlock();
    mycond.notify_one();
}

void* Message_manager::run() {

    while (true) {
        unique_lock<mutex> mlock(mymutex);
        while (msg_queue.empty()) {
            mycond.wait(mlock);
        }
        syslog(LOG_INFO, "Receive message");
        if (!generate_annonce(msg_queue.front())) {
            syslog(LOG_INFO, "No advertisement made");
        }
        msg_queue.pop();
        mlock.unlock();
    }
}

/**
 * Split the receive message
 * @param vecteur
 * @param chaine
 * @param separateur
 * @return 
 */
int Split(vector<string>& vecteur, string chaine, char separateur) {
    vecteur.clear();
    string::size_type stTemp = chaine.find(separateur);

    while (stTemp != string::npos) {
        vecteur.push_back(chaine.substr(0, stTemp));
        chaine = chaine.substr(stTemp + 1);
        stTemp = chaine.find(separateur);
    }

    vecteur.push_back(chaine);

    return vecteur.size();
}

/**
 * Create the annonce with the received message
 * @param message
 */
bool Message_manager::generate_annonce(string msg) {
    // get current time
    time_t t = time(0);
    struct tm *now = localtime(&t);
    char receive_time[10];
    sprintf(receive_time, "%d:%d:%d", now->tm_hour, now->tm_min, now->tm_sec);
    char receive_date[12];
    sprintf(receive_date, "%d.%d.%d", now->tm_mday, (now->tm_mon + 1), (now->tm_year + 1900)); // month start at 0

    const char* annonce[4]; // 0 number, 1 message lang 1, 2 message lang 2

    id = -1;
    printf("Time %s, date %s\n", receive_time, receive_date);

    vector<string> VecStr;
    int nbTabl = Split(VecStr, msg, '|'); // split the annonce
    if (nbTabl != 4) { // Message Error
        syslog(LOG_ERR, "Cannot read message : syntax error");
        id = atoi(VecStr[3].c_str()); // Get the id
    }

    for (int i = 0; i < 3; ++i) {
        annonce[i] = VecStr[i].c_str();
    }

    int fileName = synthesis_manager->synthesired(annonce[1], annonce[2]);
    if (fileName == -1) {
        return false;
    }

    syslog(LOG_INFO, "Message: %s:%s, call %s", annonce[1], annonce[2], annonce[0]);
    str_annonce strann;
    char audiofile[10];
    sprintf(audiofile, "%d.wav", fileName);
    printf("audio file : %s\n", audiofile);
    strcpy(strann.audio_file, audiofile);
    strcpy(strann.phone_number, annonce[0]);
    strann.bd_id = id;

    if (call_manager->make_call(strann)) {
        syslog(LOG_INFO, "Advertisement send");
    } else {
        syslog(LOG_INFO, "Error send advertisement");
    }
    
    return true;
}

Message_manager::~Message_manager() {
}

