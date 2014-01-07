//=============================================================================
//File Name: Resource.h
//Description: Holds all resource IDs for win32 GUI components
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#ifndef RESOURCE_H
#define RESOURCE_H

#define IDD_ABOUTBOX          101

// MJPEG stream toggle button
#define IDC_STREAM_BUTTON     201

// reloads the robot code kernel module
#define IDC_RELOAD_BUTTON     202

// saves currently running robot code kernel module as the fallback version
#define IDC_ALF_SAVE          204

// unloads the current robot code kernel module and loads the fallback version
#define IDC_ALF_FALLBACK      205

// exits program
#define IDC_EXIT_BUTTON       206

// allows selection of autonomous routine
#define IDC_AUTON_COMBOBOX    207

// toggles color blind mode
#define IDC_COLORBLIND_CHK    208

#define IDC_STATIC            -1

#define IDM_ABOUT             301

// hotkeys
#define HK_SAVE               401
#define HK_FALLBACK           402

#endif // RESOURCE_H
