sound_seg.so: sound_seg.c
	gcc -fPIC -shared -o sound_seg.so sound_seg.c
