#include "lib/Widgets.hpp"
#include <ctime>
#include <cstring>
#include <stdexcept>

using namespace InWM;

// Custom desktop widget to draw dot pattern
class DesktopWidget : public Widget {
public:
    DesktopWidget(Application* app, Widget* parent = nullptr) : Widget(app, parent) {}
    
    void draw(Drawable drawable, GC gc) override {
        if (!isVisible()) return;
        
        const Rect& bounds = getBounds();
        
        // Fill with desktop gray
        XSetForeground(m_app->getDisplay(), gc, 0xC0C0C0);
        XFillRectangle(m_app->getDisplay(), drawable, gc, 
                       bounds.x, bounds.y, bounds.width, bounds.height);
        
        // Draw classic Mac OS desktop dot pattern
        XSetForeground(m_app->getDisplay(), gc, 0x808080); // Darker gray for dots
        
        // Draw dot pattern - dots every 8 pixels
        for (int y = 4; y < bounds.height; y += 8) {
            for (int x = 4; x < bounds.width; x += 8) {
                XDrawPoint(m_app->getDisplay(), drawable, gc, x, y);
            }
        }
        
        // Draw some desktop icons (simple representations)
        drawDesktopIcons(drawable, gc);
    }
    
private:
    void drawDesktopIcons(Drawable drawable, GC gc) {
        // Draw "Trash" icon in bottom-right corner
        int trashX = getBounds().width - 60;
        int trashY = getBounds().height - 80;
        
        // Trash can outline
        XSetForeground(m_app->getDisplay(), gc, 0x000000);
        XDrawRectangle(m_app->getDisplay(), drawable, gc, trashX, trashY, 32, 40);
        XDrawRectangle(m_app->getDisplay(), drawable, gc, trashX - 2, trashY - 5, 36, 5);
        
        // Label
        XDrawString(m_app->getDisplay(), drawable, gc, trashX - 5, trashY + 55, "Trash", 5);
        
        // Draw "HD" icon in top-right corner
        int hdX = getBounds().width - 60;
        int hdY = 40;
        
        XDrawRectangle(m_app->getDisplay(), drawable, gc, hdX, hdY, 32, 32);
        XDrawString(m_app->getDisplay(), drawable, gc, hdX + 8, hdY + 20, "HD", 2);
        XDrawString(m_app->getDisplay(), drawable, gc, hdX - 10, hdY + 48, "Hard Disk", 9);
    }
};

class System8Bar {
  public:
    System8Bar() {
        // Create application
        m_app = Application::create();
        if (!m_app) {
            throw std::runtime_error("Failed to create bar application");
        }
        
        createDesktop();
        createBarWindow();
        createMenus();
        setupTimer();
    }
    
    void run() {
        m_desktop->show();
        m_window->show();
        
        // Explicitly control stacking order - ensure desktop stays at bottom
        XLowerWindow(m_app->getDisplay(), m_desktop->getXWindow());
        XRaiseWindow(m_app->getDisplay(), m_window->getXWindow());
        
        printf("System 8 Desktop and Menu Bar started\n");
        m_app->run();
    }

private:
    void createDesktop() {
        // Get screen dimensions
        Screen* screen = DefaultScreenOfDisplay(m_app->getDisplay());
        int screenWidth = WidthOfScreen(screen);
        int screenHeight = HeightOfScreen(screen);
        
        // Create full-screen desktop background
        m_desktop = m_app->createWindow("Desktop", screenWidth, screenHeight);
        
        // Position at screen origin using new method
        m_desktop->setPosition(0, 0);
        
        // Create custom desktop widget to draw dot pattern
        m_desktopWidget = std::make_shared<DesktopWidget>(m_app.get(), m_desktop.get());
        m_desktopWidget->setBounds(Rect(0, 0, screenWidth, screenHeight));
        m_desktop->addChild(m_desktopWidget);
    }
    
    void createBarWindow() {
        Screen* screen = DefaultScreenOfDisplay(m_app->getDisplay());
        int screenWidth = WidthOfScreen(screen);
        m_window = m_app->createWindow("InOS Menu Bar", screenWidth, 24);
        
        // Position at top-left using new method
        m_window->setPosition(0, 0);
    }
    
    void createMenus() {
        // Apple menu (leftmost)
        m_appleMenu = std::make_shared<Dropdown>(m_app.get(), "\xEF\x82\x8F"); // Apple symbol
        m_appleMenu->setBounds(Rect(5, 2, 30, 20));
        // Set menu position to appear below the dropdown
        m_appleMenu->setMenuPosition(5, 22);
        m_appleMenu->addItem("About InOS", [](const Event& e) { 
            printf("About InOS selected\n"); 
        });
        m_appleMenu->addSeparator();
        m_appleMenu->addItem("System Preferences...", [](const Event& e) { 
            printf("Opening System Preferences\n"); 
        });
        m_appleMenu->addSeparator();
        m_appleMenu->addItem("Recent Items", [](const Event& e) { 
            printf("Recent Items selected\n"); 
        });
        m_appleMenu->addSeparator();
        m_appleMenu->addItem("Sleep", [](const Event& e) { 
            printf("Sleep mode activated\n"); 
        });
        m_appleMenu->addItem("Restart", [](const Event& e) { 
            printf("Restart requested\n"); 
        });
        m_appleMenu->addItem("Shut Down", [](const Event& e) { 
            printf("Shutdown requested\n"); 
        });
        m_window->addChild(m_appleMenu);
        
        // File menu
        m_fileMenu = std::make_shared<Dropdown>(m_app.get(), "File");
        m_fileMenu->setBounds(Rect(40, 2, 40, 20));
        m_fileMenu->setMenuPosition(40, 22);
        m_fileMenu->addItem("New", [](const Event& e) { 
            printf("New file\n"); 
        });
        m_fileMenu->addItem("Open...", [](const Event& e) { 
            printf("Open file dialog\n"); 
        });
        m_fileMenu->addSeparator();
        m_fileMenu->addItem("Close", [](const Event& e) { 
            printf("Close file\n"); 
        });
        m_fileMenu->addItem("Save", [](const Event& e) { 
            printf("Save file\n"); 
        });
        m_fileMenu->addItem("Save As...", [](const Event& e) { 
            printf("Save As dialog\n"); 
        });
        m_window->addChild(m_fileMenu);
        
        // Edit menu
        m_editMenu = std::make_shared<Dropdown>(m_app.get(), "Edit");
        m_editMenu->setBounds(Rect(85, 2, 40, 20));
        m_editMenu->setMenuPosition(85, 22);
        m_editMenu->addItem("Undo", [](const Event& e) { 
            printf("Undo\n"); 
        });
        m_editMenu->addItem("Redo", [](const Event& e) { 
            printf("Redo\n"); 
        });
        m_editMenu->addSeparator();
        m_editMenu->addItem("Cut", [](const Event& e) { 
            printf("Cut\n"); 
        });
        m_editMenu->addItem("Copy", [](const Event& e) { 
            printf("Copy\n"); 
        });
        m_editMenu->addItem("Paste", [](const Event& e) { 
            printf("Paste\n"); 
        });
        m_editMenu->addSeparator();
        m_editMenu->addItem("Select All", [](const Event& e) { 
            printf("Select All\n"); 
        });
        m_window->addChild(m_editMenu);
        
        // View menu
        m_viewMenu = std::make_shared<Dropdown>(m_app.get(), "View");
        m_viewMenu->setBounds(Rect(130, 2, 40, 20));
        m_viewMenu->setMenuPosition(130, 22);
        m_viewMenu->addItem("Icon View", [](const Event& e) { 
            printf("Icon View\n"); 
        });
        m_viewMenu->addItem("List View", [](const Event& e) { 
            printf("List View\n"); 
        });
        m_viewMenu->addSeparator();
        m_viewMenu->addItem("Show Desktop", [](const Event& e) { 
            printf("Show Desktop\n"); 
        });
        m_viewMenu->addItem("Hide Desktop", [](const Event& e) { 
            printf("Hide Desktop\n"); 
        });
        m_window->addChild(m_viewMenu);
        
        // Special menu
        m_specialMenu = std::make_shared<Dropdown>(m_app.get(), "Special");
        m_specialMenu->setBounds(Rect(175, 2, 55, 20));
        m_specialMenu->setMenuPosition(175, 22);
        m_specialMenu->addItem("Clean Up Desktop", [](const Event& e) { 
            printf("Cleaning up desktop\n"); 
        });
        m_specialMenu->addItem("Empty Trash", [](const Event& e) { 
            printf("Emptying trash\n"); 
        });
        m_specialMenu->addSeparator();
        m_specialMenu->addItem("Burn Disc...", [](const Event& e) { 
            printf("Burn disc dialog\n"); 
        });
        m_specialMenu->addItem("Eject Disc", [](const Event& e) { 
            printf("Ejecting disc\n"); 
        });
        m_window->addChild(m_specialMenu);
    }
    
    void setupTimer() {
        // In a real implementation, you'd set up a timer to update the clock
        // For now, we'll just update it once
        updateClock();
    }
    
    void updateClock() {
        time_t rawtime;
        struct tm * timeinfo;
        char timestr[80];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(timestr, sizeof(timestr), "%H:%M", timeinfo);
        
        m_timeString = std::string(timestr);
    }
    
    AppPtr m_app;
    WindowPtr m_window;
    WindowPtr m_desktop;
    std::shared_ptr<DesktopWidget> m_desktopWidget;
    DropdownPtr m_appleMenu;
    DropdownPtr m_fileMenu;
    DropdownPtr m_editMenu;
    DropdownPtr m_viewMenu;
    DropdownPtr m_specialMenu;
    std::string m_timeString;
};

int main() {
    printf("System 8 Menu Bar\n");
    printf("=================\n");
    
    try {
        System8Bar bar;
        bar.run();
    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());
        return 1;
    }
    
    return 0;
}