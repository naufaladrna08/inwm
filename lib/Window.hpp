#ifndef INWM_WINDOW_HPP
#define INWM_WINDOW_HPP

#include "GUI.hpp"

namespace InWM {

// Forward declarations
class Dropdown;

class Window : public Widget {
public:
    Window(Application* app, const std::string& title, int width, int height);
    ~Window();

    // Widget interface
    void draw(Drawable drawable, GC gc) override;
    void handleEvent(const Event& event) override;
    
    // Window specific methods
    void show();
    void hide();
    void setTitle(const std::string& title);
    const std::string& getTitle() const { return m_title; }
    
    // Force window position
    void setPosition(int x, int y);
    
    // X11 event handling
    bool handleX11Event(const XEvent& xevent);
    
    ::Window getXWindow() const { return m_xwindow; }

    // Dropdown management
    void closeAllDropdowns();
    
private:
    void createXWindow();
    Event convertX11Event(const XEvent& xevent);
    Widget* findWidgetAt(int x, int y);
    bool isDropdownWidget(Widget* widget);
    bool isDropdownOpen(Widget* widget);
    
    std::string m_title;
    ::Window m_xwindow;
    bool m_visible = false;
};

} // namespace InWM

#endif