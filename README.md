# InWM

InWM adalah window manager untuk InOS berbasis X11. Dibutuhkan `xorg-dev` untuk mengcompile ini.

## Features

### Window Manager
- **System 8-style window decoration** - Gray titlebar with close button on left side
- **Window dragging** - Click and drag titlebar to move windows  
- **Window resizing** - Drag the resize handle in bottom-right corner
- **Window snapping** - Drag to edge of screen or use keyboard shortcuts
  - Left snap: Drag to left edge or `Super+Left`
  - Right snap: Drag to right edge or `Super+Right` 
  - Maximize: Drag to top or `Super+Up`
  - Restore: `Super+Down`
- **System 8-style menu bar** - Classic menu bar with File, Edit, View, Special menus

### GUI Library
- **Complete widget framework** for creating System 8-style applications
- **Button widgets** with 3D appearance and click handling
- **Dropdown menus** perfect for recreating classic Mac OS menu bars
- **Event system** with clean callback-based handling
- **Easy integration** with the InWM window manager

## Building

```bash
make                    # Build window manager and GUI library
make new_bar           # Build improved menu bar using GUI library
make example_settings  # Build example settings app
make clean             # Clean all build files
```

### GUI Library
```bash
cd lib
make                    # Build GUI library
make test_gui          # Build GUI test program
make example_settings  # Build settings application example
```

## Running

### Window Manager
```bash
./inwm         # Start window manager
./bar/bar      # Start original menu bar
./new_bar      # Start improved GUI-based menu bar
```

### Example Applications
```bash
cd lib
./test_gui            # Basic GUI test
./example_settings    # Settings application demo
```

## Controls

### Window Management
- **Drag titlebar**: Move window
- **Drag resize handle**: Resize window  
- **Click close button**: Close window
- **Super+Arrow keys**: Snap windows to screen edges
- **Bar clicks**: Access System 8-style menus

### GUI Applications  
- **Click buttons**: Trigger actions
- **Click dropdown menus**: Open/close menus
- **Select menu items**: Execute menu actions

## Creating Applications

Use the included GUI library to create System 8-style applications:

```cpp
#include "lib/Widgets.hpp"
using namespace InWM;

auto app = Application::create();
auto window = app->createWindow("My App", 400, 300);

// Add widgets
auto button = std::make_shared<Button>(app.get(), "Click Me");
button->setBounds(Rect(50, 50, 100, 30));
button->setEventCallback(CLICK, [](const Event& e) {
    printf("Button clicked!\n");
});
window->addChild(button);

window->show();
app->run();
```

See `lib/README.md` for complete GUI library documentation and examples.

## Project Structure

```
inwm/
├── inwm                    # Window manager executable
├── WindowManager.cpp/hpp   # Window manager implementation
├── bar/                    # Original menu bar
├── lib/                    # GUI library for applications
│   ├── *.hpp              # Header files  
│   ├── *.cpp              # Implementation
│   ├── libinwm.a          # Static library
│   ├── test_gui           # GUI test program
│   └── example_settings   # Settings app example
├── new_bar                # Improved GUI-based menu bar
└── test.sh               # Test script
```

This provides both a complete window manager and a framework for creating authentic System 8-style desktop applications.
