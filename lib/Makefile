# InWM GUI Library Makefile

CXX ?= g++
CXXFLAGS ?= -Wall -g -std=c++17
CXXFLAGS += `pkg-config --cflags x11`
LDFLAGS += `pkg-config --libs x11`

LIB_NAME = libinwm
LIB_STATIC = $(LIB_NAME).a
LIB_SHARED = $(LIB_NAME).so

HEADERS = \
	GUI.hpp \
	Application.hpp \
	Window.hpp \
	Button.hpp \
	Dropdown.hpp \
	Widgets.hpp

SOURCES = \
	GUI.cpp \
	Application.cpp \
	Window.cpp \
	Button.cpp \
	Dropdown.cpp

OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(LIB_STATIC) $(LIB_SHARED)

# Static library
$(LIB_STATIC): $(OBJECTS)
	ar rcs $@ $^

# Shared library
$(LIB_SHARED): $(OBJECTS)
	$(CXX) -shared -o $@ $^ $(LDFLAGS)

# Object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

# Install headers and library (optional)
install: $(LIB_STATIC) $(LIB_SHARED)
	@echo "Installing InWM GUI library..."
	@mkdir -p ../include/inwm
	@cp $(HEADERS) ../include/inwm/
	@mkdir -p ../lib
	@cp $(LIB_STATIC) $(LIB_SHARED) ../lib/

# Clean
.PHONY: clean
clean:
	rm -f $(OBJECTS) $(LIB_STATIC) $(LIB_SHARED)

# Test programs
test: test_gui

test_gui: test_gui.cpp $(LIB_STATIC)
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -linwm $(LDFLAGS)

.PHONY: all install clean test