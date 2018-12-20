CPP = g++
CPPFLAGS = -Wall -std=c++11 -O3
HERDERS = BLOCK.h TERMINAL.h NET.h
SOURCES = main.cpp BLOCK.cpp TERMINAL.cpp NET.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = hw3

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean: 
	rm -rf *.o $(EXECUTABLE)