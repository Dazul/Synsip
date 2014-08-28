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

#include "queue.h"
#include <stdlib.h>

void **my_array;
int queue_size;
int used_size;
int next_put;
int next_get;

void init_queue(int size)
{
	queue_size = size;
	next_put = 0;
	next_get = 0;
	used_size = 0;
	my_array = malloc(sizeof(void*) * size);
}

void destroy_queue()
{
	free(my_array);
}

int is_full()
{
	if(used_size == queue_size){
		return 1;
	}
	return 0;
}

int is_empty()
{
	if(used_size == 0){
		return 1;
	}
	return 0;
}

void add_element(void* elm)
{
	used_size++;
	my_array[next_put] = elm;
	next_put = (next_put + 1) % queue_size;
}

void* remove_element()
{
	void* elm;
	used_size--;
	elm = my_array[next_get];
	next_get = (next_get + 1) % queue_size;
	return elm;
}
