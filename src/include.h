/*
* Synsip, automated calling machine working with text to speech synthesis
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

#ifndef SYNSIP_INCLUDE_H
#define	SYNSIP_INCLUDE_H

typedef struct {
    char registrar[50];
    char user[30];
    char password[50];
    int max_calls;
    char script_path[30];
    char script_name[30];
    int listen_port;
    int sip_port;
}synsip_config;

#endif
