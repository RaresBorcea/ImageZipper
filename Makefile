.PHONY: build run clean

build: quadtree

quadtree: quadtree.o
	gcc -Wall quadtree.o -o quadtree -lm
quadtree.o: quadtree.c
	gcc -c quadtree.c -o quadtree.o
run: quadtree
	./quadtree
clean:
	rm -rf quadtree.o quadtree
