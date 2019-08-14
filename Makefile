sshell: sshell.h sshell.c
	gcc -Wall -Werror -g sshell.c -o sshell

clean:
	rm -f *.o sshell core
