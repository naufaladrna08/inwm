#include "WindowManager.hpp"
#include <cstdio>
using std::unique_ptr;


bool WindowManager::m_wmDetected;
Client WindowManager::s_dragWin;
int WindowManager::m_mouseX;
int WindowManager::m_mouseY;

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
  XSelectInput(m_dpy, m_root, SubstructureRedirectMask | SubstructureNotifyMask | PointerMotionMask | ButtonPressMask);

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
      case ButtonPress:
        OnButtonPressNotify(e.xbutton);
        break;
      case ButtonRelease:
        OnButtonReleaseNotify(e.xbutton);
        break;
      case MotionNotify:
        OnMotionNotify(e.xmotion);
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
    Window frame = m_clients[e.window].m_frame;
    XConfigureWindow(m_dpy, frame, e.value_mask, &changes);
  }

  XConfigureWindow(m_dpy, e.window, e.value_mask, &changes);
  printf("Resize window %d, %d\n", e.width, e.height);
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
  Frame(e.window, "Hello, World!");
  XMapWindow(m_dpy, e.window);
}

void WindowManager::Frame(Window w, const std::string& title) {
  Atom wm_delete = XInternAtom(m_dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(m_dpy, w, &wm_delete, 1);

  const unsigned int BORDER_WIDTH = 3;
  const unsigned long BORDER_COLOR = 0xffffff;
  const unsigned long BACKGROUND_COLOR = 0x1ABC9C;
  const unsigned int TITLEBAR_HEIGHT = 20;
  const unsigned long TITLEBAR_COLOR = 0x34495E;
  const unsigned int CLOSE_BUTTON_SIZE = 15;
  const unsigned long CLOSE_BUTTON_COLOR = 0xE74C3C;
  const int TEXT_PADDING = 10;

  XWindowAttributes xattr;
  XGetWindowAttributes(m_dpy, w, &xattr);

  // Create the frame window, including space for the title bar
  Window frame = XCreateSimpleWindow(m_dpy, m_root, xattr.x, xattr.y, xattr.width, xattr.height + TITLEBAR_HEIGHT, BORDER_WIDTH, BORDER_COLOR, BACKGROUND_COLOR);
  Window titlebar = XCreateSimpleWindow(m_dpy, frame,  0, 0,  xattr.width, TITLEBAR_HEIGHT,  0, BORDER_COLOR, TITLEBAR_COLOR);
  Window closeButton = XCreateSimpleWindow(
    m_dpy, titlebar,
    xattr.width - CLOSE_BUTTON_SIZE - 5,
    (TITLEBAR_HEIGHT - CLOSE_BUTTON_SIZE) / 2,
    CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE,
    0, BORDER_COLOR, CLOSE_BUTTON_COLOR
  );

  Window text = XCreateSimpleWindow(
    m_dpy, titlebar,
    TEXT_PADDING, 0,
    xattr.width - CLOSE_BUTTON_SIZE - TEXT_PADDING * 2, TITLEBAR_HEIGHT,
    0, BORDER_COLOR, TITLEBAR_COLOR
  );

  XSelectInput(m_dpy, frame, SubstructureRedirectMask | SubstructureNotifyMask);
  XSelectInput(m_dpy, titlebar, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
  XSelectInput(m_dpy, closeButton, ButtonPressMask);

  XMapWindow(m_dpy, frame);
  XMapWindow(m_dpy, titlebar);
  XMapWindow(m_dpy, closeButton);
  XMapWindow(m_dpy, text);

  XReparentWindow(m_dpy, w, frame, 0, TITLEBAR_HEIGHT);
  XAddToSaveSet(m_dpy, w);

  XStoreName(m_dpy, frame, title.c_str());
  XRaiseWindow(m_dpy, frame);
  XSetInputFocus(m_dpy, frame, RevertToPointerRoot, 0);

  m_clients[w] = {
    frame, w, closeButton, titlebar, title, false, { xattr.width, xattr.height }
  };
}

void WindowManager::Unframe(Window w) {
  const Window frame = m_clients[w].m_frame;

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

void WindowManager::OnButtonPressNotify(const XButtonEvent& e) {
  for (const auto& [clientWindow, client] : m_clients) {
    if (e.window == client.m_closeButton) {
      // Send a WM_DELETE_WINDOW message to the client to request it to close
      XEvent ev;
      ev.xclient.type = ClientMessage;
      ev.xclient.window = client.m_client;
      ev.xclient.message_type = XInternAtom(m_dpy, "WM_PROTOCOLS", false);
      ev.xclient.format = 32;
      ev.xclient.data.l[0] = XInternAtom(m_dpy, "WM_DELETE_WINDOW", false);
      ev.xclient.data.l[1] = CurrentTime;

      XSendEvent(m_dpy, client.m_client, false, NoEventMask, &ev);
      XFlush(m_dpy);  // Ensure the event is sent immediately
      return;
    }

    if (e.window == client.m_titlebar) {
      s_dragWin = client;
      m_mouseX = e.x_root;
      m_mouseY = e.y_root;

      XWindowAttributes attr;
      XGetWindowAttributes(m_dpy, client.m_frame, &attr);

      // Store window's starting position and calculate the offset
      m_winStartX = attr.x;
      m_winStartY = attr.y;
      m_dragOffsetX = m_mouseX - m_winStartX;
      m_dragOffsetY = m_mouseY - m_winStartY;

      printf("Started dragging window.\n");
      return;
    }
  }
}

void WindowManager::OnButtonReleaseNotify(const XButtonEvent& e) {
  // Stop dragging
  if (e.button == Button1 && s_dragWin.m_frame != None) {
    s_dragWin = {};
    printf("Stopped dragging window.\n");
  }
}

void WindowManager::OnMotionNotify(const XMotionEvent& e) {
  if (e.state & Button1Mask && s_dragWin.m_frame != None) {
    // Calculate the new window position by subtracting the drag offset
    int newX = e.x_root - m_dragOffsetX;
    int newY = e.y_root - m_dragOffsetY;

    // Move the window to the new position
    XMoveWindow(m_dpy, s_dragWin.m_frame, newX, newY);
    
    // Dock window to the left and right 1280x720
    printf("Mouse position: %d, %d\n", e.x_root, e.y_root);
    if (e.x_root < 10 && !s_dragWin.m_isDocked) {
      XResizeWindow(m_dpy, s_dragWin.m_frame, 1280 / 2, 720);
      XResizeWindow(m_dpy, s_dragWin.m_client, 1280 / 2, 720);
      XResizeWindow(m_dpy, s_dragWin.m_titlebar, 1280 / 2, 20);

      XMoveWindow(m_dpy, s_dragWin.m_frame, 0, 0);
      XMoveWindow(m_dpy, s_dragWin.m_closeButton, 1280 / 2 - 20, 2);

      s_dragWin.m_isDocked = true;
    } else if (e.x_root > 1280 - 10 && !s_dragWin.m_isDocked) {
      XResizeWindow(m_dpy, s_dragWin.m_frame, 1280 / 2, 720);
      XResizeWindow(m_dpy, s_dragWin.m_client, 1280 / 2, 720);
      XResizeWindow(m_dpy, s_dragWin.m_titlebar, 1280 / 2, 20);

      XMoveWindow(m_dpy, s_dragWin.m_frame, 1280 / 2, 0);
      XMoveWindow(m_dpy, s_dragWin.m_closeButton, 1280 / 2 - 20, 2);

      s_dragWin.m_isDocked = true;
    } else if (s_dragWin.m_isDocked) {
      XResizeWindow(m_dpy, s_dragWin.m_frame, s_dragWin.m_sizeBeforeDock.x, s_dragWin.m_sizeBeforeDock.y);
      XResizeWindow(m_dpy, s_dragWin.m_client, s_dragWin.m_sizeBeforeDock.x, s_dragWin.m_sizeBeforeDock.y);
      XResizeWindow(m_dpy, s_dragWin.m_titlebar, s_dragWin.m_sizeBeforeDock.x, 20);

      XMoveWindow(m_dpy, s_dragWin.m_frame, newX, newY);
      XMoveWindow(m_dpy, s_dragWin.m_closeButton, s_dragWin.m_sizeBeforeDock.x - 20, 2);

      s_dragWin.m_isDocked = false;
    }
  }
}