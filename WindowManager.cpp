#include "WindowManager.hpp"
#include <cstdio>
using std::unique_ptr;


bool WindowManager::wm_detected_;

/*
 * Factory method for establishing a connection to an X server and creating
 * a Window Manager instance.
 */
std::unique_ptr<WindowManager> WindowManager::Create() {
  /* Open the X display. */
  Display* dpy = XOpenDisplay(nullptr);
  if (dpy == nullptr) {
    printf("Unable to start X Display.\n");
    return nullptr;
  }

  return unique_ptr<WindowManager> (new WindowManager(dpy));
}

/*
 * Invoke internally by Create()
 */
WindowManager::WindowManager(Display* dpy)
: _dpy (dpy),
  _root(DefaultRootWindow(_dpy)) {
}


/*
 * Disconnect from the X server.
 */
WindowManager::~WindowManager() {
  XCloseDisplay(_dpy);
}

void WindowManager::Run() {
  /* Initializastion */
  wm_detected_ = false;
  
  XSetErrorHandler(&WindowManager::OnWMDetected);
  XSelectInput(_dpy, _root, SubstructureRedirectMask | SubstructureNotifyMask);

  if (wm_detected_) {
    printf("Detected another window manager %s", XDisplayString(_dpy));
    return;
  }

  XSetErrorHandler(&WindowManager::OnXError);
  
  XEvent e;
  XNextEvent(_dpy, &e);

  switch (e.type) {
    case CreateNotify:
      OnCreateNotify(e.xcreatewindow);
      break;
    case DestroyNotify:
      OnDestroyNotify(e.xdestroywindow);
      break;
    case ReparentNotify:
      OnReparentNotify(e.xreparent);
      break;
      //...
      // etc. etc.
      //...
    default:
      printf("Ignored event");
  }
}

int WindowManager::OnWMDetected(Display* dpy, XErrorEvent* e) {
  // if (static_cast<int>(e->error_code) == BadAccess) {
  //   wm_detected_ = true;
  // }

  return 0;
}

int WindowManager::OnXError(Display* dpy, XErrorEvent* e) {
  // printf(e->) TODO: xixixi

  return 0;
}

void WindowManager::OnCreateNotify(const XCreateWindowEvent& e) {}
void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e) {}
void WindowManager::OnReparentNotify(const XReparentEvent& e) {}
