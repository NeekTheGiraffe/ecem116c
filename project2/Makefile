CC=g++
DEBUG_FLAGS=-std=c++17 -Wall -Wextra -Weffc++ -pedantic -g -fsanitize=address -fsanitize=undefined

all: clean build

check: build
	python3 check.py

debug: *cpp *.h
	$(CC) $(DEBUG_FLAGS) *.cpp -o memory_driver

build: *.cpp *.h
	$(CC) *.cpp -o memory_driver

clean:
	rm -f memory_driver