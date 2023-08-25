all: WelcomeToTheSlam

WelcomeToTheSlam: WelcomeToTheSlam.c
	gcc WelcomeToTheSlam.c -Wall -Wextra -o WelcomeToTheSlam -lX11 -lGL -lGLU -lm ./lib/libggfonts.so ./lib/ppm.c

clean:
	rm -f WelcomeToTheSlam
	rm -f *.o

