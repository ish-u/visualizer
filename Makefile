main: main.c
	gcc  main.c -o main `sdl2-config --cflags --libs` 
	./main

.PHONY: clean
clean:
	rm -f main
