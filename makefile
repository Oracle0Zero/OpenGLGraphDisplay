vpath %.cpp src
vpath %.c src
vpath %.h include

CC = g++
CPPFLAGS = -fopenmp -lfreetype
OUTPUT_OPTION = -o $@
LOADLIBES = -lGL -lGLU -lSOIL -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi 

main.exec: main.o shader.o glad.o imgui.o imgui_impl_glfw.o imgui_impl_opengl3.o imgui_draw.o imgui_tables.o imgui_widgets.o imgui_demo.cpp
	$(CC) $^ $(CPPFLAGS) $(LOADLIBES) -o $@
	rm -r *.o

%.o: %.c
	$(CC) -c $(CPPFLAGS) $< $(OUTPUT_OPTION)

%.o: %.cpp
	$(CC) -c $(CPPFLAGS) $< $(OUTPUT_OPTION)	

