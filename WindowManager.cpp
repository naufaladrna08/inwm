#include "WindowManager.hpp"
#include <cstdio>
using std::unique_ptr;


bool WindowManager::m_wmDetected;

/*
 * Factory method for establishing a connection to an X server and creating
 * a Window Manager instance.
 */
std::unique_ptr<WindowManager> WindowManager::Create() {
  /* Open the X display. */
  Display* dpy = XOpenDisplay(nullptr);
  if (dpy == nullptr) {
    printf("Unable to open display: %s\n", XDisplayName(nullptr));
    return nullptr;
  }

  return std::unique_ptr<WindowManager> (new WindowManager(dpy));
}

/*
 * Invoke internally by Create()
 */
WindowManager::WindowManager(Display* dpy)
: m_dpy (dpy),
  m_root(DefaultRootWindow(m_dpy)) {
}

/*
 * Disconnect from the X server.
 */
WindowManager::~WindowManager() {
  XCloseDisplay(m_dpy);
}

void WindowManager::Run() {
  /* Initialization */
  m_wmDetected = false;
  
  XSetErrorHandler(&WindowManager::OnWMDetected);
  XSelectInput(m_dpy, m_root, SubstructureRedirectMask | SubstructureNotifyMask);

  XSync(m_dpy, false);
  if (m_wmDetected) {
    printf("Detected another window manager %s\n", XDisplayString(m_dpy));
    return;
  }

  XSetErrorHandler(&WindowManager::OnXError);
  
  while (true) {
    XEvent e;
    XNextEvent(m_dpy, &e);

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
        break;
      case MapRequest:
        OnMapRequest(e.xmaprequest);
        break;
      case MapNotify:
        OnMapNotify(e.xmap);
        break;
      case UnmapNotify:
        OnUnmapNotify(e.xunmap);
        break;
      case ConfigureNotify:
        OnConfigureNotify(e.xconfigure);
        break;
      
      default:
        printf("Ignored event\n");
    }
  }
}

int WindowManager::OnWMDetected(Display* dpy, XErrorEvent* e) {
  if (static_cast<int>(e->error_code) == BadAccess) {
    m_wmDetected = true;
  }

  return 0;
}

int WindowManager::OnXError(Display* dpy, XErrorEvent* e) {
  char error[255];
  printf("Received X error: %d\n", XGetErrorText(dpy, e->error_code, error, 255));

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
    XConfigureWindow(m_dpy, frame, e.value_mask, &changes);
  }

  XConfigureWindow(m_dpy, e.window, e.value_mask, &changes);
  printf("Resize window %d, %d\n", e.width, e.height);
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
  Frame(e.window);
  XMapWindow(m_dpy, e.window);
}

void WindowManager::Frame(Window w) {
  const unsigned int BORDER_WIDTH = 3;
  const unsigned long BORDER_COLOR = 0xffffff;
  const unsigned long BACKGROUND_COLOR = 0x1ABC9C;

  XWindowAttributes xattr;
  XGetWindowAttributes(m_dpy, w, &xattr);

  const Window frame = XCreateSimpleWindow(
    m_dpy,
    m_root,
    xattr.x, xattr.y,
    xattr.width, xattr.height,
    BORDER_WIDTH, BORDER_COLOR,
    BACKGROUND_COLOR
  );

  XAddToSaveSet(m_dpy, w);
  XSelectInput(m_dpy, frame, SubstructureRedirectMask | SubstructureNotifyMask);
  XReparentWindow(m_dpy, w, frame, 0, 0);
  XMapWindow(m_dpy, frame);

  m_clients[w] = frame;
}

void WindowManager::Unframe(Window w) {
  const Window frame = m_clients[w];

  XUnmapWindow(m_dpy, w);
  XReparentWindow(m_dpy, w, m_root, 0, 0);
  XRemoveFromSaveSet(m_dpy, w);
  XDestroyWindow(m_dpy, frame);
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