LAGS = `pkg-config --cflags opencv` -std=c++11
LIBS = `pkg-config --libs opencv` -lpthread

% : %.cpp
	g++ $(CFLAGS) -o $@.out $< $(LIBS)

cube : cube.cpp shader.cpp shader.hpp
	g++ -std=c++11 -c cube.cpp shader.cpp shader.hpp
	g++ cube.o shader.o -o cube.out -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lGLEW

clean :
	rm *.o *.out
