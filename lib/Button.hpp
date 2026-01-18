#ifndef INWM_BUTTON_HPP
#define INWM_BUTTON_HPP

#include "GUI.hpp"

namespace InWM {

class Button : public Widget {
  public:
    Button(Application* app, Widget* parent = nullptr);
    Button(Application* app, const std::string& text, Widget* parent = nullptr);
    
    // Widget interface
    void draw(Drawable drawable, GC gc) override;
    void handleEvent(const Event& event) override;
    
    // Button specific
    void setText(const std::string& text) { m_text = text; }
    const std::string& getText() const { return m_text; }
    
    void setPressed(bool pressed) { m_pressed = pressed; }
    bool isPressed() const { return m_pressed; }

  private:
    std::string m_text;
    bool m_pressed = false;
    bool m_hover = false;
};

} // namespace InWM

#endif