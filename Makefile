all: drawGL

SOURCES    = main.cpp Shapes.cpp
OBJECTS    = $(SOURCES:.cpp=.o)

.cpp.o:
	g++ -c -Wall $< -o $@

drawGL: main.o Shapes.o
	g++ $(OBJECTS) -lGL -lGLU -lglut $(LDFLAGS) -o $@

clean: 
	rm -f *.o
	rm drawGL
