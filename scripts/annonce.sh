#!/bin/bash

###########################################################################
# Copyright (C) 2014  Luis Domingues
# 
# This file is part of Synsip.
# 
# Synsip is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
# 
# Synsip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Synsip.  If not, see <http://www.gnu.org/licenses/>.
#
###########################################################################
#
# Modifie this script to change the text to speech engine.
#
###########################################################################

espeak -a 20 -p 50 -s 120 -v mb/mb-fr4 "$(cat $2/$1)" --pho --phonout=$2/$1.pho 2> /dev/null
mbrola /usr/share/mbrola/voices/fr4 $2/$1.pho $2/$1.voice.wav 2> /dev/null
sox $2/intro.wav $2/$1.voice.wav $2/$1.wav 2> /dev/null
