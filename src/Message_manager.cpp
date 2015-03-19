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

queue<char*> msg_queue;
mutex mymutex;
condition_variable mycond;

Message_manager::Message_manager(Synthesis_manager *synthesis_manager, Database_manager *database_manager, Call_manager *call_manager) {
    this->synthesis_manager = synthesis_manager;
    this->database_manager = database_manager;
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

void Message_manager::analyse_message(char *message) {
    // convert to queue char
    char new_message[512];
    sprintf(new_message, "%s", message);

    unique_lock<mutex> mlock(mymutex);
    msg_queue.push(new_message);
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
bool Message_manager::generate_annonce(char message[]) {
    // get current time
    time_t t = time(0);
    struct tm *now = localtime(&t);

    char receive_time[10];
    sprintf(receive_time, "%d:%d:%d", now->tm_hour, now->tm_min, now->tm_sec);
    char receive_date[12];
    sprintf(receive_date, "%d.%d.%d", now->tm_mday, (now->tm_mon + 1), (now->tm_year + 1900)); // month start at 0

    string msg = message; // convert to string
    const char* annonce[9]; // 0 type, 1 number, 2 diffusion, 3 message, 4 voie, 5 train, 6 gare, 7 depart, 8 language

    id = -1;
    str_msg.receivetime = receive_time;
    str_msg.receivedate = receive_date;
    printf("Time %s, date %s\n", receive_time, receive_date);

    vector<string> VecStr;
    int nbTabl = Split(VecStr, msg, '|'); // split the annonce
    if (nbTabl != 10) { // 10 size of message standard, 11 size of message delayed
        if (nbTabl != 11) {
            syslog(LOG_ERR, "Cannot read message : syntax error");
            // Store message error in Database
            str_msg.status = ERR_MESSAGE;
            str_msg.mesage = (char*) msg.c_str();

            id = database_manager->savemessage(str_msg);
            if (id == -1) {
                syslog(LOG_INFO, "Message error no store in database");
            }
            return false;
        }
        id = atoi(VecStr[9].c_str()); // Get the id
    }

    for (int i = 0; i < 9; ++i) {
        annonce[i] = VecStr[i].c_str();

    }

    // Create message for Database
    str_msg.type = (char*) annonce[0];
    str_msg.hp = (char*) annonce[1];
    str_msg.diffusion = (char*) annonce[2];
    str_msg.mesage = (char*) annonce[3];
    str_msg.voie = (char*) annonce[4];
    str_msg.train = (char*) annonce[5];
    str_msg.gare = (char*) annonce[6];
    str_msg.depart = (char*) annonce[7];
    str_msg.langue = (char*) annonce[8];

    // Check if is a delayed message
    if (strcmp(annonce[2], "") != 0) {
        // Store in database
        str_msg.status = EN_ATTENTE;
        id = database_manager->savemessage(str_msg);
        if (id == -1) {
            syslog(LOG_INFO, "Message no store in database");
        }

        vector<string> datetime;
        string dt = annonce[2];
        //split date
        int size = Split(datetime, dt, '-');
        string date = datetime[0];
        vector<string> d;
        size = Split(d, date, '.');
        string day = d[0];
        string month = d[1];
        string year = d[2];
        string time = datetime[1];
        char commande[300];
        char newmessage[256];
        sprintf(newmessage, "%s|%s||%s|%s|%s|%s|%s|%s|%d|", annonce[0], annonce[1], annonce[3], annonce[4], annonce[5], annonce[6], annonce[7], annonce[8], id);
        sprintf(commande, "echo \"%s '%s' %d\" | at %s %s/%s/%s", "send", newmessage, config->listen_port_local, time.c_str(), month.c_str(), day.c_str(), year.c_str());
        cout << commande << endl;
        if (system(commande) == -1) {
            syslog(LOG_ERR, "Cannot execute delay commande : %d", errno);
        }

        return true;
    }

    // Store in database or update message already store
    if (id != -1) { // update
        if (!database_manager->updatemessage(EN_ATTENTE, id)) {
            syslog(LOG_INFO, "Message no update in database");
        }
    } else {
        str_msg.status = EN_ATTENTE;
        id = database_manager->savemessage(str_msg);
        if (id == -1) {
            syslog(LOG_INFO, "Message no store in database");
        }
    }

    // Generate the advertisement depending the type
    // Check language
    int language = -1; // No language
    if (strcmp(annonce[8], "FR") == 0) {
        language = 0;
    } else if (strcmp(annonce[8], "DE") == 0) {
        language = 1;
    } else {
        syslog(LOG_ERR, "Language not know");
        return false;
    }


    // Generate the advertisement
    char advertisement[512];
    if (strcmp(annonce[0], "1") == 0) { // Type 1, personnel message
        cout << "Type 1" << endl;
        sprintf(advertisement, "%s", annonce[3]);

    } else if (strcmp(annonce[0], "2") == 0) { // message type 2, standard advertisement
        if (language == 0) {
            sprintf(advertisement, "Voie, %s, Entrée du train, %s. Ce train dessert %s.", annonce[4], annonce[5], annonce[6]);
        } else if (language == 1) {
            sprintf(advertisement, "Gleis, %s, Einfahrt des Zuges, %s. Der Zug fährt nach %s.", annonce[4], annonce[5], annonce[6]);
        }
        if (strcmp(annonce[7], "") != 0) {
            if (language == 0) {
                sprintf(advertisement, "%s Départ %s", advertisement, annonce[7]);
            } else if (language == 1) {
                sprintf(advertisement, "%s Abfahrt um %s", advertisement, annonce[7]);
            }
        }
    } else if (strcmp(annonce[0], "3") == 0) { // message type 3, secure advertisement
        if (language == 0) {
            sprintf(advertisement, "Attention, sur voie %s.", annonce[4]);
        } else if (language == 1) {
            sprintf(advertisement, "Achtung, auf Gleis %s.", annonce[4]);
        }
        if (strcmp(annonce[5], "") != 0) {
            if (language == 0) {
                sprintf(advertisement, "%s Entrée du train %s. %s.", advertisement, annonce[5], annonce[3]);
            } else if (language == 1) {
                sprintf(advertisement, "%s Einfahrt des Zuges %s. %s.", advertisement, annonce[5], annonce[3]);
            }
        } else {
            sprintf(advertisement, "%s %s", advertisement, annonce[3]);

        }
    } else {
        syslog(LOG_INFO, "Message type %s no exist", annonce[0]);
        return false;
    }

    int fileName = synthesis_manager->synthesired(advertisement, language);
    if (fileName == -1) {
        return false;
    }

    syslog(LOG_INFO, "Message: %s, call %s", advertisement, annonce[1]);
    str_annonce strann;
    char audiofile[10];
    sprintf(audiofile, "%d.wav", fileName);
    printf("audio file : %s\n", audiofile);
    strcpy(strann.audio_file, audiofile);
    strcpy(strann.phone_number, annonce[1]);
    strann.bd_id = id;

    if (call_manager->make_call(strann)) {
        syslog(LOG_INFO, "Advertisement send");
    } else {
        syslog(LOG_INFO, "Error send advertisement");
    }


    // update in database
    if (id != -1) {
        if (!database_manager->updatemessage(EN_ATTENTE, id)) {
            syslog(LOG_INFO, "Message no update in database");
        }
    }
    
    return true;
}

Message_manager::~Message_manager() {
}

