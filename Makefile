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

.PHONY: clean
clean:
	rm -f inwm $(OBJECTS)
