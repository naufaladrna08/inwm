#ifndef INWM_WIDGETS_HPP
#define INWM_WIDGETS_HPP

// Include all InWM GUI components
#include "GUI.hpp"
#include "Application.hpp" 
#include "Window.hpp"
#include "Button.hpp"
#include "Dropdown.hpp"

// Convenience namespace for easier usage
namespace InWM {
    // Type aliases for easier usage
    using AppPtr = std::unique_ptr<Application>;
    using WindowPtr = std::shared_ptr<Window>;
    using WidgetPtr = std::shared_ptr<Widget>;
    using ButtonPtr = std::shared_ptr<Button>;
    using DropdownPtr = std::shared_ptr<Dropdown>;
}

#endif