#!/usr/bin/env python3

import subprocess
import time

def open_terminal(command, window_name, keystroke, x, y, width, height, sleep_time=1):
    # Open a new terminal window with a unique title and run the command
    subprocess.Popen(['gnome-terminal', '--title', window_name, '--', 'bash', '-c', f'{command}; exec bash'])
    time.sleep(sleep_time)  # Wait for the terminal to open

    # Use xdotool to find the terminal window by its title and send keystrokes
    window_id = subprocess.check_output(['xdotool', 'search', '--name', window_name]).strip()
    subprocess.call(['xdotool', 'windowactivate', window_id])
    subprocess.call(['xdotool', 'type', keystroke])
    subprocess.call(['xdotool', 'key', 'Return'])
    subprocess.call(['xdotool', 'key', 'Return'])

    # Use wmctrl to resize and position the terminal window
    subprocess.call(['wmctrl', '-i', '-r', window_id, '-e', f'0,{x},{y},{width},{height}'])

def main():
    # Get screen dimensions
    screen_width = 1920
    screen_height = 1080
    quad_width = screen_width // 2
    quad_height = screen_height // 2

    # Define terminal commands, window names, keystrokes, and positions
    commands = [
        ("tmotorui 29 vcan0", "Terminal 1", "b", 0, 0, quad_width, quad_height),
        ("tmotorui 20 vcan0", "Terminal 2", "c", quad_width, 0, quad_width, quad_height),
        ("tmotorui 10 vcan0", "Terminal 3", "d", 0, quad_height, quad_width, quad_height),
        ("tmotorui 10 vcan0", "Terminal 4", "e", quad_width, quad_height, quad_width, quad_height)
    ]

    for command, window_name, keystroke, x, y, width, height in commands:
        open_terminal(command, window_name, keystroke, x, y, width, height)

if __name__ == "__main__":
    main()
