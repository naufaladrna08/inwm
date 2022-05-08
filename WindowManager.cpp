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
    case ConfigureRequest:
      OnConfigureRequest(e.xconfigurerequest);
    case MapRequest:
      OnMapRequest(e.xmaprequest);
    case MapNotify:
      OnMapNotify(e.xmap);
    case UnmapNotify:
      OnUnmapNotify(e.xunmap);
      break;
    case ConfigureNotify:
      OnConfigureNotify(e.xconfigure);
    
    default:
      printf("Ignored event");
  }
}

int WindowManager::OnWMDetected(Display* dpy, XErrorEvent* e) {
  if (static_cast<int>(e->error_code) == BadAccess) {
    wm_detected_ = true;
  }

  return 0;
}

int WindowManager::OnXError(Display* dpy, XErrorEvent* e) {
  // printf(e->) TODO: xixixi

  return 0;
}

void WindowManager::OnCreateNotify(const XCreateWindowEvent& e) {}
void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e) {}
void WindowManager::OnReparentNotify(const XReparentEvent& e) {}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e) {
  XWindowChanges changes;
  changes.x = e.x;
  changes.y = e.y;
  changes.width = e.width;
  changes.height = e.height;
  changes.border_width = e.border_width;
  changes.sibling = e.above;
  changes.stack_mode = e.detail;

  if (m_clients.count(e.window)) {
    const Window frame = m_clients[e.window];
    XConfigureWindow(_dpy, frame, e.value_mask, &changes);
  }

  XConfigureWindow(_dpy, e.window, e.value_mask, &changes);
  printf("Resize window %d, %d\n", e.width, e.height);
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
  Frame(e.window);
  XMapWindow(_dpy, e.window);
}

void WindowManager::Frame(Window w) {
  const unsigned int BORDER_WIDTH = 3;
  const unsigned long BORDER_COLOR = 0xffffff;
  const unsigned long BACKGROUND_COLOR = 0x1ABC9C;

  XWindowAttributes xattr;

  const Window frame = XCreateSimpleWindow(
    _dpy,
    _root,
    xattr.x, xattr.y,
    xattr.width, xattr.height,
    BORDER_WIDTH, BORDER_COLOR,
    BACKGROUND_COLOR
  );

  XAddToSaveSet(_dpy, w);
  XSelectInput(_dpy, frame, SubstructureRedirectMask | SubstructureNotifyMask);
  XReparentWindow(_dpy, w, frame, 0, 0);
  XMapWindow(_dpy, frame);

  m_clients[w] = frame;
}

void WindowManager::Unframe(Window w) {
  const Window frame = m_clients[w];

  XUnmapWindow(_dpy, w);
  XReparentWindow(_dpy, w, _root, 0, 0);
  XRemoveFromSaveSet(_dpy, w);
  XDestroyWindow(_dpy, frame);
  m_clients.erase(w);
}

void WindowManager::OnMapNotify(const XMapEvent& e) {}

void WindowManager::OnUnmapNotify(const XUnmapEvent& e) {
  if (!m_clients.count(e.window)) {
    printf("Ignore unmap notify for non-client window");
    return;
  }

  Unframe(e.window);
}

void WindowManager::OnConfigureNotify(const XConfigureEvent& e) {}