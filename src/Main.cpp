//=============================================================================
//File Name: Main.cpp
//Description: Receives data from the robot and displays it in a GUI
//Author: Tyler Veness
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
#include <vector>
#include <string>
#include <cstring>

#include "WinGDI/ProgressBar.hpp"
#include "WinGDI/StatusLight.hpp"
#include "WinGDI/Text.hpp"
#include "WinGDI/UIFont.hpp"
#include "MJPEG/MjpegStream.hpp"
#include "Settings.hpp"
#include "Resource.h"

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wingdi.h>

#define _WIN32_IE 0x0400
#include <commctrl.h>

// Global because the window is closed by a button in CALLBACK OnEvent
HWND gAutonComboBox = NULL;

// Global because the window is closed by a button in CALLBACK OnEvent
HWND gMainWinPtr = NULL;

// Global because IP configuration settings are needed in CALLBACK OnEvent
Settings gSettings( "IPSettings.txt" );

// Stores all Drawables to be drawn with WM_PAINT message
std::vector<Drawable*> gDrawables;

// Allows manipulation of MjpegStream in CALLBACK OnEvent
MjpegStream* gStreamWinPtr = NULL;

// Allows usage of socket in CALLBACK OnEvent
sf::UdpSocket* gDataSocketPtr = NULL;
sf::TcpSocket* gCmdSocketPtr = NULL;

// Used to tell message loop when to exit
volatile bool gExit = false;

template <class T>
std::wstring numberToString( T number ) {
    return static_cast<std::wostringstream*>( &(std::wostringstream() << number) )->str();
}

LRESULT CALLBACK OnEvent( HWND handle , UINT message , WPARAM wParam , LPARAM lParam );

INT WINAPI WinMain( HINSTANCE Instance , HINSTANCE , LPSTR , INT ) {
    INITCOMMONCONTROLSEX icc;

    // Initialise common controls.
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    const char* mainClassName = "DriverStationDisplay";

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
    WindowClass.hIcon         = NULL;
    WindowClass.hCursor       = LoadCursor( NULL , IDC_ARROW );
    WindowClass.hbrBackground = mainBrush;
    WindowClass.lpszMenuName  = NULL;
    WindowClass.lpszClassName = mainClassName;
    WindowClass.hIconSm       = NULL;
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
    gMainWinPtr = mainWindow;

    MjpegStream streamWin( gSettings.getValueFor( "streamHost" ) ,
            std::atoi( gSettings.getValueFor( "streamPort" ).c_str() ) ,
            mainWindow ,
            ( GetSystemMetrics(SM_CXSCREEN) - 320 ) / 2 ,
            60 ,
            320 ,
            240 ,
            Instance );
    gStreamWinPtr = &streamWin;

    /* ===== Robot Data Sending Variables ===== */
    sf::UdpSocket robotData;
    robotData.bind( std::atoi( gSettings.getValueFor( "robotDataPort" ).c_str() ) );
    robotData.setBlocking( false );
    gDataSocketPtr = &robotData;

    sf::IpAddress receiveIP;
    unsigned short receivePort;

    sf::Packet dataPacket;
    /* ======================================== */

    sf::TcpSocket robotCmd;
    gCmdSocketPtr = &robotCmd;

    /* ===== GUI elements ===== */
    ProgressBar drive1Meter( Vector2i( 12 , 12 ) , L"0% Forward" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );
    gDrawables.push_back( &drive1Meter );

    ProgressBar drive2Meter( Vector2i( 12 , drive1Meter.getPosition().Y + drive1Meter.getSize().Y + 14 + 24 ) , L"0% Turn" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );
    gDrawables.push_back( &drive2Meter );

    ProgressBar turretMeter( Vector2i( streamWin.getPosition().X , 12 ) , L"Manual: 0%" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );
    gDrawables.push_back( &turretMeter );

    ProgressBar targetRPMMeter( Vector2i( streamWin.getPosition().X + 100 /* width of prev. bar */ + 10 , 12 ) , L"RPM \u2192 0" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );
    gDrawables.push_back( &targetRPMMeter );

    ProgressBar rpmMeter( Vector2i( streamWin.getPosition().X + streamWin.getSize().X - 100 /* width of this bar */ , 12 ) , L"RPM: 0" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );
    gDrawables.push_back( &rpmMeter );

    StatusLight isLowGearLight( Vector2i( 12  , 129 ) , L"Low Gear" );
    gDrawables.push_back( &isLowGearLight );
    StatusLight isHammerDownLight( Vector2i( 12 , 169 ) , L"Hammer Down" );
    gDrawables.push_back( &isHammerDownLight );

    StatusLight turretLockLight( Vector2i( streamWin.getPosition().X + streamWin.getSize().X + 10 , 110 ) , L"Lock" );
    gDrawables.push_back( &turretLockLight );

    StatusLight isAutoAimingLight( Vector2i( streamWin.getPosition().X + streamWin.getSize().X + 10 , 150 ) , L"Auto Aim" );
    gDrawables.push_back( &isAutoAimingLight );

    StatusLight kinectOnlineLight( Vector2i( streamWin.getPosition().X + streamWin.getSize().X + 10 , 190 ) , L"Kinect" );
    gDrawables.push_back( &kinectOnlineLight );

    StatusLight isShootingLight( Vector2i( streamWin.getPosition().X + streamWin.getSize().X + 10 , 230 ) , L"Shooter On" );
    gDrawables.push_back( &isShootingLight );

    StatusLight shooterManualLight( Vector2i( streamWin.getPosition().X + streamWin.getSize().X + 10 , 270 ) , L"Manual RPM" );
    gDrawables.push_back( &shooterManualLight );

    Text distanceText( Vector2i( streamWin.getPosition().X + streamWin.getSize().X + 10 , 66 ) , UIFont::getInstance()->segoeUI14() , RGB( 255 , 255 , 255 ) , RGB( 87 , 87 , 87 ) );
    distanceText.setString( L"0 ft" );
    gDrawables.push_back( &distanceText );
    /* ======================== */

    // Packet data
    std::string header;
    unsigned int drive1ScaleZ = 0;
    unsigned int drive2ScaleZ = 0;
    unsigned int turretScaleZ = 0;
    bool isLowGear = false;
    unsigned char isHammerDown = 0;
    unsigned int shooterRPM = 0;
    bool shooterIsManual = false;
    bool isShooting = false;
    bool isAutoAiming = false;
    bool turretLockedOn = false;
    unsigned char kinectOnline = sf::Socket::Error;
    unsigned int distanceToTarget;
    std::string tempAutonName;
    std::vector<std::string> autonNames;

    // Make sure the main window is shown before continuing
    ShowWindow( mainWindow , SW_SHOW );

    while ( !gExit ) {
        if ( PeekMessage( &message , NULL , 0 , 0 , PM_NOREMOVE ) ) {
            if ( GetMessage( &message , NULL , 0 , 0 ) > 0 ) {
                // If a message was waiting in the message queue, process it
                TranslateMessage( &message );
                DispatchMessage( &message );
            }
            else {
                gExit = true;
            }
        }
        else {
            streamWin.display();

            // Retrieve data sent from robot and unpack it
            if ( robotData.receive( dataPacket , receiveIP , receivePort ) == sf::Socket::Done ) {
                // Unpacks header telling packet type (either "display" or "autonList")
                dataPacket >> header;

                if ( std::strcmp( header.c_str() , "display" ) == 0 ) {
                    /* Unpacks the following variables:
                     *
                     * unsigned int: drive1 ScaleZ
                     * unsigned int: drive2 ScaleZ
                     * unsigned int: turret ScaleZ
                     * bool: drivetrain is in low gear
                     * unsigned char: is hammer mechanism deployed
                     * unsigned int: shooter RPM
                     * bool: shooter RPM control is manual
                     * bool: isShooting
                     * bool: isAutoAiming
                     * bool: turret is locked on
                     * unsigned char: Kinect is online
                     * unsigned int: distance to target
                     */

                    dataPacket >> drive1ScaleZ
                    >> drive2ScaleZ
                    >> turretScaleZ
                    >> isLowGear
                    >> isHammerDown
                    >> shooterRPM
                    >> shooterIsManual
                    >> isShooting
                    >> isAutoAiming
                    >> turretLockedOn
                    >> kinectOnline
                    >> distanceToTarget;
                }

                else if ( std::strcmp( header.c_str() , "autonList" ) == 0 ) {
                    /* Unpacks the following variables:
                     *
                     * Autonomous Modes (contained in rest of packet):
                     * std::string: autonomous routine name
                     * ...
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
                    }
                }

                /* ===== Adjust GUI interface to match data from robot ===== */
                drive1Meter.setPercent( static_cast<float>(drive1ScaleZ) / 100000.f * 100.f );
                drive1Meter.setString( numberToString( static_cast<float>(drive1ScaleZ) / 1000.f ) + L"%  Forward" );

                drive2Meter.setPercent( static_cast<float>(drive2ScaleZ) / 100000.f * 100.f );
                drive2Meter.setString( numberToString( static_cast<float>(drive2ScaleZ) / 1000.f ) + L"%  Turn" );

                turretMeter.setPercent( static_cast<float>(turretScaleZ) / 100000.f * 100.f );
                turretMeter.setString( L"Manual: " + numberToString( static_cast<float>(turretScaleZ) / 1000.f ) + L"%" );

                targetRPMMeter.setPercent( static_cast<float>(turretScaleZ) / 100000.f * 100.f );
                targetRPMMeter.setString( L"RPM \u2192 " + numberToString( static_cast<float>(turretScaleZ) / 100000.f * 4260.f ) );

                if ( isLowGear ) {
                    isLowGearLight.setActive( StatusLight::active );
                }
                else {
                    isLowGearLight.setActive( StatusLight::inactive );
                }

                if ( isHammerDown == 1 ) { // if hammer is deployed
                    isHammerDownLight.setActive( StatusLight::active );
                }
                else if ( isHammerDown == 2 ) { // if hammer is transitioning between deployed and retracted
                    isHammerDownLight.setActive( StatusLight::standby );
                }
                else { // else hammer is retracted
                    isHammerDownLight.setActive( StatusLight::inactive );
                }

                rpmMeter.setPercent( static_cast<float>(shooterRPM) / 100000.f / 4260.f * 100.f );
                rpmMeter.setString( L"RPM: " + numberToString( static_cast<float>(shooterRPM) / 100000.f ) );

                if ( shooterIsManual ) {
                    shooterManualLight.setActive( StatusLight::active );
                }
                else {
                    shooterManualLight.setActive( StatusLight::inactive );
                }

                turretLockLight.setActive( turretLockedOn ? StatusLight::active : StatusLight::inactive );

                if ( kinectOnline == sf::Socket::Done ) {
                    kinectOnlineLight.setActive( StatusLight::active );
                }
                else if ( kinectOnline == sf::Socket::NotReady ) {
                    kinectOnlineLight.setActive( StatusLight::standby );
                }
                else {
                    kinectOnlineLight.setActive( StatusLight::inactive );
                }

                isShootingLight.setActive( isShooting ? StatusLight::active : StatusLight::inactive );

                isAutoAimingLight.setActive( isAutoAiming ? StatusLight::active : StatusLight::inactive );

                /* Sets distance to target to be displayed
                 * 0.00328084f converts from millimeters to feet
                 */
                distanceText.setString( numberToString( distanceToTarget * 0.00328084f ) + L" ft" );
                /* ========================================================= */
			}

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
                if ( gMainWinPtr != NULL ) {
                    DestroyWindow( gMainWinPtr );
                }

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

        // Creates 1:1 relationship between logical units and pixels
        int oldMapMode = SetMapMode( hdc , MM_TEXT );

        for ( unsigned int i = 0 ; i < gDrawables.size() ; i++ ) {
            gDrawables[i]->draw( hdc );
        }

        // Restore old DC mapping mode
        SetMapMode( hdc , oldMapMode );

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
