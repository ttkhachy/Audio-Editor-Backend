#TODO
CC = gcc


OBJ = sound_seg.o

DEPS = sound_struct.h

SOURCE = sound_seg.c


FLAGS = -fPIC -Wvla -Werror -fsanitize=address -g

$(OBJ): $(SOURCE) $(DEPS)
	$(CC) $(FLAGS) -c  $(SOURCE) -o $(OBJ)