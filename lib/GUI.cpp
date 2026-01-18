#include "GUI.hpp"
#include "Application.hpp"
#include <algorithm>

namespace InWM {

Widget::Widget(Application* app, Widget* parent) 
: m_app(app), m_parent(parent) { }

void Widget::resize(int width, int height) {
  m_bounds.width = width;
  m_bounds.height = height;
}

void Widget::move(int x, int y) {
  m_bounds.x = x;
  m_bounds.y = y;
}

void Widget::setEventCallback(EventType type, EventCallback callback) {
  // Remove existing callback for this type
  auto it = std::find_if(m_callbacks.begin(), m_callbacks.end(),
                        [type](const auto& pair) { return pair.first == type; });
  if (it != m_callbacks.end()) {
      m_callbacks.erase(it);
  }
  
  m_callbacks.emplace_back(type, callback);
}

void Widget::triggerEvent(const Event& event) {
  // Call widget's handleEvent first
  handleEvent(event);
  
  // Then call registered callbacks
  for (const auto& [eventType, callback] : m_callbacks) {
    if (eventType == event.type) {
      callback(event);
    }
  }
}

void Widget::addChild(std::shared_ptr<Widget> child) {
  m_children.push_back(child);
  // Set this widget as the parent of the child
  child->m_parent = this;
}

void Widget::removeChild(Widget* child) {
  m_children.erase(
    std::remove_if(m_children.begin(), m_children.end(),
                  [child](const auto& ptr) { return ptr.get() == child; }),
    m_children.end()
  );
}

void Widget::drawRect3D(Drawable drawable, GC gc, const Rect& rect, bool raised) {
  // Fill background
  XSetForeground(m_app->getDisplay(), gc, Colors::BUTTON_FACE);
  XFillRectangle(m_app->getDisplay(), drawable, gc, 
                  rect.x, rect.y, rect.width, rect.height);
  
  // Draw 3D border
  if (raised) {
    // Light border (top, left)
    XSetForeground(m_app->getDisplay(), gc, Colors::WHITE);
    XDrawLine(m_app->getDisplay(), drawable, gc, 
              rect.x, rect.y, rect.x + rect.width - 1, rect.y);
    XDrawLine(m_app->getDisplay(), drawable, gc, 
              rect.x, rect.y, rect.x, rect.y + rect.height - 1);
    
    // Dark border (bottom, right)
    XSetForeground(m_app->getDisplay(), gc, Colors::GRAY_DARK);
    XDrawLine(m_app->getDisplay(), drawable, gc, 
              rect.x, rect.y + rect.height - 1, 
              rect.x + rect.width - 1, rect.y + rect.height - 1);
    XDrawLine(m_app->getDisplay(), drawable, gc, 
              rect.x + rect.width - 1, rect.y, 
              rect.x + rect.width - 1, rect.y + rect.height - 1);
  } else {
    // Inverted for pressed appearance
    XSetForeground(m_app->getDisplay(), gc, Colors::GRAY_DARK);
    XDrawLine(m_app->getDisplay(), drawable, gc, 
              rect.x, rect.y, rect.x + rect.width - 1, rect.y);
    XDrawLine(m_app->getDisplay(), drawable, gc, 
              rect.x, rect.y, rect.x, rect.y + rect.height - 1);
    
    XSetForeground(m_app->getDisplay(), gc, Colors::WHITE);
    XDrawLine(m_app->getDisplay(), drawable, gc, 
              rect.x, rect.y + rect.height - 1, 
              rect.x + rect.width - 1, rect.y + rect.height - 1);
    XDrawLine(m_app->getDisplay(), drawable, gc, 
              rect.x + rect.width - 1, rect.y, 
              rect.x + rect.width - 1, rect.y + rect.height - 1);
  }
}

void Widget::drawText(Drawable drawable, GC gc, const std::string& text, int x, int y) {
  XSetForeground(m_app->getDisplay(), gc, Colors::BLACK);
  XDrawString(m_app->getDisplay(), drawable, gc, x, y, text.c_str(), text.length());
}

} // namespace InWM