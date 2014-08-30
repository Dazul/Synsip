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
* along with Synsip.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Synthesis_manager.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <arpa/nameser_compat.h>

using namespace std;

Synthesis_manager::Synthesis_manager() {
    cout << "Create synthesize" << endl;
    srand (time(NULL));
}

int Synthesis_manager::synthesired(char* annonce) { // message=0x7ffff6fd4d90 \"message test\\r\\n\"
    cout << annonce << endl;
    int fileName = rand() % 100 + 1;;
    char ordner[] = "/home/synsip";

    // Text file
    char commandefile[sizeof (annonce) + 256];
    sprintf(commandefile, "echo \"%s\" > /%s/%d", annonce, ordner, fileName); // generate command
    cout << commandefile << endl;
    system(commandefile); // execute command


    char script[256];
    sprintf(script, "%s/annonce.sh %d %s", ordner, fileName, ordner);


    cout << "commande : " << script << endl;

    system(script);
    //system("echo message > text.txt");
    usleep(1000);
    /*
    char *playCommande = new char[14];
    sprintf(playCommande, "aplay %d.wav", fileName);
    system(playCommande);
     */
    return fileName;
}

Synthesis_manager::~Synthesis_manager() {
}

