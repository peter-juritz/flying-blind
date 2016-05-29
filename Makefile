CC = g++ -Wall 

all:
	$(CC) src/main.cpp -pthread -o main -lGL -lGLU  -L/usr/lib  -I/usr/include/SDL -D_REENTRANT  -L/usr/lib/x86_64-linux-gnu -lSDL  -lm -I"./SDL" src/landscape.cpp src/ship.cpp src/beacon.cpp -O6 -ggdb -lpthread  libs/libIrrKlang.so 
	mkdir DATA

clean:
	@echo Cleaning up...
	rm *.o main
	@echo Done.
