#ifndef WM_HPP
#define WM_HPP

extern "C" {
  #include <X11/Xlib.h>
}
#include <memory>
#include <unordered_map>

class WindowManager {
  public:
    /*
     * Factory method for establishing a connection to an X server and creating
     * a Window Manager instance.
     */
    static std::unique_ptr<WindowManager> Create();
    std::unordered_map<Window, Window> m_clients;

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
    
    /* Event handlers */
    void OnCreateNotify(const XCreateWindowEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnReparentNotify(const XReparentEvent& e);
    void OnConfigureRequest(const XConfigureRequestEvent& e);
    void OnMapRequest(const XMapRequestEvent& e);
    void OnMapNotify(const XMapEvent& e);
    void OnUnmapNotify(const XUnmapEvent& e);
    void OnConfigureNotify(const XConfigureEvent& e);

    void Frame(Window w);
    void Unframe(Window w);
};

#endif
