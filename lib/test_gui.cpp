#include "Widgets.hpp"
#include <iostream>
#include <cstdio>

using namespace InWM;

int main() {
    printf("InWM GUI Library Test\n");
    printf("====================\n");
    
    // Create application
    auto app = Application::create();
    if (!app) {
        printf("Failed to create application\n");
        return 1;
    }
    
    // Create main window
    auto window = app->createWindow("InWM GUI Test", 400, 300);
    
    // Create a button
    auto button1 = std::make_shared<Button>(app.get(), "Click Me!");
    button1->setBounds(Rect(50, 50, 100, 30));
    button1->setEventCallback(CLICK, [](const Event& e) {
        printf("Button 1 clicked!\n");
    });
    window->addChild(button1);
    
    // Create another button
    auto button2 = std::make_shared<Button>(app.get(), "Exit");
    button2->setBounds(Rect(200, 50, 80, 30));
    button2->setEventCallback(CLICK, [&app](const Event& e) {
        printf("Exit button clicked - closing application\n");
        app->quit();
    });
    window->addChild(button2);
    
    // Create a dropdown menu
    auto dropdown = std::make_shared<Dropdown>(app.get(), "File");
    dropdown->setBounds(Rect(50, 100, 80, 25));
    dropdown->addItem("New", [](const Event& e) { printf("New file selected\n"); });
    dropdown->addItem("Open", [](const Event& e) { printf("Open file selected\n"); });
    dropdown->addSeparator();
    dropdown->addItem("Save", [](const Event& e) { printf("Save file selected\n"); });
    dropdown->addItem("Save As...", [](const Event& e) { printf("Save As selected\n"); });
    dropdown->addSeparator();
    dropdown->addItem("Exit", [&app](const Event& e) { 
        printf("Exit from menu selected\n");
        app->quit(); 
    });
    window->addChild(dropdown);
    
    // Create another dropdown
    auto editMenu = std::make_shared<Dropdown>(app.get(), "Edit");
    editMenu->setBounds(Rect(140, 100, 80, 25));
    editMenu->addItem("Cut", [](const Event& e) { printf("Cut selected\n"); });
    editMenu->addItem("Copy", [](const Event& e) { printf("Copy selected\n"); });
    editMenu->addItem("Paste", [](const Event& e) { printf("Paste selected\n"); });
    window->addChild(editMenu);
    
    // Show window and run application
    window->show();
    
    printf("GUI window created. Try clicking buttons and dropdown menus.\n");
    printf("Close the window or click 'Exit' to quit.\n");
    
    app->run();
    
    printf("Application finished.\n");
    return 0;
}