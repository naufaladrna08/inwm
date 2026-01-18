#include "Dropdown.hpp"
#include "Application.hpp"
#include "GUI.hpp"
#include "Window.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace InWM {

  Dropdown::Dropdown(Application* app, Widget* parent)
  : Widget(app, parent), m_title("Menu") { }

  Dropdown::Dropdown(Application* app, const std::string& title, Widget* parent)
  : Widget(app, parent), m_title(title) { }

  void Dropdown::draw(Drawable drawable, GC gc) {
    if (!isVisible()) return;
    
    const Rect& bounds = getBounds();
    
    // Draw dropdown title (looks like a button when not open, or pressed when open)
    drawRect3D(drawable, gc, bounds, !m_open);
    
    // Draw title text
    int textX = bounds.x + MENU_PADDING;
    int textY = bounds.y + bounds.height / 2 + 4;
    
    if (m_open) {
      textX += 1;
      textY += 1;
    }
    
    XSetForeground(m_app->getDisplay(), gc, isEnabled() ? Colors::BLACK : Colors::GRAY_DARK);
    XDrawString(m_app->getDisplay(), drawable, gc, textX, textY, m_title.c_str(), m_title.length());
    
    // Draw dropdown arrow
    int arrowX = bounds.x + bounds.width - 12;
    int arrowY = bounds.y + bounds.height / 2;
    XDrawLine(m_app->getDisplay(), drawable, gc, arrowX, arrowY - 2, arrowX + 6, arrowY - 2);
    XDrawLine(m_app->getDisplay(), drawable, gc, arrowX + 1, arrowY - 1, arrowX + 5, arrowY - 1);
    XDrawLine(m_app->getDisplay(), drawable, gc, arrowX + 2, arrowY, arrowX + 4, arrowY);
    XDrawLine(m_app->getDisplay(), drawable, gc, arrowX + 3, arrowY + 1, arrowX + 3, arrowY + 1);
    
    // Draw dropdown menu if open
    if (m_open) {
      drawMenu(drawable, gc);
    }
    
    // Draw children
    for (auto& child : getChildren()) {
      if (child->isVisible()) {
        child->draw(drawable, gc);
      }
    }
  }

  void Dropdown::drawMenu(Drawable drawable, GC gc) {
    int menuWidth = getMenuWidth();
    int menuHeight = getMenuHeight();
    
    // Menu position (below dropdown or at specified position)
    int menuX = (m_menuX != 0) ? m_menuX : getBounds().x;
    int menuY = (m_menuY != 0) ? m_menuY : getBounds().y + getBounds().height;
    
    // Ensure coordinates are reasonable
    if (menuX < 0) menuX = 0;
    if (menuY < 0) menuY = getBounds().y + getBounds().height;
    
    Rect menuRect(menuX, menuY, menuWidth, menuHeight);
    
    // Draw menu background with 3D border
    XSetForeground(m_app->getDisplay(), gc, Colors::GRAY_LIGHT);
    XFillRectangle(m_app->getDisplay(), drawable, gc, 
                   menuRect.x, menuRect.y, menuRect.width, menuRect.height);
    
    // Draw menu border
    XSetForeground(m_app->getDisplay(), gc, Colors::BLACK);
    XDrawRectangle(m_app->getDisplay(), drawable, gc, 
                   menuRect.x, menuRect.y, menuRect.width - 1, menuRect.height - 1);
    
    // Draw menu items
    int itemY = menuY + MENU_PADDING;
    
    for (size_t i = 0; i < m_items.size(); ++i) {
        const auto& item = m_items[i];
        
        if (item.isSeparator) {
            // Draw separator line
            int sepY = itemY + SEPARATOR_HEIGHT / 2;
            XSetForeground(m_app->getDisplay(), gc, Colors::GRAY_DARK);
            XDrawLine(m_app->getDisplay(), drawable, gc, 
                     menuX + MENU_PADDING, sepY, 
                     menuX + menuWidth - MENU_PADDING, sepY);
            itemY += SEPARATOR_HEIGHT;
        } else {
            Rect itemRect(menuX + 1, itemY, menuWidth - 2, ITEM_HEIGHT);
            
            // Highlight selected item
            if ((int)i == m_selectedItem && item.enabled) {
                XSetForeground(m_app->getDisplay(), gc, Colors::BLUE);
                XFillRectangle(m_app->getDisplay(), drawable, gc, 
                              itemRect.x, itemRect.y, itemRect.width, itemRect.height);
            }
            
            // Draw item text
            XSetForeground(m_app->getDisplay(), gc, 
                          item.enabled ? 
                          ((int)i == m_selectedItem ? Colors::WHITE : Colors::BLACK) : 
                          Colors::GRAY_DARK);
            
            XDrawString(m_app->getDisplay(), drawable, gc, 
                       itemRect.x + MENU_PADDING, itemRect.y + 14, 
                       item.text.c_str(), item.text.length());
            
            itemY += ITEM_HEIGHT;
        }
    }
  }

  void Dropdown::handleEvent(const Event& event) {
    if (!isEnabled()) return;
    
    const Rect& bounds = getBounds();
    
    switch (event.type) {
      case CLICK:
        if (event.button > 0) { // Button press
          if (bounds.contains(event.x, event.y)) {
            // Toggle dropdown - but make sure coordinates are correct
            setOpen(!m_open);
            return; // Don't process further
          } else if (m_open) {
            // Click outside dropdown title - check if in menu area
            int menuWidth = getMenuWidth();
            int menuHeight = getMenuHeight();
            int menuX = (m_menuX != 0) ? m_menuX : bounds.x;
            int menuY = (m_menuY != 0) ? m_menuY : bounds.y + bounds.height;
            
            Rect menuRect(menuX, menuY, menuWidth, menuHeight);
            
            // Debug: ensure menu coordinates are reasonable
            if (menuX >= 0 && menuY >= 0 && menuRect.contains(event.x, event.y)) {
                // Click in menu - find which item
                int itemY = menuY + MENU_PADDING;
                
                for (size_t i = 0; i < m_items.size(); ++i) {
                    const auto& item = m_items[i];
                    
                    if (item.isSeparator) {
                        itemY += SEPARATOR_HEIGHT;
                    } else {
                        if (event.y >= itemY && event.y < itemY + ITEM_HEIGHT && item.enabled) {
                            selectItem(i);
                            return;
                        }
                        itemY += ITEM_HEIGHT;
                    }
                }
            } else {
                // Click outside menu - close it
                setOpen(false);
            }
          }
        }
        break;
          
      case HOVER:
        if (m_open) {
          // Update selected item based on mouse position
          int menuWidth = getMenuWidth();
          int menuHeight = getMenuHeight();
          int menuX = (m_menuX != 0) ? m_menuX : bounds.x;
          int menuY = (m_menuY != 0) ? m_menuY : bounds.y + bounds.height;
          
          Rect menuRect(menuX, menuY, menuWidth, menuHeight);
          
          // Only process hover if coordinates are valid
          if (menuX >= 0 && menuY >= 0 && menuRect.contains(event.x, event.y)) {
            int itemY = menuY + MENU_PADDING;
            m_selectedItem = -1;
            
            for (size_t i = 0; i < m_items.size(); ++i) {
              const auto& item = m_items[i];
              
              if (item.isSeparator) {
                itemY += SEPARATOR_HEIGHT;
              } else {
                if (event.y >= itemY && event.y < itemY + ITEM_HEIGHT && item.enabled) {
                  m_selectedItem = i;
                  break;
                }
                itemY += ITEM_HEIGHT;
              }
            }
          } else {
            m_selectedItem = -1;
          }
        }
        break;
    }
  }

  void Dropdown::addItem(const MenuItem& item) {
    m_items.push_back(item);
  }

  void Dropdown::addItem(const std::string& text, EventCallback callback) {
    m_items.emplace_back(text, callback);
  }

  void Dropdown::addSeparator() {
    m_items.push_back(MenuItem::createSeparator());
  }

  void Dropdown::clearItems() {
    m_items.clear();
  }

  void Dropdown::setOpen(bool open) {
    if (m_open != open) {
      // If opening this dropdown, close all others first
      if (open && m_parent) {
        printf("Msuk sini ga ya? Kalau iya berarti parentnya ada dong\n");
        // Find parent window and close all its dropdowns
        Widget* currentParent = m_parent;
        while (currentParent && currentParent->getParent()) {
          currentParent = currentParent->getParent();
        }
        
        // If we found the window, close all its dropdowns
        auto window = dynamic_cast<class Window*>(currentParent);
        if (window) {
          window->closeAllDropdowns();
        }
      }
      
      m_open = open;
      m_selectedItem = -1;

      // Force an Expose event 
      if (m_parent) {
        printf("Help\n");
        Widget* current = m_parent;
        while (current->getParent()) {
          current = current->getParent();
        }

        auto window = dynamic_cast<Window*>(current);
        if (window) {
          ::Window xid = window->getXWindow();

          XEvent event;
          memset(&event, 0, sizeof(event));
          event.type = Expose;
          event.xexpose.window = xid;
          event.xexpose.x = 0;
          event.xexpose.y = 0;
          event.xexpose.width = window->getBounds().width;
          event.xexpose.height = window->getBounds().height;
          event.xexpose.count = 0;

          XSendEvent(m_app->getDisplay(), xid, False, ExposureMask, &event);
          XFlush(m_app->getDisplay());
        }
      }
    }
  }

  int Dropdown::getMenuHeight() const {
    int height = MENU_PADDING * 2;
    
    for (const auto& item : m_items) {
      height += item.isSeparator ? SEPARATOR_HEIGHT : ITEM_HEIGHT;
    }
    
    return height;
  }

  int Dropdown::getMenuWidth() const {
    int maxWidth = 120; // Minimum width
    
    for (const auto& item : m_items) {
      if (!item.isSeparator) {
        int itemWidth = item.text.length() * 8 + MENU_PADDING * 2;
        maxWidth = std::max(maxWidth, itemWidth);
      }
    }
    
    return maxWidth;
  }

  void Dropdown::selectItem(int index) {
    if (index >= 0 && index < (int)m_items.size() && 
      !m_items[index].isSeparator && m_items[index].enabled) {
      
      setOpen(false);
      
      // Execute callback if available
      if (m_items[index].callback) {
        Event event = {};
        event.type = CLICK;
        event.target = this;
        m_items[index].callback(event);
      }
    }
  }

  bool Dropdown::isPointInMenu(int x, int y) const {
    if (!m_open) return false;
    
    const Rect& bounds = getBounds();
    int menuWidth = getMenuWidth();
    int menuHeight = getMenuHeight();
    int menuX = (m_menuX != 0) ? m_menuX : bounds.x;
    int menuY = (m_menuY != 0) ? m_menuY : bounds.y + bounds.height;
    
    if (menuX < 0 || menuY < 0) return false;
    
    Rect menuRect(menuX, menuY, menuWidth, menuHeight);
    return menuRect.contains(x, y);
  }

} // namespace InWM