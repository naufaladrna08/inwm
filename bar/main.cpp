/* 
 * InWM Bar - Single-Window Window Manager Bar
 * 
 * The only bar that you need. It shows the title of the currently focused window,
 * menu items, task switcher, and system tray. The system tray is slightly different
 * than traditional "Windows" system tray. Because the Bar itself is just a title bar
 * menu and task switcher, we need to put the network manager, volume control, time
 * etc somewhere, and that's the system tray.
 * 
 * (c) 2025 InWM Project
 */

#include "Bar.hpp"
#include <iostream>
#include <memory>

int main(int argc, char** argv) {
  Display* dpy = XOpenDisplay(NULL);
  if (!dpy) {
    std::cerr << "Failed to open display" << std::endl;
    return 1;
  }

  int screen = DefaultScreen(dpy);
  Window root = RootWindow(dpy, screen);

  std::unique_ptr<Bar> bar(new Bar(dpy, root));
  bar->Run();

  XCloseDisplay(dpy);
  return 0;
}