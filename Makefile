CV_FLAGS = `pkg-config --cflags opencv` 
C_FLAG = -std=c++11 -Wall -pg
CV_LIBS = `pkg-config --libs opencv` -lpthread
GL_LIBS = -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lGLEW

% : %.cpp
	g++ $(CV_FLAGS) $(C_FLAG) -c $< 
	g++ $(CV_FLAGS) $(C_FLAG) -o $@.out $@.o $(CV_LIBS)	

stereoView.o: stereoView.cpp
	g++ $(C_FLAG) -c stereoView.cpp

tuner.o: tuner.cpp
	g++ $(C_FLAG) -c tuner.cpp	

sbmTuner : stereoView.o tuner.o
	g++ $(CV_FLAGS) $(C_FLAG) stereoView.o tuner.o -o sbmTuner.out $(CV_LIBS) 

eyeTracking.o : eyeTracking.cpp Global.h 
	g++ $(C_FLAG) -c eyeTracking.cpp	

cube.o : cube.cpp shader.cpp shader.hpp
	g++ $(C_FLAG) -c cube.cpp shader.cpp shader.hpp

#global.o : global.cpp
#	g++ $(CV-FLAGS) $(C_FLAG) -c global.cpp

main : cube.o eyeTracking.o shader.o stereoView.o Global.h
	g++ $(C_FLAG) -c main.cpp
	g++ $(C_FLAG) cube.o eyeTracking.o shader.o stereoView.o main.o -o main.out $(CV_LIBS) $(GL_LIBS)

clean :
	rm *.o *.out
