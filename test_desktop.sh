#!/bin/bash

# Test script for InWM with Desktop and Menu Bar
echo "Testing InWM with Desktop Background and Menu Bar"
echo "================================================"

echo "Starting InWM window manager..."
./inwm &
WM_PID=$!

sleep 2

echo "Starting Desktop and Menu Bar..."
./new_bar &
BAR_PID=$!

sleep 1

echo "Opening test applications..."

# Try to open some applications for testing
if command -v xterm >/dev/null 2>&1; then
    echo "Opening terminal windows..."
    xterm -title "Terminal 1" -geometry 80x24+100+100 &
    sleep 0.5
    xterm -title "Terminal 2" -geometry 80x24+300+150 &
fi

if command -v xcalc >/dev/null 2>&1; then
    echo "Opening calculator..."
    xcalc -geometry +500+200 &
fi

if command -v xeyes >/dev/null 2>&1; then
    echo "Opening xeyes..."
    xeyes -geometry +200+300 &
fi

echo ""
echo "System 8 Desktop Environment Started!"
echo "====================================="
echo ""
echo "Features to try:"
echo "- Click the Apple menu (🍎) for system options"
echo "- Use File, Edit, View, Special menus"
echo "- Notice the desktop pattern in the background"
echo "- Desktop icons: Hard Disk (top-right), Trash (bottom-right)"
echo "- Drag windows by titlebar"
echo "- Resize windows with bottom-right handle"
echo "- Snap windows: Super+Left/Right/Up/Down"
echo "- Click window parts to bring to front"
echo ""
echo "Press Ctrl+C to stop the desktop environment"

# Wait for interrupt
trap "echo 'Stopping InOS Desktop...'; kill $WM_PID $BAR_PID 2>/dev/null; exit 0" INT
wait $WM_PID