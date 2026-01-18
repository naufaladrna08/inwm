# InWM GUI Library

A minimal C++ GUI library for creating System 8-style applications in the InWM window manager.

## Overview

The InWM GUI Library provides a simple framework for creating native applications with a classic Mac OS System 8 appearance. It includes widgets like buttons and dropdown menus that are essential for building desktop applications.

## Architecture

The library is built around these core components:

- **Application**: Main application class that manages X11 connection and event loop
- **Window**: Top-level window container for widgets  
- **Widget**: Base class for all UI elements
- **Button**: Clickable button with 3D appearance
- **Dropdown**: System 8-style dropdown menu with separators

## Key Features

**System 8 Styling**: Authentic gray 3D appearance with proper highlighting/shadows  
**Event System**: Clean callback-based event handling  
**Dropdown Menus**: Perfect for recreating classic Mac OS menu bars  
**Easy Integration**: Works seamlessly with InWM window manager  
**Modern C++**: Uses C++17 features for clean, type-safe code

## Quick Start

### 1. Include the Library

```cpp
#include "lib/Widgets.hpp"
using namespace InWM;
```

### 2. Create Basic Application

```cpp
// Create application
auto app = Application::create();
auto window = app->createWindow("My App", 400, 300);

// Create button
auto button = std::make_shared<Button>(app.get(), "Click Me");
button->setBounds(Rect(50, 50, 100, 30));
button->setEventCallback(CLICK, [](const Event& e) {
    printf("Button clicked!\n");
});
window->addChild(button);

// Show and run
window->show();
app->run();
```

### 3. Create Dropdown Menus

```cpp
auto fileMenu = std::make_shared<Dropdown>(app.get(), "File");
fileMenu->setBounds(Rect(10, 10, 60, 25));

fileMenu->addItem("New", [](const Event& e) { 
    printf("New file\n"); 
});
fileMenu->addItem("Open", [](const Event& e) { 
    printf("Open file\n"); 
});
fileMenu->addSeparator();
fileMenu->addItem("Exit", [&app](const Event& e) { 
    app->quit(); 
});

window->addChild(fileMenu);
```

## Building Applications

### Compile with the Library

```bash
# Build the GUI library first
cd lib && make

# Compile your application
g++ -std=c++17 `pkg-config --cflags x11` -I./lib \
    -o myapp myapp.cpp -Llib -linwm `pkg-config --libs x11`
```

### Makefile Integration

```makefile
CXXFLAGS += -std=c++17 `pkg-config --cflags x11`
LDFLAGS += -Llib -linwm `pkg-config --libs x11`

myapp: myapp.cpp lib/libinwm.a
	$(CXX) $(CXXFLAGS) -I./lib -o $@ $< $(LDFLAGS)

lib/libinwm.a:
	$(MAKE) -C lib
```

## Widget Reference

### Application
- `Application::create()` - Create application instance
- `createWindow(title, width, height)` - Create new window
- `run()` - Start event loop
- `quit()` - Exit application

### Window  
- `show()` - Display window
- `hide()` - Hide window
- `setTitle(title)` - Change window title
- `addChild(widget)` - Add widget to window

### Button
- `setText(text)` - Set button label
- `setEventCallback(CLICK, callback)` - Handle clicks

### Dropdown
- `addItem(text, callback)` - Add menu item
- `addSeparator()` - Add separator line
- `setOpen(bool)` - Open/close programmatically

### Widget (Base)
- `setBounds(Rect(x, y, w, h))` - Set position/size
- `setVisible(bool)` - Show/hide widget
- `setEnabled(bool)` - Enable/disable widget
- `setEventCallback(type, callback)` - Handle events

## Event System

Events are handled through callbacks:

```cpp
widget->setEventCallback(CLICK, [](const Event& e) {
    // e.x, e.y = click coordinates
    // e.button = mouse button
    // e.target = widget that was clicked
});
```

Available event types:
- `CLICK` - Mouse click
- `HOVER` - Mouse movement
- `FOCUS` - Widget gains focus
- `UNFOCUS` - Widget loses focus  
- `KEY_PRESS` - Key pressed

## Examples

### Settings Application
See `lib/example_settings.cpp` for a complete settings app with multiple dropdowns.

### Menu Bar
See `new_bar.cpp` for a System 8-style menu bar implementation.

### Simple Test
See `lib/test_gui.cpp` for basic widget demonstration.

## Creating Custom Widgets

Inherit from `Widget` and implement:

```cpp
class CustomWidget : public Widget {
public:
    CustomWidget(Application* app, Widget* parent = nullptr)
        : Widget(app, parent) {}
    
    void draw(Drawable drawable, GC gc) override {
        // Custom drawing code
        const Rect& bounds = getBounds();
        // Draw your widget...
    }
    
    void handleEvent(const Event& event) override {
        // Custom event handling
    }
};
```

## System 8 Menu Bar Pattern

The dropdown widget is specifically designed to recreate System 8 menu bars:

```cpp
// Apple menu
auto appleMenu = std::make_shared<Dropdown>(app.get(), "🍎");
appleMenu->addItem("About This Computer", callback);
appleMenu->addSeparator();
appleMenu->addItem("System Preferences...", callback);

// File menu  
auto fileMenu = std::make_shared<Dropdown>(app.get(), "File");
// ... add items

// Position them in a row for menu bar effect
```

## Integration with InWM

The library is designed to work perfectly with the InWM window manager:

- Windows get proper System 8-style decoration from InWM
- Consistent gray color scheme matches window frames
- Dropdown menus work well with window snapping
- Applications can be minimized/maximized like classic Mac OS

## Building Library

```bash
cd lib
make                    # Build static and shared libraries
make test_gui          # Build test program
make example_settings  # Build settings example
```

This creates `libinwm.a` (static) and `libinwm.so` (shared) libraries.

## Future Enhancements

Planned widgets for future versions:
- Label widget for text display
- TextBox for text input
- CheckBox and RadioButton
- ScrollBar and ListView
- Dialog boxes and file choosers
- StatusBar for bottom-of-window status

The current foundation makes adding these widgets straightforward while maintaining the System 8 aesthetic.