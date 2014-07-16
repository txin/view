CFLAGS = `pkg-config --cflags opencv` -std=c++11
LIBS = `pkg-config --libs opencv` -lpthread

% : %.cpp
	g++ $(CFLAGS) -o $@.out $< $(LIBS)
cube : cube.cpp
	g++ -std=c++11 -c cube.cpp
	g++ cube.o -o cube.out -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi

clean :
	rm *.o
