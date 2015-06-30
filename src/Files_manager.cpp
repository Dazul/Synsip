/*
* Synsip, automated calling machine working with text to speech synthesis
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

#include "Files_manager.h"
#include <syslog.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

#define MSG_SIZE 512 // the max message size (1024 char)

Files_manager::Files_manager(Message_manager *message_manager, synsip_config* config) {
	cout << "Files_manager create" << endl;
	this->message_manager = message_manager;
	this-> config = config;
}

/**
 * Start and join the thread to read the reqs
 * @return true if start and join
 *         false else
 */
bool Files_manager::start_read_reqs() {
	if (this->start() < 0) {
		return false;
	}
	cout << "Files_manager wait connection" << endl;
	return true;
	}

/**
 * The thread. Read the reqs.
 * @return 
 */
void* Files_manager::run() {
	ifstream fin;
	string filepath;
	DIR *dp;
	struct dirent *dirp;
	struct stat filestat;
	char* req_path = config->request_path;
	string msg;
	char c_msg[MSG_SIZE];
	memset(c_msg, 0, MSG_SIZE);
	while (true) {
		dp = opendir(req_path);
		if (dp == NULL)
		{
			syslog(LOG_ERR, "Error( %s ) opening %s", strerror(errno), req_path);
			return NULL;
		}
	
		while ((dirp = readdir( dp )))
		{
			filepath = string(req_path) + "/" + dirp->d_name;
			// If the file is a directory (or is in some way invalid) we'll skip it 
			if (stat( filepath.c_str(), &filestat )) continue;
			if (S_ISDIR( filestat.st_mode )) continue;
	
			fin.open(filepath.c_str());
			getline(fin, msg);
			strncpy(c_msg, msg.c_str(), msg.length());
			transfer_message(msg.c_str());
			memset(c_msg, 0, MSG_SIZE);
			// Clear the message buffer
			fin.close();
			if(remove(filepath.c_str()) != 0)
			{
				syslog(LOG_ERR, "Error( %s ) deleting %s", strerror(errno), filepath.c_str());
			}
		}
		
		closedir(dp);
	
		sleep(config->wait_read_files);
	}
}

void Files_manager::transfer_message(const char* message) {
	message_manager->analyse_message(message);
}

Files_manager::~Files_manager() {
	syslog(LOG_INFO, "Files closed");
}

