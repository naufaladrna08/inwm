CXXFLAGS ?= -Wall -g
CXXFLAGS += -std=c++17
CXXFLAGS += `pkg-config --cflags x11 libglog`
LDFLAGS += `pkg-config --libs x11 libglog`

all: inwm

HEADERS = \
	WindowManager.hpp
SOURCES = \
	WindowManager.cpp \
	main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

inwm: $(HEADERS) $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

BAR_HEADERS = \
	bar/Bar.hpp
BAR_SOURCES = \
	bar/Bar.cpp \
	bar/main.cpp
BAR_OBJECTS = $(BAR_SOURCES:.cpp=.o)

bar/bar: $(BAR_HEADERS) $(BAR_OBJECTS)
	$(CXX) -o $@ $(BAR_OBJECTS) $(LDFLAGS) 

.PHONY: clean
clean:
	rm -f inwm $(OBJECTS)
