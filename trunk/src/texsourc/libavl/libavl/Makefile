#  ======================================================================================
#  File         : Makefile
#  Author       : Adrien Oliva
#  Last Change  : 01/02/2013 | 10:16:07 AM | Wednesday,January
#  Description  : Build script for LibAvl library
#  ======================================================================================

#       DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
#                   Version 2, December 2004 
#
# Copyright (C) 2013 Adrien Oliva <adrien.oliva@yapbreak.fr>
#
# Everyone is permitted to copy and distribute verbatim or modified 
# copies of this license document, and changing it is allowed as long 
# as the name is changed. 
#
#           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
#  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION 
#
# 0. You just DO WHAT THE FUCK YOU WANT TO.

TARGET	= libavl.so

include Makefile.global

# Link
libavl.so: avl.lo

# Dependencies
avl.o: avl.h syslog.h
avl.lo: avl.h syslog.h

