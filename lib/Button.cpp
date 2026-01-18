#include "Button.hpp"
#include "Application.hpp"

namespace InWM {

Button::Button(Application* app, Widget* parent)
: Widget(app, parent), m_text("Button") { }

Button::Button(Application* app, const std::string& text, Widget* parent)
: Widget(app, parent), m_text(text) { }

void Button::draw(Drawable drawable, GC gc) {
  if (!isVisible()) return;
  
  const Rect& bounds = getBounds();
  
  // Draw 3D button
  drawRect3D(drawable, gc, bounds, !m_pressed);
  
  // Calculate text position (centered)
  int textWidth = m_text.length() * 8; // Approximate character width
  int textX = bounds.x + (bounds.width - textWidth) / 2;
  int textY = bounds.y + bounds.height / 2 + 4; // Slight offset for centering
  
  // Offset text if pressed
  if (m_pressed) {
    textX += 1;
    textY += 1;
  }
  
  // Draw text
  XSetForeground(m_app->getDisplay(), gc, isEnabled() ? Colors::BLACK : Colors::GRAY_DARK);
  XDrawString(m_app->getDisplay(), drawable, gc, textX, textY, m_text.c_str(), m_text.length());
  
  // Draw children
  for (auto& child : getChildren()) {
    if (child->isVisible()) {
      child->draw(drawable, gc);
    }
  }
}

void Button::handleEvent(const Event& event) {
  if (!isEnabled()) return;
  
  switch (event.type) {
    case CLICK:
      if (event.button > 0) { // Button press
        m_pressed = true;
      } else { // Button release
        m_pressed = false;
        // Trigger click callback if released over button
        if (getBounds().contains(event.x, event.y)) {
          Event clickEvent = event;
          clickEvent.type = CLICK;
          clickEvent.target = this;
          triggerEvent(clickEvent);
        }
      }
      break;
        
    case HOVER:
      m_hover = getBounds().contains(event.x, event.y);
      break;
  }
}

} // namespace InWM