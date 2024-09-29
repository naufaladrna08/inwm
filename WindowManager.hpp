#ifndef WM_HPP
#define WM_HPP

extern "C" {
  #include <X11/Xlib.h>
}
#include <memory>
#include <unordered_map>
#include <iostream>

struct Vector2D {
  int x;
  int y;
};

struct Client {
  Window m_frame;
  Window m_client;
  Window m_closeButton;
  Window m_titlebar;
  std::string m_title;
  bool m_isDocked;
  Vector2D m_sizeBeforeDock;
};

class WindowManager {
  public:
    /*
     * Factory method for establishing a connection to an X server and creating
     * a Window Manager instance.
     */
    static std::unique_ptr<WindowManager> Create();
    std::unordered_map<Window, Client> m_clients;

    /*
     * Disconnect from the X server.
     */
    ~WindowManager();

    void Run();

  private:
    /*
     * Invoke internally by Create()
     */
    WindowManager(Display* dpy);

    /*
     * Handle to the underlying Xlib Display struct.
     */
    Display* m_dpy;

    /*
     * Handle to root window.
     */
    const Window m_root;

    static int OnXError(Display* dpy, XErrorEvent* e);
    static int OnWMDetected(Display* dpy, XErrorEvent* e);
    static bool m_wmDetected;

    static Client s_dragWin;
    static int m_mouseX, m_mouseY;
    int m_winStartX;  // Window's initial X position when dragging starts
    int m_winStartY;  // Window's initial Y position when dragging starts
    int m_dragOffsetX;  // Offset between mouse and window X position
    int m_dragOffsetY;  // Offset between mouse and window Y position
    
    /* Event handlers */
    void OnCreateNotify(const XCreateWindowEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnReparentNotify(const XReparentEvent& e);
    void OnConfigureRequest(const XConfigureRequestEvent& e);
    void OnMapRequest(const XMapRequestEvent& e);
    void OnMapNotify(const XMapEvent& e);
    void OnUnmapNotify(const XUnmapEvent& e);
    void OnConfigureNotify(const XConfigureEvent& e);
    void OnButtonPressNotify(const XButtonEvent& e);
    void OnButtonReleaseNotify(const XButtonEvent& e);
    void OnMotionNotify(const XMotionEvent& e);

    void Frame(Window w, const std::string& title = "");
    void Unframe(Window w);
};

#endif
