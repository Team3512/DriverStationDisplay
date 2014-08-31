//=============================================================================
//File Name: Main.cpp
//Description: Receives data from the robot and displays it in a GUI
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <atomic>
#include <thread>
#include <chrono>

#include "MJPEG/MjpegStream.hpp"
#include "OpenGL/StatusLight.hpp"
#include "Settings.hpp"
#include "DisplaySettings.hpp"
#include "Util.hpp"
#include "Resource.h"

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

extern "C" struct GLWindow {
    HWND window;
    HDC dc;
    HGLRC threadRC;
};

GLWindow gMainGLWin;

GLWindow enableOpenGL( HWND window ) {
    // Initialize OpenGL window
    GLWindow winStruct;
    winStruct.window = window;

    // Get dimensions of window
    RECT windowPos;
    GetClientRect( winStruct.window , &windowPos );
    Vector2i size( windowPos.right , windowPos.bottom );

    // Stores pixel format
    int format;

    // Set the pixel format for the DC
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
        1,                     // version number
        PFD_DRAW_TO_WINDOW |   // support winStruct.window
        PFD_SUPPORT_OPENGL |   // support OpenGL
        PFD_DOUBLEBUFFER,      // double buffered
        PFD_TYPE_RGBA,         // RGBA type
        24,                    // 24-bit color depth
        8, 0, 8, 8, 8, 16,     // 8 bits per color component, evenly spaced
        0,                     // no alpha buffer
        0,                     // shift bit ignored
        0,                     // no accumulation buffer
        0, 0, 0, 0,            // accum bits ignored
        16,                    // 16-bit z-buffer
        0,                     // no stencil buffer
        0,                     // no auxiliary buffer
        PFD_MAIN_PLANE,        // main layer
        0,                     // reserved
        0, 0, 0                // layer masks ignored
    };

    // Get the device context (DC)
    winStruct.dc = GetDC( winStruct.window );

    // Get the best available match of pixel format for the device context
    format = ChoosePixelFormat( winStruct.dc , &pfd );
    SetPixelFormat( winStruct.dc , format , &pfd );

    // Create the render context (RC)
    winStruct.threadRC = wglCreateContext( winStruct.dc );

    return winStruct;
}

void disableOpenGL( GLWindow winStruct ) {
    wglMakeCurrent( NULL , NULL );
    wglDeleteContext( winStruct.threadRC );
    ReleaseDC( winStruct.window , winStruct.dc );
}

LRESULT CALLBACK OnEvent( HWND handle , UINT message , WPARAM wParam , LPARAM lParam );

INT WINAPI WinMain( HINSTANCE Instance , HINSTANCE , LPSTR , INT ) {
    INITCOMMONCONTROLSEX icc;

    // Initialize common controls.
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    const char* mainClassName = "DriverStationDisplay";

    HICON mainIcon = LoadIcon( Instance , "mainIcon" );

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
    WindowClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    WindowClass.lpszMenuName  = NULL;
    WindowClass.lpszClassName = mainClassName;
    WindowClass.hIconSm       = mainIcon;
    RegisterClassEx(&WindowClass);

    MSG message;

    int mainWinHeight = GetSystemMetrics(SM_CYSCREEN) - 240;

    // Create a new window to be used for the lifetime of the application
    HWND mainWindow = CreateWindowEx( 0 ,
            mainClassName ,
            "DriverStationDisplay" ,
            WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN ,
            0 ,
            0 ,
            GetSystemMetrics(SM_CXSCREEN) ,
            mainWinHeight ,
            NULL ,
            NULL ,
            Instance ,
            NULL );

    gMainGLWin = enableOpenGL( mainWindow );

    WindowCallbacks streamCallback;
    streamCallback.clickEvent = [&](int x , int y) {};

    /* If this isn't allocated on the heap, it can't be destroyed soon enough.
     * If it were allocated on the stack, it would be destroyed when it leaves
     * WinMain's scope, which is after its parent window is destroyed. This
     * causes the cleanup in this object's destructor to not complete
     * successfully.
     */
    gStreamWinPtr = new MjpegStream( gSettings.getString( "streamHost" ) ,
            gSettings.getInt( "streamPort" ) ,
            gSettings.getString( "streamRequestPath" ) ,
            mainWindow ,
            ( GetSystemMetrics(SM_CXSCREEN) - 320 ) / 2 ,
            60 ,
            320 ,
            240 ,
            Instance ,
            &streamCallback );

    /* ===== Robot Data Sending Variables ===== */
    sf::UdpSocket robotData;
    robotData.bind( gSettings.getInt( "dsDataPort" ) );
    robotData.setBlocking( false );
    gDataSocketPtr = &robotData;

    sf::IpAddress receiveIP;
    unsigned short receivePort;

    sf::Packet dataPacket;
    /* ======================================== */

    sf::TcpSocket robotCmd;
    gCmdSocketPtr = &robotCmd;

    gDrawables = new DisplaySettings( "" , 12 , 12 , gStreamWinPtr->getPosition().X + gStreamWinPtr->getSize().X + 10 , 12 );

    // Used for displaying message box to user while also updating the display
    std::thread* msgBoxThrPtr = NULL;

    // Packet data
    std::string header;
    std::string tempAutonName;
    std::vector<std::string> autonNames;

    // Used for auto-connect with robot
    std::chrono::time_point<std::chrono::system_clock> connectTime;
    bool connectedBefore = false;
    std::atomic<bool> connectDlgOpen( false );

    // Make sure the main window is shown before continuing
    ShowWindow( mainWindow , SW_SHOW );

    // Used for sending connect packets to robot
    sf::IpAddress robotIP( gSettings.getString( "robotIP" ) );
    unsigned short robotDataPort = gSettings.getInt( "robotDataPort" );

    // Reloads robot code kernel module
    RegisterHotKey( mainWindow , HK_SAVE , MOD_CONTROL , 0x53 ); // Ctrl + S
    RegisterHotKey( mainWindow , HK_FALLBACK , MOD_CONTROL , 0x46 ); // Ctrl + F

    bool isExiting = false;
    while ( !isExiting ) {
        if ( PeekMessage( &message , NULL , 0 , 0 , PM_REMOVE ) ) {
            if ( message.message != WM_QUIT ) {
                // If a message was waiting in the message queue, process it
                TranslateMessage( &message );
                DispatchMessage( &message );
            }
            else {
                isExiting = true;
            }
        }
        else {
            // If timeout has passed, remove GUI and attempt reconnect
            if ( std::chrono::system_clock::now() - connectTime  > std::chrono::milliseconds(2000) && !connectDlgOpen ) {
                char data[16] = "connect\r\n";

                if ( gDataSocketPtr != NULL ) {
                    gDataSocketPtr->send( data , 16 , robotIP , robotDataPort );
                }

                connectTime = std::chrono::system_clock::now();
            }

            // Retrieve data sent from robot and unpack it
            if ( robotData.receive( dataPacket , receiveIP , receivePort ) == sf::Socket::Done ) {
                // Unpacks header telling packet type (either "display" or "autonList")
                dataPacket >> header;

                if ( std::strcmp( header.c_str() , "display\r\n" ) == 0 ) {
                    gDrawables->updateGuiTable( dataPacket );
                    NetUpdate::updateElements();

                    /* Only allow keep-alive (resetting timer) if we have a
                     * valid GUI; we need to connect and create the GUI before
                     * accepting display data
                     */
                    if ( connectedBefore ) {
                        connectTime = std::chrono::system_clock::now();
                    }
                }

                else if ( std::strcmp( header.c_str() , "guiCreate\r\n" ) == 0 ) {
                    gDrawables->reloadGUI( dataPacket );

                    if ( !connectedBefore ) {
                        connectedBefore = true;
                    }

                    connectTime = std::chrono::system_clock::now();
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

                        /* Select the first Autonomous automatically; it will
                         * be updated to the correct value when the
                         * 'autonConfirmed' packet arrives
                         */
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

                    LPARAM namePtr = reinterpret_cast<LPARAM>( tempName.c_str() );

                    // Change local selection to match sent one
                    SendMessage( gAutonComboBox , CB_SELECTSTRING , -1 , namePtr );

                    /* Don't let the message box thread attempt to spawn again
                     * before the previous one has exited; it will hang on
                     * joining the previous thread and crash due to window
                     * messages no longer being processed.
                     */
                    if ( !connectDlgOpen ) {
                        // Delete old thread before spawning new one
                        delete msgBoxThrPtr;

                        msgBoxThrPtr = new std::thread( [&]{ connectDlgOpen = true;
                                MessageBox( mainWindow , autoName.c_str() , "Autonomous Change" , MB_ICONINFORMATION | MB_OK );
                                connectDlgOpen = false;
                        } );
                        msgBoxThrPtr->detach();
                    }
                }

                // Make the window redraw the controls
                InvalidateRect( mainWindow , NULL , FALSE );
            }

            // Make the window redraw the controls
            InvalidateRect( mainWindow , NULL , FALSE );

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // Neither of these were allocated on the stack before storing their pointers
    delete gDrawables;

    // Delete message box thread
    delete msgBoxThrPtr;

    // Delete MJPEG stream window
    delete gStreamWinPtr;

    // Clean up OpenGL before destroying main window
    disableOpenGL( gMainGLWin );

    // Clean up windows
    DestroyWindow( mainWindow );
    UnregisterClass( mainClassName , Instance );

    return message.wParam;
}

LRESULT CALLBACK OnEvent( HWND handle , UINT message , WPARAM wParam , LPARAM lParam ) {
    switch ( message ) {
    case WM_CREATE: {
        HGDIOBJ hfDefault = GetStockObject( DEFAULT_GUI_FONT );

        // Create "reload code" button
        HWND reloadButton = CreateWindowEx( 0,
            "BUTTON",
            "Reload Code",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            0 * ( 5 + 28 ) + 5,
            100,
            28,
            handle,
            reinterpret_cast<HMENU>( IDC_RELOAD_BUTTON ),
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);

        SendMessage(reloadButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        // Create "save code" button
        HWND saveButton = CreateWindowEx( 0,
            "BUTTON",
            "Save Code",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            1 * ( 5 + 28 ) + 5,
            100,
            28,
            handle,
            reinterpret_cast<HMENU>( IDC_ALF_SAVE ),
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);

        SendMessage(saveButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        // Create "fallback code" button
        HWND fallbackButton = CreateWindowEx( 0,
            "BUTTON",
            "Fallback Code",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            2 * ( 5 + 28 ) + 5,
            100,
            28,
            handle,
            reinterpret_cast<HMENU>( IDC_ALF_FALLBACK ),
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);

        SendMessage(fallbackButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        // Create "exit" button
        HWND exitButton = CreateWindowEx( 0,
            "BUTTON",
            "Exit",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            3 * ( 5 + 28 ) + 5,
            100,
            28,
            handle,
            reinterpret_cast<HMENU>( IDC_EXIT_BUTTON ),
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);

        SendMessage(exitButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        // Create autonomous mode selection box
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
            ((LPCREATESTRUCT)lParam)->hInstance,
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

        // Create color blind mode check box
        HWND colorBlindChk = CreateWindowEx( 0,
            "BUTTON",
            "Color Blind Mode",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            5 * ( 5 + 28 ) + 5,
            100,
            13,
            handle,
            reinterpret_cast<HMENU>( IDC_COLORBLIND_CHK ),
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);

        SendMessage(colorBlindChk,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        CheckDlgButton( handle , IDC_COLORBLIND_CHK , BST_UNCHECKED );

        break;
    }

    case WM_COMMAND: {
        char data[16];

        sf::IpAddress robotIP( gSettings.getString( "robotIP" ) );
        unsigned short robotDataPort = gSettings.getInt( "robotDataPort" );
        unsigned short alfCmdPort = gSettings.getInt( "alfCmdPort" );

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

            case IDC_ALF_SAVE: {
                std::strcpy( data , "save\r\n" );

                if ( gCmdSocketPtr != NULL ) {
                    gCmdSocketPtr->connect( robotIP , alfCmdPort , sf::milliseconds( 500 ) );
                    gCmdSocketPtr->send( data , 16 );
                    gCmdSocketPtr->disconnect();
                }

                break;
            }

            case IDC_ALF_FALLBACK: {
                std::strcpy( data , "fallback\r\n" );

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

            case IDC_COLORBLIND_CHK: {
                UINT checked = IsDlgButtonChecked( handle ,
                        IDC_COLORBLIND_CHK );

                if ( checked == BST_CHECKED ) {
                    CheckDlgButton( handle , IDC_COLORBLIND_CHK ,
                            BST_UNCHECKED );

                    StatusLight::setColorBlind( false );
                }
                else if ( checked == BST_UNCHECKED ) {
                    CheckDlgButton( handle , IDC_COLORBLIND_CHK ,
                            BST_CHECKED );

                    StatusLight::setColorBlind( true );
                }

                break;
            }
        }

        break;
    }

    case WM_PAINT: {
        GLenum glError;

        // Get dimensions of window
        RECT windowPos;
        GetClientRect( handle , &windowPos );
        Vector2i size( windowPos.right , windowPos.bottom );

        wglMakeCurrent( gMainGLWin.dc , gMainGLWin.threadRC );

        // Check for OpenGL errors
        glError = glGetError();
        if( glError != GL_NO_ERROR ) {
            std::cerr << "wglMakeCurrent: " << gluErrorString( glError ) << "\n";
        }

        PAINTSTRUCT ps;
        BeginPaint( handle , &ps );

        static int bufWidth = size.X;
        static int bufHeight = size.Y;
        static BYTE* pxlBuf = new BYTE[size.X * size.Y * 4];

        if ( bufWidth != size.X || bufHeight != size.Y ) {
            delete[] pxlBuf;
            pxlBuf = new BYTE[size.X * size.Y * 4];

            bufWidth = size.X;
            bufHeight = size.Y;
        }

        /* ===== Draw WinGDI objects ===== */
        // Create new device context
        HDC gdiDC = CreateCompatibleDC( gMainGLWin.dc );

        // Create the bitmap used for graphics
        HBITMAP gdiBmp = CreateCompatibleBitmap( gMainGLWin.dc , size.X , size.Y );

        // Give each graphic a bitmap to use
        HBITMAP oldgdiBmp = static_cast<HBITMAP>(SelectObject( gdiDC , gdiBmp ));

        // Create region for background
        HRGN backgroundRegion = CreateRectRgn( 0 , 0 , size.X , size.Y );

        // Fill graphic with background color
        FillRgn( gdiDC , backgroundRegion , GetSysColorBrush( COLOR_3DFACE ) );

        // Free the region
        DeleteObject( backgroundRegion );

        // Draw WinGDI objects
        gDrawables->drawDisplay( gdiDC );

        // Store bits from graphics in another buffer
        BMPtoPXL( gdiDC , gdiBmp , size.X , size.Y , pxlBuf );

        // Put old bitmaps back in DCs before deleting them
        SelectObject( gdiDC , oldgdiBmp );

        // Free WinGDI objects
        DeleteDC( gdiDC );
        DeleteObject( gdiBmp );
        /* =============================== */

        /* ===== Initialize OpenGL ===== */
        glClearDepth( 1.f );

        glDepthFunc( GL_LESS );
        glDepthMask( GL_FALSE );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_BLEND );
        glDisable( GL_ALPHA_TEST );
        glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
        glShadeModel( GL_FLAT );

        // Set up screen
        glViewport( 0 , 0 , size.X , size.Y );
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0 , size.X , size.Y , 0 , -1.f , 1.f );
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        // Check for OpenGL errors
        glError = glGetError();
        if( glError != GL_NO_ERROR ) {
            std::cerr << "Main.cpp OpenGL: " << gluErrorString( glError ) << "\n";
        }
        /* ============================= */

        /* ===== Convert BGRA image to RGBA ===== */
        /*BYTE temp;
        for ( int pos = 0 ; pos < size.X * size.Y ; pos++ ) {
            temp = pxlBuf[4*pos+0];
            // Swap R and B channels
            pxlBuf[4*pos+0] = pxlBuf[4*pos+2];
            pxlBuf[4*pos+2] = temp;
        }*/
        /* ====================================== */

        static int textureSize = 0;

        glEnable( GL_TEXTURE_2D );
        glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );

        /* If our image won't fit in the texture, make a bigger one whose width and
         * height are a power of two.
         */
        if( size.X > textureSize || size.Y > textureSize ) {
            textureSize = npot( std::max( size.X , size.Y ) );

            uint8_t* tmpBuf = new uint8_t[textureSize * textureSize * 3];
            glTexImage2D( GL_TEXTURE_2D , 0 , 3 , textureSize , textureSize , 0 ,
                    GL_RGB , GL_UNSIGNED_BYTE , tmpBuf );
            delete[] tmpBuf;
        }

        /* Represents the amount of the texture to display. These are ratios
         * between the dimensions of the image in the texture and the actual
         * texture dimensions. Once these are set for the specific image to be
         * displayed, they are passed into glTexCoord2f.
         */
        double wratio = 0.f;
        double hratio = 0.f;

        glTexSubImage2D( GL_TEXTURE_2D , 0 , 0 , 0 , size.X , size.Y ,
                GL_RGBA , GL_UNSIGNED_BYTE , pxlBuf );

        wratio = (float)size.X / (float)textureSize;
        hratio = (float)size.Y / (float)textureSize;

        // Position the GL texture in the Win32 window
        glBegin( GL_TRIANGLE_FAN );
        glColor4f( 1.f , 1.f , 1.f , 1.f );
        glTexCoord2f( 0 , 0 ); glVertex3f( 0 , 0 , 0 );
        glTexCoord2f( wratio , 0 ); glVertex3f( size.X , 0 , 0 );
        glTexCoord2f( wratio , hratio ); glVertex3f( size.X , size.Y , 0 );
        glTexCoord2f( 0 , hratio ); glVertex3f( 0 , size.Y , 0 );
        glEnd();

        /*glBegin( GL_TRIANGLES );
        glColor4f( backColor.glR() , backColor.glG() , backColor.glB() , backColor.glA() );
        glVertex3f( 0 , 0 , 0 );
        glVertex3f( size.X , 0 , 0 );
        glVertex3f( size.X , size.Y , 0 );

        glVertex3f( 0 , 0 , 0 );
        glVertex3f( size.X , size.Y , 0 );
        glVertex3f( 0 , size.Y , 0 );
        glEnd();*/

        // Check for OpenGL errors
        glError = glGetError();
        if( glError != GL_NO_ERROR ) {
            std::cerr << "Main.cpp OpenGL failure: " << gluErrorString( glError ) << "\n";
        }

        glDisable( GL_TEXTURE_2D );

        gDrawables->drawDisplay( NULL );

        // Check for OpenGL errors
        glError = glGetError();
        if( glError != GL_NO_ERROR ) {
            std::cerr << "Drawables OpenGL failure: " << gluErrorString( glError ) << "\n";
        }

        // Display OpenGL drawing
        SwapBuffers( gMainGLWin.dc );

        EndPaint( handle , &ps );

        break;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);

        break;
    }

    case WM_HOTKEY: {
        switch ( wParam ) {
        case HK_SAVE: {
            SendMessage( handle , WM_COMMAND , IDC_ALF_SAVE , 0 );

            break;
        }
        case HK_FALLBACK: {
            SendMessage( handle , WM_COMMAND , IDC_ALF_FALLBACK , 0 );

            break;
        }
        }

        break;
    }

    case WM_MJPEGSTREAM_START: {
        gStreamWinPtr->repaint();

        break;
    }

    case WM_MJPEGSTREAM_STOP: {
        gStreamWinPtr->repaint();

        break;
    }

    case WM_MJPEGSTREAM_NEWIMAGE: {
        gStreamWinPtr->repaint();

        break;
    }

    default: {
        return DefWindowProc(handle, message, wParam, lParam);
    }
    }

    return 0;
}
