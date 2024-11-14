rm main.exec
g++ -std=c++11 -c main.cpp shader.cpp;
g++ -c  glad.c;
g++ main.o -o main.exec glad.o shader.o -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi;
./main.exec

