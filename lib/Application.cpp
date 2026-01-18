#include "Application.hpp"
#include "Window.hpp"
#include <cstdio>

namespace InWM {

std::unique_ptr<Application> Application::create() {
  Display* display = XOpenDisplay(nullptr);
  if (!display) {
    printf("Failed to open X display\n");
    return nullptr;
  }
  
  return std::unique_ptr<Application>(new Application(display));
}

Application::Application(Display* display) 
: m_display(display), m_root(DefaultRootWindow(display)) {
  int screen = DefaultScreen(m_display);
  m_gc = DefaultGC(m_display, screen);
}

Application::~Application() {
  if (m_display) {
    XCloseDisplay(m_display);
  }
}

void Application::run() {
  XEvent event;
  
  while (m_running) {
    XNextEvent(m_display, &event);
    handleX11Event(event);
  }
}

std::shared_ptr<Window> Application::createWindow(const std::string& title, int width, int height) {
  auto window = std::make_shared<Window>(this, title, width, height);
  m_windows.push_back(window);
  return window;
}

void Application::closeWindow(Window* window) {
  m_windows.erase(
    std::remove_if(m_windows.begin(), m_windows.end(),
                  [window](const auto& ptr) { return ptr.get() == window; }),
    m_windows.end()
  );
}

void Application::handleX11Event(const XEvent& xevent) {
  // Find the window that should handle this event
  for (auto& window : m_windows) {
    if (window->handleX11Event(xevent)) {
      break; // Event was handled
    }
  }
}

} // namespace InWM