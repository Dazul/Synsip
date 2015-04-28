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
			cout << config.registrar << endl;
		} else if (line.find("call_target") != -1) {
			sin >> config.call_target;
			cout << config.call_target << endl;
		} else if (line.find("user") != -1) {
			sin >> config.user;
			cout << config.user << endl;
		} else if (line.find("password") != -1){
			sin >> config.password;
			cout << config.password << endl;
		} else if (line.find("sip_port") != -1){
			sin >> config.sip_port;
			cout << config.sip_port << endl;
		} else if (line.find("max_calls") != -1){
			sin >> config.max_calls;
			cout << config.max_calls << endl;
		} else if (line.find("script_path") != -1){
			sin >> config.script_path;
			cout << config.script_path << endl;
		} else if (line.find("scriptfr_name") != -1){
			sin >> config.scriptfr_name;
			cout << config.scriptfr_name << endl;
		} else if (line.find("scriptde_name") != -1){
			sin >> config.scriptde_name;
			cout << config.scriptde_name << endl;
		} else if (line.find("listen_port") != -1){
			sin >> config.listen_port;
			cout << config.listen_port << endl;
		}
	}
}

Config_parser::~Config_parser(){}
