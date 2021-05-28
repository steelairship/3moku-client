Shyheim Williams
CMSC242
Project 3

This is a client I wrote for my networks and systems class and it is designed to interact with a server my professor wrote. 

to compile:
	gcc -o proj3 main.c

to run:
	proj3 [host] [port] [gameID]
	example: proj3 turing 9908 33914

notes: sometimes does not read the bytes properly when text gets displayed as weird characters. When I run the server myself it runs properly and doesn't display the wrong characters. Just terminate the program and retry until it displays properly.
