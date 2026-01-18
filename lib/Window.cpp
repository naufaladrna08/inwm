#include "Window.hpp"
#include "Application.hpp"
#include "Dropdown.hpp"
#include <algorithm>

namespace InWM {

Window::Window(Application* app, const std::string& title, int width, int height)
    : Widget(app, nullptr), m_title(title) {
    
    setBounds(Rect(100, 100, width, height)); // Default position
    createXWindow();
}

Window::~Window() {
    if (m_xwindow) {
        XDestroyWindow(m_app->getDisplay(), m_xwindow);
    }
}

void Window::createXWindow() {
    const Rect& bounds = getBounds();
    
    m_xwindow = XCreateSimpleWindow(
        m_app->getDisplay(),
        m_app->getRoot(),
        bounds.x, bounds.y,
        bounds.width, bounds.height,
        1, Colors::GRAY_DARK, Colors::GRAY_LIGHT
    );
    
    // Set window properties
    XStoreName(m_app->getDisplay(), m_xwindow, m_title.c_str());
    
    // Set window manager hints for positioning
    XSizeHints* sizeHints = XAllocSizeHints();
    if (sizeHints) {
        sizeHints->flags = PPosition | PSize | USPosition | USSize;
        sizeHints->x = bounds.x;
        sizeHints->y = bounds.y;
        sizeHints->width = bounds.width;
        sizeHints->height = bounds.height;
        XSetWMNormalHints(m_app->getDisplay(), m_xwindow, sizeHints);
        XFree(sizeHints);
    }
    
    // Select events we want to receive
    XSelectInput(m_app->getDisplay(), m_xwindow, 
                ExposureMask | ButtonPressMask | ButtonReleaseMask | 
                PointerMotionMask | KeyPressMask | StructureNotifyMask);
}

void Window::draw(Drawable drawable, GC gc) {
    if (!m_visible) return;
    
    // Clear background
    const Rect& bounds = getBounds();
    XSetForeground(m_app->getDisplay(), gc, Colors::GRAY_LIGHT);
    XFillRectangle(m_app->getDisplay(), drawable, gc, 0, 0, bounds.width, bounds.height);
    
    // Draw all non-dropdown children first, or closed dropdowns (just the title)
    for (auto& child : getChildren()) {
        if (child->isVisible()) {
            auto dropdown = dynamic_cast<class Dropdown*>(child.get());
            if (!dropdown || !dropdown->isOpen()) {
                child->draw(drawable, gc);
            }
        }
    }
    
    // Draw open dropdown menus last (on top) 
    for (auto& child : getChildren()) {
        if (child->isVisible()) {
            auto dropdown = dynamic_cast<class Dropdown*>(child.get());
            if (dropdown && dropdown->isOpen()) {
                child->draw(drawable, gc);
            }
        }
    }
}

void Window::handleEvent(const Event& event) {
    // Default window event handling
}

void Window::show() {
    if (!m_visible) {
        XMapWindow(m_app->getDisplay(), m_xwindow);
        m_visible = true;
        setVisible(true);
    }
}

void Window::hide() {
    if (m_visible) {
        XUnmapWindow(m_app->getDisplay(), m_xwindow);
        m_visible = false;
        setVisible(false);
    }
}

void Window::setTitle(const std::string& title) {
    m_title = title;
    XStoreName(m_app->getDisplay(), m_xwindow, m_title.c_str());
}

void Window::setPosition(int x, int y) {
    Rect bounds = getBounds();
    bounds.x = x;
    bounds.y = y;
    setBounds(bounds);
    
    // Force X window position
    XMoveWindow(m_app->getDisplay(), m_xwindow, x, y);
    
    // Update window manager hints
    XSizeHints* sizeHints = XAllocSizeHints();
    if (sizeHints) {
        sizeHints->flags = PPosition | USPosition;
        sizeHints->x = x;
        sizeHints->y = y;
        XSetWMNormalHints(m_app->getDisplay(), m_xwindow, sizeHints);
        XFree(sizeHints);
    }
}

bool Window::isDropdownWidget(Widget* widget) {
    // Check if widget is a Dropdown (simple type check)
    return dynamic_cast<class Dropdown*>(widget) != nullptr;
}

bool Window::isDropdownOpen(Widget* widget) {
    auto dropdown = dynamic_cast<class Dropdown*>(widget);
    return dropdown && dropdown->isOpen();
}

void Window::closeAllDropdowns() {
    for (auto& child : getChildren()) {
        auto dropdown = dynamic_cast<class Dropdown*>(child.get());
        if (dropdown && dropdown->isOpen()) {
            dropdown->setOpen(false);
        }
    }
}

bool Window::handleX11Event(const XEvent& xevent) {
    // Check if this event belongs to our window
    ::Window eventWindow = None;
    
    switch (xevent.type) {
        case Expose:
            eventWindow = xevent.xexpose.window;
            break;
        case ButtonPress:
        case ButtonRelease:
            eventWindow = xevent.xbutton.window;
            break;
        case MotionNotify:
            eventWindow = xevent.xmotion.window;
            break;
        case KeyPress:
            eventWindow = xevent.xkey.window;
            break;
        case ConfigureNotify:
            eventWindow = xevent.xconfigure.window;
            break;
        default:
            return false;
    }
    
    if (eventWindow != m_xwindow) {
        return false;
    }
    
    // Handle the event
    switch (xevent.type) {
        case Expose:
            if (xevent.xexpose.count == 0) { // Only redraw on last expose event
                draw(m_xwindow, m_app->getDefaultGC());
            }
            break;
            
        case ButtonPress:
        case ButtonRelease: {
            Event guiEvent = convertX11Event(xevent);
            Widget* target = findWidgetAt(guiEvent.x, guiEvent.y);
            if (target) {
                guiEvent.target = target;
                target->triggerEvent(guiEvent);
            } else {
                guiEvent.target = this;
                triggerEvent(guiEvent);
            }
            break;
        }
        
        case MotionNotify: {
            Event guiEvent = convertX11Event(xevent);
            guiEvent.type = HOVER;
            Widget* target = findWidgetAt(guiEvent.x, guiEvent.y);
            if (target) {
                guiEvent.target = target;
                target->triggerEvent(guiEvent);
            }
            break;
        }
        
        case KeyPress: {
            Event guiEvent = convertX11Event(xevent);
            guiEvent.target = this;
            triggerEvent(guiEvent);
            break;
        }
        
        case ConfigureNotify:
            // Window was resized
            setBounds(Rect(xevent.xconfigure.x, xevent.xconfigure.y,
                          xevent.xconfigure.width, xevent.xconfigure.height));
            break;
    }
    
    return true;
}

Event Window::convertX11Event(const XEvent& xevent) {
    Event event = {};
    
    switch (xevent.type) {
        case ButtonPress:
            event.type = CLICK;
            event.x = xevent.xbutton.x;
            event.y = xevent.xbutton.y;
            event.button = xevent.xbutton.button;
            break;
            
        case ButtonRelease:
            event.type = CLICK;
            event.x = xevent.xbutton.x;
            event.y = xevent.xbutton.y;
            event.button = -xevent.xbutton.button; // Negative for release
            break;
            
        case MotionNotify:
            event.type = HOVER;
            event.x = xevent.xmotion.x;
            event.y = xevent.xmotion.y;
            break;
            
        case KeyPress:
            event.type = KEY_PRESS;
            event.key = XLookupKeysym((XKeyEvent*)&xevent.xkey, 0);
            break;
    }
    
    return event;
}

Widget* Window::findWidgetAt(int x, int y) {
    // Search children in reverse order (top to bottom)
    for (auto it = getChildren().rbegin(); it != getChildren().rend(); ++it) {
        auto& child = *it;
        if (child->isVisible() && child->getBounds().contains(x, y)) {
            return child.get();
        }
    }
    return nullptr;
}

} // namespace InWM