OBJS = main.cpp 
CPP = g++
CFLAGS = -g 

all: main.cpp  
	$(CPP) $(OBJS) -o cam $(CFLAGS) -std=c++11 
part2: main2.cpp
	$(CPP) main2.cpp -o cam $(CFLAGS) -std=c++11
clean: 
	$(RM) -r cam
