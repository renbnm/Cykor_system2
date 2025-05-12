myshell: main.o
	gcc -o myshell main.o

main.o: main.c
	gcc -c main.c

clean:
	rm -f *.o myshell
