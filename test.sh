#!/bin/bash

# Test script for InWM
# This script helps test the window manager by opening some test applications

echo "Testing InWM Window Manager"
echo "=========================="

echo "Starting InWM window manager in background..."
./inwm &
WM_PID=$!

sleep 2

echo "Starting InWM bar..."
./bar/bar &
BAR_PID=$!

sleep 1

echo "Opening test applications..."

# Try to open some common applications for testing
if command -v xterm >/dev/null 2>&1; then
    echo "Opening xterm..."
    xterm -title "Test Terminal 1" &
    sleep 1
    xterm -title "Test Terminal 2" &
fi

if command -v xclock >/dev/null 2>&1; then
    echo "Opening xclock..."
    xclock &
fi

if command -v xeyes >/dev/null 2>&1; then
    echo "Opening xeyes..."
    xeyes &
fi

echo ""
echo "Test applications launched!"
echo "Try the following:"
echo "- Drag windows by their titlebar"
echo "- Resize windows using the bottom-right handle"
echo "- Snap windows to edges by dragging or using:"
echo "  - Super+Left: Snap to left half"
echo "  - Super+Right: Snap to right half" 
echo "  - Super+Up: Maximize"
echo "  - Super+Down: Restore"
echo "- Click the close button to close windows"
echo "- Click on the menu bar items"
echo ""
echo "Press Ctrl+C to stop the window manager"

# Wait for interrupt
trap "echo 'Stopping InWM...'; kill $WM_PID $BAR_PID 2>/dev/null; exit 0" INT
wait $WM_PID