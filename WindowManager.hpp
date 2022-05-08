#ifndef WM_HPP
#define WM_HPP

extern "C" {
  #include <X11/Xlib.h>
}
#include <memory>

class WindowManager {
  public:
    /*
     * Factory method for establishing a connection to an X server and creating
     * a Window Manager instance.
     */
    static std::unique_ptr<WindowManager> Create();

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
    Display* _dpy;

    /*
     * Handle to root window.
     */
    const Window _root;

    static int OnXError(Display* dpy, XErrorEvent* e);
    static int OnWMDetected(Display* dpy, XErrorEvent* e);
    static bool wm_detected_;
    
    /* Event handlers */
    void OnCreateNotify(const XCreateWindowEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnReparentNotify(const XReparentEvent& e);
};

#endif
