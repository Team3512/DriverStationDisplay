//=============================================================================
//File Name: ButtonID.hpp
//Description: Contains ID's of Win32 button controls
//Author: Tyler Veness
//=============================================================================

#ifndef BUTTON_ID_HPP
#define BUTTON_ID_HPP

// Define child-window identifiers for catching their window events
enum {
    IDC_STREAM_BUTTON = 101, // MJPEG stream toggle button
    IDC_CONNECT_BUTTON = 102, // connects to robot so it knows where to send HUD data
    IDC_RELOAD_BUTTON = 103, // reloads the robot code kernel object
    IDC_NEWSETTINGS_BUTTON = 104, // reloads robot's settings file
    IDC_REBOOT_BUTTON = 105, // reboots cRIO controller
    IDC_EXIT_BUTTON = 106 // exits program
};

#endif // BUTTON_ID_HPP
