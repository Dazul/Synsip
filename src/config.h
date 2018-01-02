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

typedef struct {
    char registrar[50]; // The address of the registrar (SIP telephone centrale)
    char call_target[50]; // The address of the registrar to call (in case of use of sip trunk)
    char user[30]; // The username
    char password[50]; // The userpassword
    int sip_port; // The SIP port
    unsigned int max_calls; // The number of maximum call
    char script_path[30]; // The directories script
    char script_name[30]; // The script's name
    char input_path[30]; // The input files dir path
    int listen_port; // The port to connect from the automate
    char request_path[50]; //Path to requests trought files.
    int wait_read_files; //Time in secondes between the read of request files.
} synsip_config;

typedef struct {
    char gare[50];
} message_db;

/* CONFIG_H */

