#!/bin/bash

echo $(cat $2/$1)
espeak -a 100 -p 50 -s 120 -v mb/mb-fr4 "$(cat $2/$1)" --pho --phonout=$2/$1.pho
mbrola /usr/share/mbrola/voices/fr4 $2/$1.pho $2/$1.wav
