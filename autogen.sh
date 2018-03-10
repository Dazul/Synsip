#!/bin/bash
echo 'aclocal'
aclocal
echo 'autoconf'
autoreconf -f
echo 'automake'
automake
