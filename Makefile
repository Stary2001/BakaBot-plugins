CXXFLAGS=-fPIC -std=c++11 -I$(COREPATH)/include/plugin -I$(COREPATH)/include/sock -I../src -I../src/event -shared
PLUGINS=$(patsubst %.cpp,%.so,$(wildcard ./*.cpp))

all: $(PLUGINS)

%.so : %.cpp
	g++ $(CXXFLAGS) $< -o $@
