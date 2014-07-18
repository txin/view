CFLAGS = `pkg-config --cflags opencv` -std=c++11
LIBS = `pkg-config --libs opencv` -lpthread

% : %.cpp
	g++ $(CFLAGS) -c $< 
	g++ $(CFLAGS) -o $@.out $@.o $(LIBS)	

eyeTracking.o : eyeTracking.cpp 
	g++ -std=c++11 -c eyeTracking.cpp	

cube.o : cube.cpp shader.cpp shader.hpp
	g++ -std=c++11 -c cube.cpp shader.cpp shader.hpp
#g++ cube.o shader.o -o cube.out -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lGLEW

main : cube.o eyeTracking.o shader.o
	g++ -std=c++11 -c main.cpp
	g++ cube.o eyeTracking.o shader.o main.o -o main.out -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lGLEW $(LIBS)
clean :
	rm *.o *.out
