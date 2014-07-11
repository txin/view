CFLAGS = `pkg-config --cflags opencv` -std=c++11
LIBS = `pkg-config --libs opencv`

% : %.cpp
	g++ $(CFLAGS) -o $@.out $< $(LIBS)

StereoView : StereoView.cpp
	g++ $(CFLAGS) -o $@.out $< $(LIBS) 
