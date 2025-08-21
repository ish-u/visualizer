main: main.c
	gcc  main.c ./src/gl.c -I./include/ -o main `sdl2-config --cflags --libs` 
	./main

.PHONY: clean
clean:
	rm -f main
