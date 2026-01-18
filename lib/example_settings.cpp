#include "Widgets.hpp"
#include <iostream>
#include <cstdio>

using namespace InWM;

// Simple Settings Application Demo
class SettingsApp {
public:
    SettingsApp() {
        // Create application
        m_app = Application::create();
        if (!m_app) {
            throw std::runtime_error("Failed to create application");
        }
        
        createMainWindow();
        createWidgets();
    }
    
    void run() {
        m_window->show();
        printf("Settings application started. Try the different controls.\n");
        m_app->run();
    }

private:
    void createMainWindow() {
        m_window = m_app->createWindow("InOS Settings", 500, 400);
    }
    
    void createWidgets() {
        // Title
        // (In a real implementation, you'd add a Label widget for text)
        
        // Display section
        auto displayDropdown = std::make_shared<Dropdown>(m_app.get(), "Display Resolution");
        displayDropdown->setBounds(Rect(20, 30, 150, 25));
        displayDropdown->addItem("1024x768", [](const Event& e) { 
            printf("Resolution: 1024x768 selected\n"); 
        });
        displayDropdown->addItem("1280x720", [](const Event& e) { 
            printf("Resolution: 1280x720 selected\n"); 
        });
        displayDropdown->addItem("1920x1080", [](const Event& e) { 
            printf("Resolution: 1920x1080 selected\n"); 
        });
        m_window->addChild(displayDropdown);
        
        // Sound section
        auto soundDropdown = std::make_shared<Dropdown>(m_app.get(), "Sound Output");
        soundDropdown->setBounds(Rect(20, 70, 150, 25));
        soundDropdown->addItem("Speakers", [](const Event& e) { 
            printf("Sound: Speakers selected\n"); 
        });
        soundDropdown->addItem("Headphones", [](const Event& e) { 
            printf("Sound: Headphones selected\n"); 
        });
        soundDropdown->addItem("Mute", [](const Event& e) { 
            printf("Sound: Muted\n"); 
        });
        m_window->addChild(soundDropdown);
        
        // Network section
        auto networkDropdown = std::make_shared<Dropdown>(m_app.get(), "Network");
        networkDropdown->setBounds(Rect(20, 110, 150, 25));
        networkDropdown->addItem("Ethernet", [](const Event& e) { 
            printf("Network: Ethernet selected\n"); 
        });
        networkDropdown->addItem("WiFi", [](const Event& e) { 
            printf("Network: WiFi selected\n"); 
        });
        networkDropdown->addItem("Disconnect", [](const Event& e) { 
            printf("Network: Disconnected\n"); 
        });
        m_window->addChild(networkDropdown);
        
        // Control buttons
        auto applyButton = std::make_shared<Button>(m_app.get(), "Apply Settings");
        applyButton->setBounds(Rect(250, 300, 120, 30));
        applyButton->setEventCallback(CLICK, [](const Event& e) {
            printf("Settings applied successfully!\n");
        });
        m_window->addChild(applyButton);
        
        auto resetButton = std::make_shared<Button>(m_app.get(), "Reset to Defaults");
        resetButton->setBounds(Rect(250, 340, 120, 30));
        resetButton->setEventCallback(CLICK, [](const Event& e) {
            printf("Settings reset to defaults\n");
        });
        m_window->addChild(resetButton);
        
        auto closeButton = std::make_shared<Button>(m_app.get(), "Close");
        closeButton->setBounds(Rect(380, 340, 80, 30));
        closeButton->setEventCallback(CLICK, [this](const Event& e) {
            printf("Closing settings application\n");
            m_app->quit();
        });
        m_window->addChild(closeButton);
        
        // Advanced settings dropdown (demonstrates System 8 style menu)
        auto advancedMenu = std::make_shared<Dropdown>(m_app.get(), "Advanced");
        advancedMenu->setBounds(Rect(20, 300, 100, 25));
        advancedMenu->addItem("System Info", [](const Event& e) { 
            printf("System Info requested\n"); 
        });
        advancedMenu->addSeparator();
        advancedMenu->addItem("Memory Settings", [](const Event& e) { 
            printf("Memory Settings opened\n"); 
        });
        advancedMenu->addItem("Startup Items", [](const Event& e) { 
            printf("Startup Items opened\n"); 
        });
        advancedMenu->addSeparator();
        advancedMenu->addItem("Factory Reset", [](const Event& e) { 
            printf("WARNING: Factory Reset selected!\n"); 
        });
        m_window->addChild(advancedMenu);
    }
    
    AppPtr m_app;
    WindowPtr m_window;
};

int main() {
    printf("InOS Settings Application\n");
    printf("========================\n");
    
    try {
        SettingsApp app;
        app.run();
    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());
        return 1;
    }
    
    printf("Settings application finished.\n");
    return 0;
}