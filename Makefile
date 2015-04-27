all: WelcomeToTheSlam

WelcomeToTheSlam: WelcomeToTheSlam.c
	gcc WelcomeToTheSlam.c -Wall -Wextra -o WelcomeToTheSlam -lX11 -lGL -lGLU -lm ./libggfonts.so -w

clean:
	rm -f WelcomeToTheSlam
	rm -f *.o

