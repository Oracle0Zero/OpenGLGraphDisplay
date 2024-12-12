vpath %.cpp src
vpath %.c src
vpath %.h include

CC = g++
CPPFLAGS = -fopenmp
OUTPUT_OPTION = -o $@
LOADLIBES = -lGL -lGLU -lSOIL -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi

main.exec: main.o shader.o glad.o
	$(CC) $^ $(CPPFLAGS) $(LOADLIBES) -o $@
	rm -r *.o

%.o: %.c
	$(CC) -c $(CPPFLAGS) $< $(OUTPUT_OPTION)

%.o: %.cpp
	$(CC) -c $(CPPFLAGS) $< $(OUTPUT_OPTION)	

