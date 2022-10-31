#
#   Makefile
#
#   Copyright 2007, 2008 Lancer-X/ASCEAI
#
#   This file is part of Meritous.
#
#   Meritous is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Meritous is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Meritous.  If not, see <http://www.gnu.org/licenses/>.
#
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lz -lpthread
#CCFLAGS = -Wall `sdl-config --cflags` -ggdb
CCFLAGS = -Os -Wall `sdl2-config --cflags`
#DEFINES = -DDEBUG_KEYS

AP_INCLUDES = -Isrc/submodules/wswrap/include\
              -Isrc/submodules/json/include\
              -Isrc/submodules/websocketpp\
              -Isrc/submodules/asio/include\
              -Isrc/submodules/valijson/include\
              -Isrc/submodules
AP_LIBS     = -lwsock32 -lws2_32 # for windows
AP_DEFINES  = -DASIO_STANDALONE

#
OBJS = 	src/levelblit.o \
		src/mapgen.o \
		src/demon.o \
		src/gamemap.o \
		src/tiles.o \
		src/save.o \
		src/help.o \
		src/audio.o \
		src/boss.o \
		src/ending.o \
		src/gamepad.o \
		src/itemstore.o \
		src/itemhandler.o \
		src/stats.o
#
default:	meritous

debug: CCFLAGS += -g -D DEBUG
debug: meritous

# this is your cpp code that bridges between apclientpp and the game
apinterface.o: src/apinterface.cpp
		g++ -c $? -o $@ ${AP_INCLUDES} ${AP_DEFINES} ${CCFLAGS}

wswrap.o: src/submodules/wswrap/src/wswrap.cpp
		g++ -c $? -o $@ ${AP_INCLUDES} ${AP_DEFINES} ${CCFLAGS}

%.o:		%.c
		gcc -c -o $@ $? ${CCFLAGS} ${DEFINES}

meritous.res: meritous.rc
		windres $? -O coff -o $@

meritous:	${OBJS} apinterface.o meritous.res
		g++ -o $@ ${OBJS} apinterface.o meritous.res ${AP_LIBS} ${LDFLAGS}

clean:		
		rm ${OBJS} wswrap.o apinterface.o

