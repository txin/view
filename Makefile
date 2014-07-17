LAGS = `pkg-config --cflags opencv` -std=c++11
LIBS = `pkg-config --libs opencv` -lpthread

% : %.cpp
	g++ $(CFLAGS) -o $@.out $< $(LIBS)
cube : cube.cpp
	g++ -std=c++11 -c cube.cpp
	g++ cube.o -o cube.out -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi

t : t.cpp shader.cpp shader.hpp
	g++ -std=c++11 -c t.cpp shader.cpp shader.hpp
	g++ t.o shader.o -o t.out -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lGLEW
clean :
	rm *.o
