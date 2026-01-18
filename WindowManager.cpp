#include "WindowManager.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <cstdio>
#include <algorithm>
extern "C" {
  #include <X11/keysym.h>
  #include <X11/fonts/font.h>
}
using std::unique_ptr;

bool WindowManager::m_wmDetected;
Client WindowManager::s_dragWin;
Client WindowManager::s_resizeWin;
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
  m_root(DefaultRootWindow(m_dpy)),
  m_isResizing(false) {
  
  // Get screen dimensions
  Screen* screen = DefaultScreenOfDisplay(m_dpy);
  m_screenWidth = WidthOfScreen(screen);
  m_screenHeight = HeightOfScreen(screen);
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
  XSelectInput(m_dpy, m_root, SubstructureRedirectMask | SubstructureNotifyMask | 
               PointerMotionMask | ButtonPressMask | KeyPressMask);

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
      case KeyPress:
        OnKeyPressNotify(e.xkey);
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

  // System 7 colors and dimensions
  const unsigned int BORDER_WIDTH = 2;  // Thicker border for 3D effect
  const unsigned long OUTER_BORDER_COLOR = 0x000000;  // Black outer border
  const unsigned long INNER_BORDER_COLOR = 0xFFFFFF;  // White inner border
  const unsigned long BACKGROUND_COLOR = 0xC0C0C0;    // Light gray background
  
  // Title bar dimensions and colors
  const unsigned int TITLEBAR_HEIGHT = 22;  // System 7 had slightly thinner title bars
  const unsigned long TITLEBAR_TOP_COLOR = 0xFFFFFF;    // White highlight at top
  const unsigned long TITLEBAR_MID_COLOR = 0x808080;    // Gray middle
  const unsigned long TITLEBAR_BOTTOM_COLOR = 0x000000; // Black shadow at bottom
  
  // Close button - System 7 used a hollow circle with dot in center
  const unsigned int CLOSE_BUTTON_SIZE = 12;
  const unsigned long CLOSE_BUTTON_COLOR = 0x000000;    // Black circle
  const unsigned long CLOSE_BUTTON_BG = 0xFFFFFF;       // White background
  
  // Zoom (maximize) button - optional for System 7
  const unsigned int ZOOM_BUTTON_SIZE = 12;
  
  // Resize handle
  const unsigned int RESIZE_HANDLE_SIZE = 12;
  const unsigned long RESIZE_HANDLE_COLOR = 0x404040;

  XWindowAttributes xattr;
  XGetWindowAttributes(m_dpy, w, &xattr);

  // Calculate total window size including borders and title bar
  int totalWidth = xattr.width + (BORDER_WIDTH * 2);
  int totalHeight = xattr.height + TITLEBAR_HEIGHT + (BORDER_WIDTH * 2);

  // Create the main frame window with black outer border
  Window frame = XCreateSimpleWindow(m_dpy, m_root, 
                                    xattr.x, xattr.y,
                                    totalWidth, totalHeight,
                                    0,  // No border on frame - we'll draw our own
                                    0, 0);  // Temporary colors

  // Create a black border window for the outer edge
  Window outerBorder = XCreateSimpleWindow(m_dpy, frame, 
                                          0, 0, 
                                          totalWidth, totalHeight,
                                          0, OUTER_BORDER_COLOR, OUTER_BORDER_COLOR);

  // Create inner white border (creates 3D effect)
  Window innerBorder = XCreateSimpleWindow(m_dpy, outerBorder,
                                          BORDER_WIDTH - 1, BORDER_WIDTH - 1,
                                          totalWidth - (BORDER_WIDTH - 1) * 2,
                                          totalHeight - (BORDER_WIDTH - 1) * 2,
                                          0, INNER_BORDER_COLOR, INNER_BORDER_COLOR);

  // Create title bar with gradient effect
  Window titlebar = XCreateSimpleWindow(m_dpy, innerBorder,
                                        BORDER_WIDTH - 1, BORDER_WIDTH - 1,
                                        xattr.width, TITLEBAR_HEIGHT,
                                        0, TITLEBAR_MID_COLOR, TITLEBAR_MID_COLOR);

  // Create the top white highlight stripe on title bar
  Window titlebarHighlight = XCreateSimpleWindow(m_dpy, titlebar,
                                                  0, 0,
                                                  xattr.width, 1,
                                                  0, TITLEBAR_TOP_COLOR, TITLEBAR_TOP_COLOR);

  // Create the bottom black shadow stripe on title bar
  Window titlebarShadow = XCreateSimpleWindow(m_dpy, titlebar,
                                              0, TITLEBAR_HEIGHT - 1,
                                              xattr.width, 1,
                                              0, TITLEBAR_BOTTOM_COLOR, TITLEBAR_BOTTOM_COLOR);

  // Create close button (hollow circle design)
  Window closeButton = XCreateSimpleWindow(
      m_dpy, titlebar,
      8, (TITLEBAR_HEIGHT - CLOSE_BUTTON_SIZE) / 2,
      CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE,
      1, CLOSE_BUTTON_COLOR, CLOSE_BUTTON_BG
  );

  // Create zoom button (optional - System 7 had these)
  Window zoomButton = XCreateSimpleWindow(
      m_dpy, titlebar,
      xattr.width - 8 - ZOOM_BUTTON_SIZE, (TITLEBAR_HEIGHT - ZOOM_BUTTON_SIZE) / 2,
      ZOOM_BUTTON_SIZE, ZOOM_BUTTON_SIZE,
      1, CLOSE_BUTTON_COLOR, CLOSE_BUTTON_BG
  );

  // Create resize handle (bottom-right corner)
  Window resizeHandle = XCreateSimpleWindow(
      m_dpy, innerBorder,
      xattr.width - RESIZE_HANDLE_SIZE + (BORDER_WIDTH - 1),
      xattr.height + (BORDER_WIDTH - 1) - RESIZE_HANDLE_SIZE,
      RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE,
      0, RESIZE_HANDLE_COLOR, RESIZE_HANDLE_COLOR
  );

  // Set background for main content area
  XSetWindowBackground(m_dpy, innerBorder, BACKGROUND_COLOR);

  // Select input masks
  XSelectInput(m_dpy, frame, SubstructureRedirectMask | SubstructureNotifyMask);
  XSelectInput(m_dpy, titlebar, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
  XSelectInput(m_dpy, closeButton, ButtonPressMask);
  XSelectInput(m_dpy, zoomButton, ButtonPressMask);
  XSelectInput(m_dpy, resizeHandle, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);

  // Map all windows
  XMapWindow(m_dpy, frame);
  XMapWindow(m_dpy, outerBorder);
  XMapWindow(m_dpy, innerBorder);
  XMapWindow(m_dpy, titlebar);
  XMapWindow(m_dpy, titlebarHighlight);
  XMapWindow(m_dpy, titlebarShadow);
  XMapWindow(m_dpy, closeButton);
  XMapWindow(m_dpy, zoomButton);
  XMapWindow(m_dpy, resizeHandle);

  // Reparent the client window
  XReparentWindow(m_dpy, w, innerBorder, 
                  BORDER_WIDTH - 1, 
                  TITLEBAR_HEIGHT + (BORDER_WIDTH - 1));
  XAddToSaveSet(m_dpy, w);

  // Set window title
  XStoreName(m_dpy, frame, title.c_str());
  
  // Set up title text drawing (you'll need to handle this in expose events)
  setupTitleText(titlebar, title);

  // Raise and focus
  XRaiseWindow(m_dpy, frame);
  XSetInputFocus(m_dpy, w, RevertToPointerRoot, CurrentTime);

  // Store client info
  m_clients[w] = {
    // frame, w, closeButton, zoomButton, titlebar, resizeHandle, title,
    // NONE, { xattr.width, xattr.height }, { xattr.x, xattr.y }
    .m_frame = frame,
    .m_client = w,
    .m_closeButton = closeButton,
    .m_titlebar = titlebar,
    .m_resizeHandle = resizeHandle,
    .m_title = title,
    .m_snapState = NONE,
    .m_sizeBeforeSnap = { xattr.width, xattr.height },
    .m_posBeforeSnap = { xattr.x, xattr.y }
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
  // Find which client this window belongs to and raise it
  Client* clickedClient = nullptr;
  for (const auto& [clientWindow, client] : m_clients) {
    if (e.window == client.m_frame || 
        e.window == client.m_titlebar || 
        e.window == client.m_closeButton || 
        e.window == client.m_resizeHandle) {
      clickedClient = const_cast<Client*>(&client);
      // XRaiseWindow(m_dpy, client.m_frame);

      // Raise in correct order from bottom to top
      XRaiseWindow(m_dpy, clickedClient->m_client);
      XRaiseWindow(m_dpy, clickedClient->m_titlebar);
      XRaiseWindow(m_dpy, clickedClient->m_closeButton);
      XRaiseWindow(m_dpy, clickedClient->m_resizeHandle);
      
      // Make sure the client window itself is also raised
      XRaiseWindow(m_dpy, clickedClient->m_client);
      
      // Force a redraw
      XClearArea(m_dpy, clickedClient->m_titlebar, 0, 0, 0, 0, True); // Clears and triggers Expose
      break;
    }
  }

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
      XFlush(m_dpy);
      return;
    }

    if (e.window == client.m_titlebar) {
      s_dragWin = client;
      m_mouseX = e.x_root;
      m_mouseY = e.y_root;

      XWindowAttributes attr;
      XGetWindowAttributes(m_dpy, client.m_frame, &attr);

      m_winStartX = attr.x;
      m_winStartY = attr.y;
      m_dragOffsetX = m_mouseX - m_winStartX;
      m_dragOffsetY = m_mouseY - m_winStartY;

      printf("Started dragging window.\n");
      return;
    }

    if (e.window == client.m_resizeHandle) {
      s_resizeWin = client;
      m_isResizing = true;
      m_mouseX = e.x_root;
      m_mouseY = e.y_root;

      XWindowAttributes attr;
      XGetWindowAttributes(m_dpy, client.m_frame, &attr);
      m_winStartX = attr.width;
      m_winStartY = attr.height;

      printf("Started resizing window.\n");
      return;
    }
  }
}

void WindowManager::OnButtonReleaseNotify(const XButtonEvent& e) {
  if (e.button == Button1) {
    // Handle snap zones when releasing drag
    if (s_dragWin.m_frame != None) {
      Client* client = FindClientByFrame(s_dragWin.m_frame);
      if (client && client->m_snapState == NONE) {
        // Check for snap zones
        if (e.x_root < 50) {
          SnapWindow(client->m_client, LEFT_SNAP);
        } else if (e.x_root > m_screenWidth - 50) {
          SnapWindow(client->m_client, RIGHT_SNAP);
        } else if (e.y_root < 10) {
          SnapWindow(client->m_client, MAXIMIZED);
        }
      }
      s_dragWin = {};
      printf("Stopped dragging window.\n");
    }
    
    if (m_isResizing && s_resizeWin.m_frame != None) {
      m_isResizing = false;
      s_resizeWin = {};
      printf("Stopped resizing window.\n");
    }
  }
}

void WindowManager::OnMotionNotify(const XMotionEvent& e) {
  if (e.state & Button1Mask) {
    if (s_dragWin.m_frame != None) {
      Client* client = FindClientByFrame(s_dragWin.m_frame);
      if (client) {
        // If window is snapped, restore it when starting to drag
        if (client->m_snapState != NONE) {
          RestoreWindow(client->m_client);
          
          // Recalculate drag offset for restored window
          XWindowAttributes attr;
          XGetWindowAttributes(m_dpy, client->m_frame, &attr);
          m_dragOffsetX = attr.width / 2;  // Center under cursor
          m_dragOffsetY = 12;  // Titlebar center
        }
        
        // Calculate new position
        int newX = e.x_root - m_dragOffsetX;
        int newY = e.y_root - m_dragOffsetY;
        
        // Keep window on screen
        newX = std::max(0, std::min(newX, m_screenWidth - 100));
        newY = std::max(0, std::min(newY, m_screenHeight - 50));
        
        XMoveWindow(m_dpy, s_dragWin.m_frame, newX, newY);
      }
    }
    
    if (m_isResizing && s_resizeWin.m_frame != None) {
      int deltaX = e.x_root - m_mouseX;
      int deltaY = e.y_root - m_mouseY;
      
      int newWidth = std::max(100, m_winStartX + deltaX);
      int newHeight = std::max(80, m_winStartY + deltaY);
      
      XResizeWindow(m_dpy, s_resizeWin.m_frame, newWidth, newHeight);
      XResizeWindow(m_dpy, s_resizeWin.m_client, newWidth, newHeight - 24);
      XResizeWindow(m_dpy, s_resizeWin.m_titlebar, newWidth, 24);
      
      // Move resize handle to new position
      XMoveWindow(m_dpy, s_resizeWin.m_resizeHandle, 
                 newWidth - 12, newHeight - 12);
    }
  }
}

void WindowManager::OnKeyPressNotify(const XKeyEvent& e) {
  KeySym key = XLookupKeysym((XKeyEvent*)&e, 0);
  
  // Super (Windows) key shortcuts for window snapping
  if (e.state & Mod4Mask) {  // Mod4 is typically the Super/Windows key
    Window focused;
    int revert;
    XGetInputFocus(m_dpy, &focused, &revert);
    
    Client* client = FindClientByFrame(focused);
    if (!client) return;
    
    switch (key) {
      case XK_Left:
        SnapWindow(client->m_client, LEFT_SNAP);
        break;
      case XK_Right: 
        SnapWindow(client->m_client, RIGHT_SNAP);
        break;
      case XK_Up:
        SnapWindow(client->m_client, MAXIMIZED);
        break;
      case XK_Down:
        RestoreWindow(client->m_client);
        break;
    }
  }
}

void WindowManager::SnapWindow(Window clientWindow, SnapState state) {
  if (!m_clients.count(clientWindow)) return;
  
  Client& client = m_clients[clientWindow];
  
  // Save current state before snapping
  if (client.m_snapState == NONE) {
    XWindowAttributes attr;
    XGetWindowAttributes(m_dpy, client.m_frame, &attr);
    client.m_sizeBeforeSnap = { attr.width, attr.height };
    client.m_posBeforeSnap = { attr.x, attr.y };
  }
  
  int newX, newY, newWidth, newHeight;
  
  switch (state) {
    case LEFT_SNAP:
      newX = 0;
      newY = 0; 
      newWidth = m_screenWidth / 2;
      newHeight = m_screenHeight;
      break;
    case RIGHT_SNAP:
      newX = m_screenWidth / 2;
      newY = 0;
      newWidth = m_screenWidth / 2;  
      newHeight = m_screenHeight;
      break;
    case MAXIMIZED:
      newX = 0;
      newY = 0;
      newWidth = m_screenWidth;
      newHeight = m_screenHeight;
      break;
    default:
      return;
  }
  
  client.m_snapState = state;
  
  XMoveResizeWindow(m_dpy, client.m_frame, newX, newY, newWidth, newHeight);
  XResizeWindow(m_dpy, client.m_client, newWidth, newHeight - 24);
  XResizeWindow(m_dpy, client.m_titlebar, newWidth, 24);
  
  // Update close button position
  XMoveWindow(m_dpy, client.m_closeButton, 6, 5);
  
  // Move resize handle to new position
  XMoveWindow(m_dpy, client.m_resizeHandle, newWidth - 12, newHeight - 12);
  
  printf("Snapped window to %s\n", 
         state == LEFT_SNAP ? "left" : 
         state == RIGHT_SNAP ? "right" : "maximized");
}

void WindowManager::RestoreWindow(Window clientWindow) {
  if (!m_clients.count(clientWindow)) return;
  
  Client& client = m_clients[clientWindow];
  
  if (client.m_snapState == NONE) return;
  
  client.m_snapState = NONE;
  
  XMoveResizeWindow(m_dpy, client.m_frame, 
                   client.m_posBeforeSnap.x, client.m_posBeforeSnap.y,
                   client.m_sizeBeforeSnap.x, client.m_sizeBeforeSnap.y);
  
  XResizeWindow(m_dpy, client.m_client, 
               client.m_sizeBeforeSnap.x, client.m_sizeBeforeSnap.y - 24);
  XResizeWindow(m_dpy, client.m_titlebar, client.m_sizeBeforeSnap.x, 24);
  
  // Update close button position  
  XMoveWindow(m_dpy, client.m_closeButton, 6, 5);
  
  // Move resize handle to restored position
  XMoveWindow(m_dpy, client.m_resizeHandle, 
             client.m_sizeBeforeSnap.x - 12, client.m_sizeBeforeSnap.y - 12);
  
  printf("Restored window to original size\n");
}

Client* WindowManager::FindClientByFrame(Window frame) {
  for (auto& [clientWindow, client] : m_clients) {
    if (client.m_frame == frame) {
      return &client;
    }
  }
  return nullptr;
}

Client* WindowManager::FindClientByWindow(Window window) {
  if (m_clients.count(window)) {
    return &m_clients[window];
  }
  return nullptr;
}

void WindowManager::setupTitleText(Window titlebar, const std::string& title) {
  // Create a graphics context for the title bar
  GC gc = XCreateGC(m_dpy, titlebar, 0, nullptr);
  
  // System 7 used Chicago font (use a sans-serif substitute)
  Font font = XLoadFont(m_dpy, "-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*");
  XSetFont(m_dpy, gc, font);
  
  // Set colors: black text with white drop shadow for 3D effect
  XSetForeground(m_dpy, gc, 0xFFFFFF);  // White shadow
  XDrawString(m_dpy, titlebar, gc, 33, 15, title.c_str(), title.length());
  
  XSetForeground(m_dpy, gc, 0x000000);  // Black text
  XDrawString(m_dpy, titlebar, gc, 32, 14, title.c_str(), title.length());
  
  // XFreeFont(m_dpy, font);
  XFreeGC(m_dpy, gc);
}