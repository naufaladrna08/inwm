CXXFLAGS ?= -Wall -g
CXXFLAGS += -std=c++17
CXXFLAGS += `pkg-config --cflags x11 libglog`
LDFLAGS += `pkg-config --libs x11 libglog`

all: inwm lib

HEADERS = \
	WindowManager.hpp
SOURCES = \
	WindowManager.cpp \
	main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

inwm: $(HEADERS) $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

# GUI Library
lib:
	$(MAKE) -C lib

# New improved bar using GUI library
new_bar: new_bar.cpp lib
	$(CXX) $(CXXFLAGS) -o $@ $< -Llib -linwm $(LDFLAGS)

# Settings example
example_settings: lib
	$(MAKE) -C lib example_settings

# Desktop test
test_desktop: inwm new_bar
	@echo "Desktop environment ready. Run './test_desktop.sh' to start."

BAR_HEADERS = \
	bar/Bar.hpp
BAR_SOURCES = \
	bar/Bar.cpp \
	bar/main.cpp
BAR_OBJECTS = $(BAR_SOURCES:.cpp=.o)

bar/bar: $(BAR_HEADERS) $(BAR_OBJECTS)
	$(CXX) -o $@ $(BAR_OBJECTS) $(LDFLAGS) 

.PHONY: clean lib
clean:
	rm -f inwm $(OBJECTS)
	rm -f new_bar
	$(MAKE) -C lib clean
