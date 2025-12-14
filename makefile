# Makefile for Shortest Path Applications
# Requires GTK+ 3.0 and Cairo

CC = gcc 	
CFLAGS = -Wall -Wextra -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0` -lm

# Source files
SOURCES = main.c tsp.c tsp_algorithms.c kirchhoff.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = run_main

# Build targets
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

# Dependencies
main.o: main.c tsp.h kirchhoff.h
tsp.o: tsp.c tsp.h tsp_algorithms.h
tsp_algorithms.o: tsp_algorithms.c tsp_algorithms.h tsp.h
kirchhoff.o: kirchhoff.c kirchhoff.h

# Installation of dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libgtk-3-dev build-essential

# Run the application
run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: all clean install-deps run