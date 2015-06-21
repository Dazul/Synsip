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
#include <fstream>
#include <sstream>
#include "Config_parser.h"

using namespace std;

Config_parser::Config_parser(){}

int Config_parser::parse_config(synsip_config& config, string config_file){
	ifstream fin(config_file);
	string line;
	while (getline(fin, line)) {
		istringstream sin(line.substr(line.find("=") + 1));
		if (line.find("registrar") != -1){
			sin >> config.registrar;
		} else if (line.find("sip_user") != -1) {
			sin >> config.user;
		} else if (line.find("password") != -1){
			sin >> config.password;
		} else if (line.find("sip_port") != -1){
			sin >> config.sip_port;
		} else if (line.find("max_calls") != -1){
			sin >> config.max_calls;
		} else if (line.find("script_path") != -1){
			sin >> config.script_path;
		} else if (line.find("script_name") != -1){
			sin >> config.script_name;
		} else if (line.find("listen_port") != -1){
			sin >> config.listen_port;
		}
	}
}

Config_parser::~Config_parser(){}
