//=============================================================================
//File Name: Main.cpp
//Description: Receives data from the robot and displays it in a GUI
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

/* TODO Get position of "DriverStation" window and adjust main window height
 * based upon that. Use a default height if not found.
 * TODO Put socket receiving in separate threads with signals and PostThreadMessage
 * (capture SIGQOUIT or SIGKILL?)
 */

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

#include "WinGDI/ProgressBar.hpp"
#include "WinGDI/StatusLight.hpp"
#include "WinGDI/Text.hpp"
#include "WinGDI/UIFont.hpp"
#include "MJPEG/MjpegStream.hpp"
#include "Settings.hpp"
#include "DisplaySettings.hpp"
#include "Resource.h"

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wingdi.h>

#define _WIN32_IE 0x0400
#include <commctrl.h>

// Global because the window is closed by a button in CALLBACK OnEvent
HWND gAutonComboBox = NULL;

// Global because IP configuration settings are needed in CALLBACK OnEvent
Settings gSettings( "IPSettings.txt" );

// Stores all Drawables to be drawn with WM_PAINT message
DisplaySettings* gDrawables = NULL;

// Allows manipulation of MjpegStream in CALLBACK OnEvent
MjpegStream* gStreamWinPtr = NULL;

// Allows usage of socket in CALLBACK OnEvent
sf::UdpSocket* gDataSocketPtr = NULL;
sf::TcpSocket* gCmdSocketPtr = NULL;

LRESULT CALLBACK OnEvent( HWND handle , UINT message , WPARAM wParam , LPARAM lParam );

INT WINAPI WinMain( HINSTANCE Instance , HINSTANCE , LPSTR , INT ) {
    INITCOMMONCONTROLSEX icc;

    // Initialize common controls.
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    const char* mainClassName = "DriverStationDisplay";

    HICON mainIcon = LoadIcon( Instance , "mainIcon" );
    HBRUSH mainBrush = CreateSolidBrush( RGB( 87 , 87 , 87 ) );

    // Define a class for our main window
    WNDCLASSEX WindowClass;
    ZeroMemory( &WindowClass , sizeof(WNDCLASSEX) );
    WindowClass.cbSize        = sizeof(WNDCLASSEX);
    WindowClass.style         = 0;
    WindowClass.lpfnWndProc   = &OnEvent;
    WindowClass.cbClsExtra    = 0;
    WindowClass.cbWndExtra    = 0;
    WindowClass.hInstance     = Instance;
    WindowClass.hIcon         = mainIcon;
    WindowClass.hCursor       = LoadCursor( NULL , IDC_ARROW );
    WindowClass.hbrBackground = mainBrush;
    WindowClass.lpszMenuName  = NULL;
    WindowClass.lpszClassName = mainClassName;
    WindowClass.hIconSm       = mainIcon;
    RegisterClassEx(&WindowClass);

    MSG message;

    int mainWinHeight = GetSystemMetrics(SM_CYSCREEN) - 240;

    // Create a new window to be used for the lifetime of the application
    HWND mainWindow = CreateWindowEx( 0 ,
            mainClassName ,
            "" ,
            WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN ,
            0 ,
            0 ,
            GetSystemMetrics(SM_CXSCREEN) ,
            mainWinHeight ,
            NULL ,
            NULL ,
            Instance ,
            NULL );

    MjpegStream streamWin( gSettings.getValueFor( "streamHost" ) ,
            std::atoi( gSettings.getValueFor( "streamPort" ).c_str() ) ,
            gSettings.getValueFor( "streamRequestPath" ) ,
            mainWindow ,
            ( GetSystemMetrics(SM_CXSCREEN) - 320 ) / 2 ,
            60 ,
            320 ,
            240 ,
            Instance );
    gStreamWinPtr = &streamWin;

    /* ===== Robot Data Sending Variables ===== */
    sf::UdpSocket robotData;
    robotData.bind( std::atoi( gSettings.getValueFor( "dsDataPort" ).c_str() ) );
    robotData.setBlocking( false );
    gDataSocketPtr = &robotData;

    sf::IpAddress receiveIP;
    unsigned short receivePort;

    sf::Packet dataPacket;
    /* ======================================== */

    sf::TcpSocket robotCmd;
    gCmdSocketPtr = &robotCmd;

    gDrawables = new DisplaySettings( "" , 12 , 12 , streamWin.getPosition().X + streamWin.getSize().X + 10 , 12 );

    // Packet data
    std::string header;
    std::string tempAutonName;
    std::vector<std::string> autonNames;

    // Make sure the main window is shown before continuing
    ShowWindow( mainWindow , SW_SHOW );

    bool isExiting = false;
    while ( !isExiting ) {
        if ( PeekMessage( &message , NULL , 0 , 0 , PM_NOREMOVE ) ) {
            if ( GetMessage( &message , NULL , 0 , 0 ) > 0 ) {
                // If a message was waiting in the message queue, process it
                TranslateMessage( &message );
                DispatchMessage( &message );
            }
            else {
                isExiting = true;
            }
        }
        else {
            streamWin.display();

            // Retrieve data sent from robot and unpack it
            if ( robotData.receive( dataPacket , receiveIP , receivePort ) == sf::Socket::Done ) {
                // Unpacks header telling packet type (either "display" or "autonList")
                dataPacket >> header;

                if ( std::strcmp( header.c_str() , "display\r\n" ) == 0 ) {
                    gDrawables->updateGuiTable( dataPacket );
                    NetUpdate::updateElements();
                }

                else if ( std::strcmp( header.c_str() , "guiCreate\r\n" ) == 0 ) {
                    gDrawables->reloadGUI( dataPacket );
                }

                else if ( std::strcmp( header.c_str() , "autonList\r\n" ) == 0 ) {
                    /* Unpacks the following variables:
                     *
                     * Autonomous Modes (contained in rest of packet):
                     * std::string: autonomous routine name
                     * <more autonomous routine names>...
                     */

                    autonNames.clear();
                    while ( !dataPacket.endOfPacket() ) {
                        dataPacket >> tempAutonName;
                        autonNames.push_back( tempAutonName );
                    }

                    /* Shouldn't be NULL since its parent window was
                     * initialized long before here, but just to be safe...
                     */
                    if ( gAutonComboBox != NULL ) {
                        /* Remove all items from combo box before adding the
                         * new ones
                         */
                        while ( SendMessage( gAutonComboBox , CB_DELETESTRING , 0 , 0 ) > 0 );

                        for ( unsigned int i = 0 ; i < autonNames.size() ; i++ ) {
                            SendMessage( gAutonComboBox ,
                                    CB_ADDSTRING ,
                                    0 ,
                                    reinterpret_cast<LPARAM>( autonNames[i].c_str() )
                            );
                        }

                        // Select the first Autonomous automatically
                        SendMessage( gAutonComboBox , CB_SETCURSEL , 0 , 0 );
                    }
                }
                /* If a new autonomous mode was selected from the robot, it
                 * sends back this packet as confirmation
                 */
                else if ( std::strcmp( header.c_str() , "autonConfirmed\r\n" ) == 0 ) {
                    std::string autoName = "Autonomous mode changed to\n";
                    std::string tempName;
                    dataPacket >> tempName;
                    autoName += tempName;

                    MessageBox( mainWindow , autoName.c_str() , "Autonomous Change" , MB_ICONINFORMATION | MB_OK );
                }

                // Make the window redraw the controls
                InvalidateRect( mainWindow , NULL , FALSE );
            }

            // Make the window redraw the controls
            InvalidateRect( mainWindow , NULL , FALSE );

            Sleep( 30 );
        }
    }

    UIFont::freeInstance();

    // Clean up windows
    DestroyWindow( mainWindow );
    UnregisterClass( mainClassName , Instance );

    return EXIT_SUCCESS;
}

LRESULT CALLBACK OnEvent( HWND handle , UINT message , WPARAM wParam , LPARAM lParam ) {
    switch ( message ) {
    case WM_CREATE: {
        HGDIOBJ hfDefault = GetStockObject( DEFAULT_GUI_FONT );

        HWND connectButton = CreateWindowEx( 0,
            "BUTTON",
            "Connect DS",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            0 * ( 5 + 28 ) + 5,
            100,
            28,
            handle,
            reinterpret_cast<HMENU>( IDC_CONNECT_BUTTON ),
            GetModuleHandle( NULL ),
            NULL);

        SendMessage(connectButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );


        HWND reloadButton = CreateWindowEx( 0,
            "BUTTON",
            "Reload Code",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            1 * ( 5 + 28 ) + 5,
            100,
            28,
            handle,
            reinterpret_cast<HMENU>( IDC_RELOAD_BUTTON ),
            GetModuleHandle( NULL ),
            NULL);

        SendMessage(reloadButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );


        HWND rebootButton = CreateWindowEx( 0,
            "BUTTON",
            "Reboot Robot",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            2 * ( 5 + 28 ) + 5,
            100,
            28,
            handle,
            reinterpret_cast<HMENU>( IDC_REBOOT_BUTTON ),
            GetModuleHandle( NULL ),
            NULL);

        SendMessage(rebootButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );


        HWND exitButton = CreateWindowEx( 0,
            "BUTTON",
            "Exit",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            3 * ( 5 + 28 ) + 5,
            100,
            28,
            handle,
            reinterpret_cast<HMENU>( IDC_EXIT_BUTTON ),
            GetModuleHandle( NULL ),
            NULL);

        SendMessage(exitButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        HWND autonComboBox = CreateWindowEx( 0,
            "COMBOBOX",
            "Autonomous Mode",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            4 * ( 5 + 28 ) + 5,
            100,
            GetSystemMetrics(SM_CYSCREEN) - 240 - ( 4 * ( 5 + 28 ) + 5 ),
            handle,
            reinterpret_cast<HMENU>( IDC_AUTON_COMBOBOX ),
            GetModuleHandle( NULL ),
            NULL );
        gAutonComboBox = autonComboBox;

        SendMessage( autonComboBox,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        SendMessage( autonComboBox,
            CB_ADDSTRING,
            0,
            reinterpret_cast<LPARAM>( "" ));

        SendMessage( autonComboBox , CB_SETCURSEL , 0 , 0 );

        break;
    }

    case WM_COMMAND: {
        char* data = static_cast<char*>( std::malloc( 16 ) );

        sf::IpAddress robotIP( gSettings.getValueFor( "robotIP" ) );
        unsigned short robotDataPort = std::atoi( gSettings.getValueFor( "robotDataPort" ).c_str() );
        unsigned short alfCmdPort = std::atoi( gSettings.getValueFor( "alfCmdPort" ).c_str() );

        switch( LOWORD(wParam) ) {
            case IDC_STREAM_BUTTON: {
                 if ( gStreamWinPtr != NULL ) {
                     if ( gStreamWinPtr->isStreaming() ) {
                         // Stop streaming
                         gStreamWinPtr->stopStream();
                     }
                     else {
                         // Start streaming
                         gStreamWinPtr->startStream();
                     }
                 }

                 break;
            }

            case IDC_CONNECT_BUTTON: {
                std::strcpy( data , "connect\r\n" );

                if ( gDataSocketPtr != NULL ) {
                    gDataSocketPtr->send( data , 16 , robotIP , robotDataPort );
                }

                break;
            }

            // These other commands get sent to ALF rather than the robot
            case IDC_RELOAD_BUTTON: {
                std::strcpy( data , "reload\r\n" );

                if ( gCmdSocketPtr != NULL ) {
                    gCmdSocketPtr->connect( robotIP , alfCmdPort , sf::milliseconds( 500 ) );
                    gCmdSocketPtr->send( data , 16 );
                    gCmdSocketPtr->disconnect();
                }

                break;
            }

            case IDC_REBOOT_BUTTON: {
                std::strcpy( data , "reboot\r\n" );

                if ( gCmdSocketPtr != NULL ) {
                    gCmdSocketPtr->connect( robotIP , alfCmdPort , sf::milliseconds( 500 ) );
                    gCmdSocketPtr->send( data , 16 );
                    gCmdSocketPtr->disconnect();
                }

                break;
            }

            case IDC_EXIT_BUTTON: {
                PostQuitMessage(0);

                break;
            }

            case IDC_AUTON_COMBOBOX: {
                switch ( HIWORD(wParam) ) {
                case CBN_SELCHANGE: {
                    // Get new Autonomous selection since it changed
                    int selection = SendMessage( (HWND)lParam , CB_GETCURSEL , 0 , 0 );

                    // If it's really a selection, tell the robot to change Autonomous
                    if ( selection != CB_ERR ) {
                        std::strcpy( data , "autonSelect\r\n" );
                        data[13] = selection;

                        if ( gDataSocketPtr != NULL ) {
                            gDataSocketPtr->send( data , 16 , robotIP , robotDataPort );
                        }
                    }
                }
                }

                break;
            }
        }

        std::free( data );

        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint( handle , &ps );

        /* ===== Create buffer DC ===== */
        RECT rect;
        GetClientRect( handle , &rect );

        HDC bufferDC = CreateCompatibleDC( hdc );
        HBITMAP bufferBmp = CreateCompatibleBitmap( hdc , rect.right , rect.bottom );

        HBITMAP oldBmp = static_cast<HBITMAP>( SelectObject( bufferDC , bufferBmp ) );
        /* ============================ */

        // Fill buffer DC with a background color
        HBRUSH backgroundBrush = CreateSolidBrush( RGB( 87 , 87 , 87 ) );
        HRGN region = CreateRectRgn( 0 , 0 , rect.right , rect.bottom );
        FillRgn( bufferDC , region , backgroundBrush );
        DeleteObject( region );
        DeleteObject( backgroundBrush );

        // Creates 1:1 relationship between logical units and pixels
        int oldMapMode = SetMapMode( bufferDC , MM_TEXT );

        gDrawables->drawDisplay( bufferDC );

        BitBlt( hdc , 0 , 0 , rect.right , rect.bottom , bufferDC , 0 , 0 , SRCCOPY );

        // Restore old DC mapping mode
        SetMapMode( bufferDC , oldMapMode );

        // Replace the old bitmap and delete the created one
        DeleteObject( SelectObject( bufferDC , oldBmp ) );

        // Free the buffer DC
        DeleteDC( bufferDC );

        EndPaint( handle , &ps );

        break;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);

        break;
    }

    case WM_MJPEGSTREAM_START: {
        gStreamWinPtr->display();

        break;
    }

    case WM_MJPEGSTREAM_STOP: {
        gStreamWinPtr->display();

        break;
    }

    case WM_MJPEGSTREAM_NEWIMAGE: {
        gStreamWinPtr->display();

        break;
    }

    default: {
        return DefWindowProc(handle, message, wParam, lParam);
    }
    }

    return 0;
}
