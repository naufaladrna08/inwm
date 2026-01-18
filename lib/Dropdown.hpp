#ifndef INWM_DROPDOWN_HPP
#define INWM_DROPDOWN_HPP

#include "GUI.hpp"
#include <vector>

// Forward declarations
namespace InWM { class Window; }

namespace InWM {

struct MenuItem {
    std::string text;
    bool enabled = true;
    bool isSeparator = false;
    EventCallback callback;
    
    MenuItem(const std::string& t, EventCallback cb = nullptr, bool en = true) 
        : text(t), enabled(en), callback(cb) {}
    
    static MenuItem createSeparator() {
        MenuItem item("");
        item.isSeparator = true;
        return item;
    }
};

class Dropdown : public Widget {
public:
    Dropdown(Application* app, Widget* parent = nullptr);
    Dropdown(Application* app, const std::string& title, Widget* parent = nullptr);
    
    // Widget interface
    void draw(Drawable drawable, GC gc) override;
    void handleEvent(const Event& event) override;
    
    // Dropdown specific
    void setTitle(const std::string& title) { m_title = title; }
    const std::string& getTitle() const { return m_title; }
    
    void addItem(const MenuItem& item);
    void addItem(const std::string& text, EventCallback callback = nullptr);
    void addSeparator();
    void clearItems();
    
    void setOpen(bool open);
    bool isOpen() const { return m_open; }
    
    // Position dropdown menu (for menu bar usage)
    void setMenuPosition(int x, int y) { m_menuX = x; m_menuY = y; }
    
    // Check if point is in dropdown menu area
    bool isPointInMenu(int x, int y) const;

  private:
    void drawMenu(Drawable drawable, GC gc);
    int getMenuHeight() const;
    int getMenuWidth() const;
    void selectItem(int index);
    
    std::string m_title;
    std::vector<MenuItem> m_items;
    bool m_open = false;
    int m_selectedItem = -1;
    int m_menuX = 0, m_menuY = 0;
    
    // Menu appearance
    static const int ITEM_HEIGHT = 20;
    static const int SEPARATOR_HEIGHT = 8;
    static const int MENU_PADDING = 4;
};

} // namespace InWM

#endif