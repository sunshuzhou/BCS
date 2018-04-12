
src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)
targets = server client

boost = /home/sunshuzhou/toolchains/
flags = -I$(boost)include -L$(boost)lib
flags += -lboost_thread -lboost_timer -lpthread -lboost_system -O3 -fomit-frame-pointer -fopenmp

all: $(targets)

$(obj): %.o: %.cpp
	g++ $< -c -o $@ $(flags)

$(targets): %: %.o
	g++ $< -o $@ $(flags)

clean:
	rm -f $(obj) $(targets)
