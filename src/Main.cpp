//=============================================================================
//File Name: Main.cpp
//Description: Receives data from the robot and displays it in a GUI
//Author: Tyler Veness
//=============================================================================

/* TODO Get position of "DriverStation" window and adjust main window height
 * based upon that. Use a default height if not found.
 * TODO Add buttons for rebooting robot and reloading robot's settings file
 * TODO Put socket receiving in separate threads with signals and PostThreadMessage
 * (capture SIGQOUIT or SIGKILL?)
 */

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <sstream>

#include "WinGDI/ProgressBar.hpp"
#include "WinGDI/StatusLight.hpp"
#include "WinGDI/Text.hpp"
#include "WinGDI/UIFont.hpp"
#include "MJPEG/MjpegStream.hpp"
#include "Settings.hpp"
#include "ButtonID.hpp"

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wingdi.h>
#include <cstring>

// Global because the window is closed by a button in CALLBACK OnEvent
HWND gMainWinPtr = NULL;

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
    WindowClass.hCursor       = NULL;
    WindowClass.hbrBackground = mainBrush;
    WindowClass.lpszMenuName  = NULL;
    WindowClass.lpszClassName = mainClassName;
    RegisterClassEx(&WindowClass);

    MSG message;

    int mainWinHeight = GetSystemMetrics(SM_CYSCREEN) - 240;

    // Create a new window to be used for the lifetime of the application
    HWND mainWindow = CreateWindowEx( 0 ,
            mainClassName ,
            "" ,
            WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS ,
            0 ,
            0 ,
            GetSystemMetrics(SM_CXSCREEN) ,
            mainWinHeight ,
            NULL ,
            NULL ,
            Instance ,
            NULL );
    gMainWinPtr = mainWindow;

    Settings settings( "IPsettings.txt" );

    MjpegStream streamWin( settings.getValueFor( "streamHost" ) ,
            std::atoi( settings.getValueFor( "streamPort" ).c_str() ) ,
            mainWindow ,
            ( GetSystemMetrics(SM_CXSCREEN) - 320 ) / 2 ,
            60 ,
            320 ,
            240 ,
            Instance );
    gStreamWinPtr = &streamWin;

    /* ===== Robot Data Sending Variables ===== */
    sf::UdpSocket robotData;
    robotData.bind( std::atoi( settings.getValueFor( "robotDataPort" ).c_str() ) );
    robotData.setBlocking( false );
    gDataSocketPtr = &robotData;

    sf::IpAddress receiveIP;
    unsigned short receivePort;

    sf::Packet dataPacket;
    /* ======================================== */

    sf::TcpSocket robotCmd;
    gCmdSocketPtr = &robotCmd;

    // Used for drawing all GUI elements
    HDC mainDC = NULL;

    /* ===== GUI elements ===== */
    ProgressBar drive1Meter( Vector( 12 , 12 ) , L"" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );

    ProgressBar drive2Meter( Vector( 12 , drive1Meter.getPosition().Y + drive1Meter.getSize().Y + 14 + 24 ) , L"" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );

    ProgressBar turretMeter( Vector( streamWin.getPosition().x , 12 ) , L"" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );

    ProgressBar targetRPMMeter( Vector( streamWin.getPosition().x + 100 /* width of prev. bar */ + 10 , 12 ) , L"" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );

    ProgressBar rpmMeter( Vector( streamWin.getPosition().x + streamWin.getSize().x - 100 /* width of this bar */ , 12 ) , L"" , RGB( 0 , 120 , 0 ) , RGB( 40 , 40 , 40 ) , RGB( 50 , 50 , 50 ) );

    StatusLight isLowGearLight( Vector( 12  , 129 ) , L"Low Gear" );
    StatusLight isHammerDownLight( Vector( 12 , 169 ) , L"Hammer Down" );

    StatusLight turretLockLight( Vector( streamWin.getPosition().x + streamWin.getSize().x + 10 , 110 ) , L"Lock" );

    StatusLight isAutoAimingLight( Vector( streamWin.getPosition().x + streamWin.getSize().x + 10 , 150 ) , L"Auto Aim" );

    StatusLight kinectOnlineLight( Vector( streamWin.getPosition().x + streamWin.getSize().x + 10 , 190 ) , L"Kinect" );

    StatusLight isShootingLight( Vector( streamWin.getPosition().x + streamWin.getSize().x + 10 , 230 ) , L"Shooter On" );

    StatusLight shooterManualLight( Vector( streamWin.getPosition().x + streamWin.getSize().x + 10 , 270 ) , L"Manual RPM" );

    Text distanceText( Vector( streamWin.getPosition().x + streamWin.getSize().x + 10 , 66 ) , RGB( 255 , 255 , 255 ) , RGB( 87 , 87 , 87 ) );
    distanceText.setString( L"0 ft" );
    /* ======================== */

    // Packet data
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

    ShowWindow( mainWindow , SW_SHOW ); // Makes sure this window is shown before continuing

    while ( !gExit ) {
        streamWin.bringToTop(); // bring window to top of other child windows

        if ( PeekMessage( &message , NULL , 0 , 0 , PM_REMOVE ) ) {
            // If a message was waiting in the message queue, process it
            TranslateMessage( &message );
            DispatchMessage( &message );
        }
        else {
            // Retrieve data sent from robot and unpack it
            if ( robotData.receive( dataPacket , receiveIP , receivePort ) == sf::Socket::Done ) {
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

                /* ===== Adjust GUI interface to match data from robot ===== */
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

            drive1Meter.setPercent( static_cast<float>(drive1ScaleZ) / 100000.f * 100.f );
            drive1Meter.setString( numberToString( static_cast<float>(drive1ScaleZ) / 1000.f ) + L"%  Forward" );

            drive2Meter.setPercent( static_cast<float>(drive2ScaleZ) / 100000.f * 100.f );
            drive2Meter.setString( numberToString( static_cast<float>(drive2ScaleZ) / 1000.f ) + L"%  Turn" );

            turretMeter.setPercent( static_cast<float>(turretScaleZ) / 100000.f * 100.f );
            turretMeter.setString( L"Manual: " + numberToString( static_cast<float>(turretScaleZ) / 1000.f ) + L"%" );

            targetRPMMeter.setPercent( static_cast<float>(turretScaleZ) / 100000.f * 100.f );
            targetRPMMeter.setString( L"RPM \u2192 " + numberToString( static_cast<float>(turretScaleZ) / 100000.f * 4260.f ) );

            rpmMeter.setPercent( static_cast<float>(shooterRPM) / 100000.f / 4260.f * 100.f );
            rpmMeter.setString( L"RPM: " + numberToString( static_cast<float>(shooterRPM) / 100000.f ) );

            mainDC = GetDC( mainWindow );

            // Creates 1:1 relationship between logical units and pixels
            SetMapMode( mainDC , MM_TEXT );

            drive1Meter.draw( mainDC );
            drive2Meter.draw( mainDC );
            turretMeter.draw( mainDC );
            isLowGearLight.draw( mainDC );
            isHammerDownLight.draw( mainDC );
            targetRPMMeter.draw( mainDC );
            rpmMeter.draw( mainDC );
            shooterManualLight.draw( mainDC );
            turretLockLight.draw( mainDC );
            isShootingLight.draw( mainDC );
            isAutoAimingLight.draw( mainDC );
            kinectOnlineLight.draw( mainDC );
            distanceText.draw( mainDC );

            DeleteObject( mainDC );

            streamWin.display();

            Sleep( 30 );
        }
    }

    UIFont::freeInstance();

    // Clean up windows
    DestroyWindow( mainWindow );
    UnregisterClass( mainClassName , Instance );

    return EXIT_SUCCESS;
}

LRESULT CALLBACK OnEvent( HWND Handle , UINT Message , WPARAM WParam , LPARAM LParam ) {
    switch ( Message ) {
    case WM_CREATE: {
        HGDIOBJ hfDefault = GetStockObject( DEFAULT_GUI_FONT );

        HWND connectButton = CreateWindowEx( 0,
            "BUTTON",
            "Connect DS",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            0 * ( 5 + 24 ) + 5,
            100,
            24,
            Handle,
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
            1 * ( 5 + 24 ) + 5,
            100,
            24,
            Handle,
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
            2 * ( 5 + 24 ) + 5,
            100,
            24,
            Handle,
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
            3 * ( 5 + 24 ) + 5,
            100,
            24,
            Handle,
            reinterpret_cast<HMENU>( IDC_EXIT_BUTTON ),
            GetModuleHandle( NULL ),
            NULL);

        SendMessage(exitButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        break;
    }

    case WM_COMMAND: {
        char* data = static_cast<char*>( std::malloc( 16 ) );

        sf::IpAddress remoteIP( 10 , 35 , 12 , 2 );
        unsigned short remotePort = 5615;


        switch( LOWORD(WParam) ) {
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
                std::strcpy( data , "connect" );

                if ( gDataSocketPtr != NULL ) {
                    gDataSocketPtr->send( data , 16 , remoteIP , remotePort );
                }

                break;
            }

            // These other commands get sent to ALF rather than the robot
            case IDC_RELOAD_BUTTON: {
                std::strcpy( data , "reload\r\n" );

                if ( gCmdSocketPtr != NULL ) {
                    gCmdSocketPtr->connect( remoteIP , 3512 );
                    gCmdSocketPtr->send( data , 16 );
                    gCmdSocketPtr->disconnect();
                }

                break;
            }

            case IDC_REBOOT_BUTTON: {
                std::strcpy( data , "reboot\r\n" );

                if ( gCmdSocketPtr != NULL ) {
                    gCmdSocketPtr->connect( remoteIP , 3512 );
                    gCmdSocketPtr->send( data , 16 );
                    gCmdSocketPtr->disconnect();
                }

                break;
            }

            case IDC_EXIT_BUTTON: {
                if ( gMainWinPtr != NULL ) {
                    DestroyWindow( gMainWinPtr );
                }

                gExit = true;
                PostQuitMessage(0);

                break;
            }

            case WM_DESTROY: {
                gExit = true;
                PostQuitMessage(0);
            }
            break;
        }

        free( data );

        break;
    }

    default: {
        return DefWindowProc(Handle, Message, WParam, LParam);
    }
    }

    return 0;
}
