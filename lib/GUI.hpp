#ifndef INWM_GUI_HPP
#define INWM_GUI_HPP

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}
#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace InWM {

// Forward declarations
class Widget;
class Application;

// System 8 color scheme
struct Colors {
  static const unsigned long GRAY_LIGHT = 0xC0C0C0;    // Light gray background
  static const unsigned long GRAY_DARK = 0x808080;     // Dark gray border/shadow
  static const unsigned long WHITE = 0xFFFFFF;         // White highlights
  static const unsigned long BLACK = 0x000000;         // Black text
  static const unsigned long BLUE = 0x0000FF;          // Selection blue
  static const unsigned long BUTTON_FACE = 0xC0C0C0;   // Button face color
};

// Event types for widgets
enum EventType {
  CLICK,
  HOVER,
  FOCUS,
  UNFOCUS,
  KEY_PRESS
};

// Event data structure
struct Event {
  EventType type;
  int x, y;           // Mouse coordinates
  int button;         // Mouse button (if applicable)
  KeySym key;         // Key pressed (if applicable)
  Widget* target;     // Widget that received the event
};

// Callback function type
using EventCallback = std::function<void(const Event&)>;

// Base rectangle structure
struct Rect {
  int x, y, width, height;
  
  Rect(int x = 0, int y = 0, int w = 0, int h = 0) 
    : x(x), y(y), width(w), height(h) {}
  
  bool contains(int px, int py) const {
    return px >= x && px < x + width && py >= y && py < y + height;
  }
};

// Base widget class
class Widget {
  public:
    Widget(Application* app, Widget* parent = nullptr);
    virtual ~Widget() = default;

    // Core methods
    virtual void draw(Drawable drawable, GC gc) = 0;
    virtual void handleEvent(const Event& event) {}
    virtual void resize(int width, int height);
    virtual void move(int x, int y);
    
    // Properties
    void setBounds(const Rect& bounds) { m_bounds = bounds; }
    const Rect& getBounds() const { return m_bounds; }
    
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    
    // Event handling
    void setEventCallback(EventType type, EventCallback callback);
    void triggerEvent(const Event& event);
    
    // Hierarchy
    void addChild(std::shared_ptr<Widget> child);
    void removeChild(Widget* child);
    const std::vector<std::shared_ptr<Widget>>& getChildren() const { return m_children; }
    Widget* getParent() const { return m_parent; }
    
    // Drawing helpers
    void drawRect3D(Drawable drawable, GC gc, const Rect& rect, bool raised = true);
    void drawText(Drawable drawable, GC gc, const std::string& text, int x, int y);

  protected:
    Application* m_app;
    Widget* m_parent;
    Rect m_bounds;
    bool m_visible = true;
    bool m_enabled = true;
    std::vector<std::shared_ptr<Widget>> m_children;
    std::vector<std::pair<EventType, EventCallback>> m_callbacks;
};

} // namespace InWM

#endif