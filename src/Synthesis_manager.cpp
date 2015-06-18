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

#include <sys/syslog.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <arpa/nameser_compat.h>
#include "Synthesis_manager.h"


Synthesis_manager::Synthesis_manager() {
}

bool Synthesis_manager::init(synsip_config* config) {
    this->config = config;
    return true;
}

/**
 * 
 * @param annonce
 * @param language 0 French, 1 German
 * @return -1 if error
 */
int Synthesis_manager::synthesired(const char* annonce, int language) {
    int fileName = rand() % 100 + 1;
    // text file
    char commandefile[sizeof (annonce) + 256];
    sprintf(commandefile, "echo \"%s\" > %s/%d", annonce, config->script_path, fileName);
    if (system(commandefile) == -1) {
        syslog(LOG_ERR, "Cannot create the annonce file, error cmd system : %s", strerror(errno));
        return -1;
    }

    char commandescript[256];
    if (language == lang_fr) {
        sprintf(commandescript, "%s/%s %d %s", config->script_path, config->scriptfr_name, fileName, config->script_path);
    }
    else if (language = lang_de) {
        sprintf(commandescript, "%s/%s %d %s", config->script_path, config->scriptde_name, fileName, config->script_path);
    }
    else{
        syslog(LOG_ERR, "Cannot make synthesis script, no language");
        return -1;
    }
    
    if(system(commandescript) == -1){
        syslog(LOG_ERR, "Cannot make the audio file");
        return -1;
    }
    usleep(1000);
    
    return fileName;
}

Synthesis_manager::~Synthesis_manager() {
}

