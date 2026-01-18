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

#ifndef INWM_BAR_HPP
#define INWM_BAR_HPP

#include <X11/Xlib.h>

class Bar {
  public:
    Bar(Display* dpy, Window root);
    ~Bar();

    void Run();

  private:
    Display* m_dpy;
    Window m_root;
    Window m_win;

    void CreateWindow();
    void DestroyWindow();
    void Draw();
    void HandleEvent(XEvent* ev);
    void HandleExpose(XEvent* ev);
    void HandleButtonPress(XEvent* ev);
};

#endif