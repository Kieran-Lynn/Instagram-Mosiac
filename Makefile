TARGET=project
USER_NAME=philadelphiaeagles
CC=gcc
FLAGS=-I./src/ `pkg-config --cflags opencv` -fopenmp
LIBS=-lm `pkg-config --libs opencv` -fopenmp 

run : $(TARGET) jsb hw4ex cleanImages
	export OMP_NUM_THREADS=25
	./$< $(USER_NAME) || /bin/true
	mv profPic/* profPic/prof_pic.jpg
	rm -f downloads/*.mp4
	./hw4ex profPic/prof_pic.jpg 75 75 downloads/*

project : project.c
	gcc -fopenmp -o $@ $^

remove_char : remove_char.c
	gcc -o $@ $^

jsb : jsb.c
	gcc -o $@ $^

clean :
	rm -f jsb remove_char project hw4ex lib/*.o 
	
cleanImages: 
	rm -f downloads/*
	rm -f profPic/*

lib/%.o: src/%.c
	gcc $(FLAGS) -c $< -o $@

hw4ex: 
	make -f MakeHW4 
