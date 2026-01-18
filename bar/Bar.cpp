#include "Bar.hpp"
#include <ctime>
#include <cstring>
#include <cstdio>
extern "C" {
#include <X11/Xatom.h>
}

Bar::Bar(Display* dpy, Window root) : m_dpy(dpy), m_root(root) {
  CreateWindow();
}

Bar::~Bar() {
  DestroyWindow();
}

void Bar::Run() {
  XEvent ev;
  while (true) {
    XNextEvent(m_dpy, &ev);
    HandleEvent(&ev);
  }
}

void Bar::CreateWindow() {
  // Get screen dimensions
  Screen* screen = DefaultScreenOfDisplay(m_dpy);
  int screenWidth = WidthOfScreen(screen);
  int screenHeight = HeightOfScreen(screen);
  
  // Create bar at top of screen with System 8-like appearance
  m_win = XCreateSimpleWindow(m_dpy, m_root, 0, 0, screenWidth, 24, 0, 0x808080, 0xC0C0C0);
  
  // Set window properties to behave like a dock/panel
  Atom wmWindowType = XInternAtom(m_dpy, "_NET_WM_WINDOW_TYPE", False);
  Atom wmWindowTypeDock = XInternAtom(m_dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
  XChangeProperty(m_dpy, m_win, wmWindowType, XA_ATOM, 32, PropModeReplace,
                  (unsigned char*)&wmWindowTypeDock, 1);
  
  // Set strut to reserve space at top of screen
  Atom wmStrut = XInternAtom(m_dpy, "_NET_WM_STRUT", False);
  long strut[4] = {0, 0, 24, 0}; // left, right, top, bottom
  XChangeProperty(m_dpy, m_win, wmStrut, XA_CARDINAL, 32, PropModeReplace,
                  (unsigned char*)&strut, 4);
  
  XSelectInput(m_dpy, m_win, ExposureMask | ButtonPressMask);
  XMapWindow(m_dpy, m_win);
  XRaiseWindow(m_dpy, m_win);
}

void Bar::DestroyWindow() {
  XDestroyWindow(m_dpy, m_win);
}

void Bar::Draw() {
  // Get screen width
  XWindowAttributes attr;
  XGetWindowAttributes(m_dpy, m_win, &attr);
  int width = attr.width;
  
  // Clear background with System 8 gray
  XSetForeground(m_dpy, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 0xC0C0C0);
  XFillRectangle(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 0, 0, width, 24);
  
  // Draw 3D border effect
  XSetForeground(m_dpy, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 0xFFFFFF); // White highlight
  XDrawLine(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 0, 0, width-1, 0); // top
  XDrawLine(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 0, 0, 0, 23); // left
  
  XSetForeground(m_dpy, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 0x808080); // Dark gray shadow
  XDrawLine(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 0, 23, width-1, 23); // bottom
  XDrawLine(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), width-1, 0, width-1, 23); // right
  
  // Draw Apple menu (black text)
  XSetForeground(m_dpy, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 0x000000);
  XDrawString(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 8, 16, "\xEF\x82\x8F", 3); // Apple logo (if font supports it)
  
  // Draw menu items
  XDrawString(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 35, 16, "File", 4);
  XDrawString(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 70, 16, "Edit", 4);
  XDrawString(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 105, 16, "View", 4);
  XDrawString(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 140, 16, "Special", 7);
  
  // Draw time on the right side
  time_t rawtime;
  struct tm * timeinfo;
  char timestr[80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timestr, sizeof(timestr), "%H:%M", timeinfo);
  
  int timeWidth = strlen(timestr) * 8; // Approximate character width
  XDrawString(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 
              width - timeWidth - 10, 16, timestr, strlen(timestr));
}

void Bar::HandleEvent(XEvent* ev) {
  switch (ev->type) {
    case Expose:
      HandleExpose(ev);
      break;
    case ButtonPress:
      HandleButtonPress(ev);
      break;
  }
}

void Bar::HandleExpose(XEvent* ev) {
  Draw();
}

void Bar::HandleButtonPress(XEvent* ev) {
  // Handle menu clicks - for now just print which area was clicked
  XButtonEvent* be = &ev->xbutton;
  
  if (be->x < 30) {
    printf("Apple menu clicked\n");
  } else if (be->x >= 35 && be->x < 65) {
    printf("File menu clicked\n");
  } else if (be->x >= 70 && be->x < 100) {
    printf("Edit menu clicked\n");
  } else if (be->x >= 105 && be->x < 135) {
    printf("View menu clicked\n");
  } else if (be->x >= 140 && be->x < 190) {
    printf("Special menu clicked\n");
  }
}

