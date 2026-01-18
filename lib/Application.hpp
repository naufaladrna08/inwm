#ifndef INWM_APPLICATION_HPP
#define INWM_APPLICATION_HPP

#include "GUI.hpp"
#include <memory>

namespace InWM {

class Window;

class Application {
public:
    static std::unique_ptr<Application> create();
    ~Application();

    // Core methods
    void run();
    void quit() { m_running = false; }
    
    // Window management
    std::shared_ptr<Window> createWindow(const std::string& title, int width, int height);
    void closeWindow(Window* window);
    
    // X11 access
    Display* getDisplay() const { return m_display; }
    ::Window getRoot() const { return m_root; }
    GC getDefaultGC() const { return m_gc; }
    
    // Event handling
    void handleX11Event(const XEvent& xevent);

private:
    Application(Display* display);
    
    Display* m_display;
    ::Window m_root;
    GC m_gc;
    bool m_running = true;
    std::vector<std::shared_ptr<Window>> m_windows;
};

} // namespace InWM

#endif