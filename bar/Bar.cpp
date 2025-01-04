#include "Bar.hpp"

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
  m_win = XCreateSimpleWindow(m_dpy, m_root, 0, 0, 800, 30, 0, 0, 0);
  XSelectInput(m_dpy, m_win, ExposureMask);
  XMapWindow(m_dpy, m_win);
}

void Bar::DestroyWindow() {
  XDestroyWindow(m_dpy, m_win);
}

void Bar::Draw() {
  XSetForeground(m_dpy, DefaultGC(m_dpy, DefaultScreen(m_dpy)), WhitePixel(m_dpy, DefaultScreen(m_dpy)));
  XFillRectangle(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 0, 0, 800, 30);
  XSetForeground(m_dpy, DefaultGC(m_dpy, DefaultScreen(m_dpy)), BlackPixel(m_dpy, DefaultScreen(m_dpy)));
  XDrawString(m_dpy, m_win, DefaultGC(m_dpy, DefaultScreen(m_dpy)), 10, 20, "InWM Bar", 8);
}

void Bar::HandleEvent(XEvent* ev) {
  switch (ev->type) {
    case Expose:
      HandleExpose(ev);
      break;
  }
}

void Bar::HandleExpose(XEvent* ev) {
  Draw();
}

