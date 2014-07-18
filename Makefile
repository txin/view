CV_FLAGS = `pkg-config --cflags opencv` 
C_FLAG = -std=c++11
CV_LIBS = `pkg-config --libs opencv` -lpthread
GL_LIBS = -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lGLEW

% : %.cpp
	g++ $(CV_FLAGS) $(C_FLAG) -c $< 
	g++ $(CV_FLAGS) $(C_FLAG) -o $@.out $@.o $(CV_LIBS)	

eyeTracking.o : eyeTracking.cpp Global.h 
	g++ $(C_FLAG) -c eyeTracking.cpp	

cube.o : cube.cpp shader.cpp shader.hpp
	g++ $(C_FLAG) -c cube.cpp shader.cpp shader.hpp

#global.o : global.cpp
#	g++ $(CV-FLAGS) $(C_FLAG) -c global.cpp

main : cube.o eyeTracking.o shader.o Global.h
	g++ $(C_FLAG) -c main.cpp
	g++ cube.o eyeTracking.o shader.o main.o -o main.out $(CV_LIBS) $(GL_LIBS)
clean :
	rm *.o *.out
