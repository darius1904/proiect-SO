CC = gcc
CFLAGS = -Wall -Wextra -g

all: treasure_manager treasure_hub monitor

treasure_manager: main.c treasure.h
	$(CC) $(CFLAGS) main.c -o treasure_manager

treasure_hub: treasure_hub.c treasure.h
	$(CC) $(CFLAGS) treasure_hub.c -o treasure_hub

monitor: monitor.c treasure.h
	$(CC) $(CFLAGS) monitor.c -o monitor

clean:
	rm -f treasure_manager treasure_hub monitor *.o
