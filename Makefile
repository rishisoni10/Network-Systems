all:
	gcc -g webserver.c
run:
	./a.out
clean:
	rm -rf a.out