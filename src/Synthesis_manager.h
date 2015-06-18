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
#pragma once

#define lang_fr 0
#define lang_de 1
#define lang_en 2

#include "config.h"


class Synthesis_manager {
public:
    Synthesis_manager();
    
    bool init(synsip_config* config);
    int synthesired(const char* annonce, int language);
    
    virtual ~Synthesis_manager();
private:
    
    synsip_config* config;

};

/* SYNTHESIS_MANAGER_H */

