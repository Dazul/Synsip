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

#ifndef CONFIG_H
#define	CONFIG_H

typedef struct {
    char registrar[50]; // The address of the registrar (SIP telephone centrale)
    char user[30]; // The username
    char password[50]; // The userpassword
    int sip_port; // The SIP port
    int max_calls; // The number of maximum call
    char script_path[30]; // The directories script
    char scriptfr_name[30]; // The script's name to do French
    char scriptde_name[30]; // The script's name to de German
    int listen_port; // The port to connect from the automate
    int listen_port_local; // The port to connect from local client
    char bd_server[50];
    char bd_user[50];
    char bd_pass[50];
    char bd_db[50];
} synsip_config;

typedef struct {
    char gare[50];
} message_db;

#endif	/* CONFIG_H */

