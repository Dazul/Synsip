/*
* Synsip, automated calling machine working with text to speech synthesis
* 
* Copyright (C) 2014  EIA-FR (https://eia-fr.ch/)
* author: Fabien Yerly
* 
* Copyright (C) 2014  Luis Domingues
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
* along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Message_manager.h"

using namespace std;

Message_manager::Message_manager(int file) {
    this->file = file;
    stream = fdopen (file, "w");
}

void Message_manager::generateMessage(int size, char* msg) {

    //char msg[] = "1;20;une;Amtrak;chablais-city;12 heure 34;20.06.2014-10:00;message";

    cout << "Create new message" << endl << endl;

    char* token;
    token = strtok(msg, "|");

    char* annonce[8]; // 0 type, 1 number, 2 diffusion, 3 message, 4 voie, 5 train, 6 gare, 7 depart
    int i = 0;
    do {
        annonce[i] = token;
        printf("-%s\n", token);
        token = strtok(NULL, "|");
        i++;
    } while(token != NULL);
// Genereate message depending on the type
    char message[256];

    if (strcmp(annonce[0], "1") == 0) { // message type 1
        cout << "Type 1" << endl;
        sprintf(message, "%s", annonce[3]);
    } 
    else if (strcmp(annonce[0], "2") == 0) { // message type 2
        sprintf(message, "Voie, %s, Entrée du train, %s. Ce train dessert %s.", annonce[4], annonce[5], annonce[6]);
        if (strcmp(annonce[7], "_") != 0) {
            sprintf(message, "%s Départ %s", message, annonce[7]);
        }
    } else if (strcmp(annonce[0], "3") == 0) { // message type 3
        sprintf(message, "Attention, sur voie %s.", annonce[4]);
        if (strcmp(annonce[5], "_") != 0) {
            sprintf(message, "%s Entrée du train %s. %s.", message, annonce[5], annonce[3]);
        } else {
            sprintf(message, "%s %s", message, annonce[3]);
        }
    }
    else {
        sprintf(message, "_");
    }

    //}
    //sprintf(message, "Pas de message");

    cout << "Message: " << message << endl;
    
    int fileName = synthesis_manager.synthesired(message);
    
    cout << "fileName : " << fileName << endl;   
    cout << "call number : " << annonce[1] << endl;
    stringstream ss;
    ss << fileName;
    string file = ss.str();
    char msgChar[255];
    strcpy(msgChar,"/home/synsip/");
    strcat(msgChar,file.c_str());
    strcat(msgChar,".wav");
    strcat(msgChar,"|sip:");
    strcat(msgChar, annonce[1]);
    strcat(msgChar,"@192.168.1.190\n\0");
    fprintf (stream, "%s", msgChar);
    fflush(stream);
}

Message_manager::~Message_manager() {
    cout << "Deleting Message Manager" << endl;
    fprintf (stream, "q\n");
    fflush(stream);
    fclose(stream);
}

