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

#ifndef CREATEMESSAGE_H
#define	CREATEMESSAGE_H

#include "Synthesis_manager.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <syslog.h>
#include "include.h"

class Message_manager {
public:
    Message_manager(int file, synsip_config *config);
    
    void generateMessage(int size, char* msg);

    virtual ~Message_manager();
private:
    Synthesis_manager *synthesis_manager;
    int file;
    synsip_config *config;
    FILE* stream;
};

#endif	/* CREATEMESSAGE_H */

