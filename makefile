CC = gcc

OBJ = sound_seg.o

HEAD = sound_struct.h

SOURCE = sound_seg.c

FLAGS = -fPIC -Wvla -Werror -fsanitize=address -g

$(OBJ): $(SOURCE) $(HEAD)
	$(CC) $(FLAGS) -c  $(SOURCE) -o $(OBJ)