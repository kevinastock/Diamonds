CC = gcc
FLAGS = -W -Wall
FILES = main.c game.c
INCLUDES = -I .
LINKTO = -lSDL -lSDL_mixer -lSDL_image -lm
AUTO = `sdl-config --libs --cflags`
TARGET = diamonds

main: game.c main.c
	$(CC) $(AUTO) $(FILES) $(INCLUDES) -o $(TARGET) $(LINKTO)

