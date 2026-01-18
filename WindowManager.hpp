#ifndef WM_HPP
#define WM_HPP

extern "C" {
  #include <X11/Xlib.h>
}
#include <memory>
#include <unordered_map>
#include <string>

struct Vector2D {
  int x;
  int y;
};

enum SnapState {
  NONE,
  LEFT_SNAP,
  RIGHT_SNAP,
  MAXIMIZED
};

struct Client {
  Window m_frame;
  Window m_client;
  Window m_closeButton;
  Window m_titlebar;
  Window m_resizeHandle;  // Bottom-right resize handle
  std::string m_title;
  SnapState m_snapState;
  Vector2D m_sizeBeforeSnap;
  Vector2D m_posBeforeSnap;
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
    static Client s_resizeWin;
    static int m_mouseX, m_mouseY;
    int m_winStartX;  // Window's initial X position when dragging starts
    int m_winStartY;  // Window's initial Y position when dragging starts
    int m_dragOffsetX;  // Offset between mouse and window X position
    int m_dragOffsetY;  // Offset between mouse and window Y position
    int m_screenWidth, m_screenHeight;  // Screen dimensions
    bool m_isResizing;  // Track resize state
    
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
    void OnKeyPressNotify(const XKeyEvent& e);
    
    /* Helper functions */
    void SnapWindow(Window clientWindow, SnapState state);
    void RestoreWindow(Window clientWindow);
    Client* FindClientByFrame(Window frame);
    Client* FindClientByWindow(Window window);

    void Frame(Window w, const std::string& title = "");
    void Unframe(Window w);
    void setupTitleText(Window titlebar, const std::string& title);
    void drawWindowTitle(Window titlebar, const std::string& title);
};

#endif
