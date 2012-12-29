//=============================================================================
//File Name: Main.cpp
//Description: Receives data from the robot and displays it in a GUI
//Author: Tyler Veness
//=============================================================================

/* TODO Get position of "DriverStation" window and adjust main window height
 * based upon that. Use a default height if not found.
 * TODO Add buttons for rebooting robot and reloading robot's settings file
 */

#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <sstream>
#include <vector>
#include <string>

#include "ProgressBar.hpp"
#include "StatusLight.hpp"
#include "MJPEG/MjpegStream.hpp"
#include "Settings.hpp"
#include "Resource.h"

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define _WIN32_IE 0x0400
#include <commctrl.h>

#include <cstring>

// Global because the window is closed by a button in CALLBACK OnEvent
sf::RenderWindow* gDrawWinPtr = NULL;

// Allows manipulation of MjpegStream in CALLBACK OnEvent
MjpegStream* gStreamWinPtr = NULL;

// Allows usage of socket in CALLBACK OnEvent
sf::UdpSocket* gDataSocketPtr = NULL;
sf::TcpSocket* gCmdSocketPtr = NULL;

template <class T>
std::wstring numberToString( T number ) {
    return static_cast<std::wostringstream*>( &(std::wostringstream() << number) )->str();
}

LRESULT CALLBACK OnEvent( HWND Handle , UINT Message , WPARAM WParam , LPARAM LParam );

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
    WindowClass.hCursor       = NULL;
    WindowClass.hbrBackground = mainBrush;
    WindowClass.lpszMenuName  = NULL;
    WindowClass.lpszClassName = mainClassName;
    RegisterClassEx(&WindowClass);

    MSG Message;

    int mainWinHeight = GetSystemMetrics(SM_CYSCREEN) - 240;

    // Create a new window to be used for the lifetime of the application
    HWND mainWindow = CreateWindowEx( 0 ,
            mainClassName ,
            "" ,
            WS_POPUP | WS_VISIBLE ,
            0 ,
            0 ,
            GetSystemMetrics(SM_CXSCREEN) ,
            mainWinHeight ,
            NULL ,
            NULL ,
            Instance ,
            NULL );

    sf::RenderWindow drawWin;

    HWND drawWindow = CreateWindowEx( 0 ,
            "STATIC" ,
            "" ,
            WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE ,
            0 ,
            0 ,
            GetSystemMetrics(SM_CXSCREEN) ,
            mainWinHeight ,
            mainWindow ,
            NULL ,
            Instance ,
            NULL );
    drawWin.create( drawWindow );
    gDrawWinPtr = &drawWin;

    Settings settings( "IPSettings.txt" );

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

    /* ===== GUI elements ===== */
    ProgressBar drive1Meter( sf::Vector2f( 100.f , 19.f ) , sf::String( "" ) , sf::Color( 0 , 120 , 0 ) , sf::Color( 40 , 40 , 40 ) , sf::Color( 50 , 50 , 50 ) );
    drive1Meter.setPosition( 12.f , 12.f );

    ProgressBar drive2Meter( sf::Vector2f( 100.f , 19.f ) , sf::String( "" ) , sf::Color( 0 , 120 , 0 ) , sf::Color( 40 , 40 , 40 ) , sf::Color( 50 , 50 , 50 ) );
    drive2Meter.setPosition( 12.f , drive1Meter.getPosition().y + drive1Meter.getSize().y + 14.f + 24.f );

    ProgressBar turretMeter( sf::Vector2f( 100.f , 19.f ) , sf::String( "" ) , sf::Color( 0 , 120 , 0 ) , sf::Color( 40 , 40 , 40 ) , sf::Color( 50 , 50 , 50 ) );
    turretMeter.setPosition( streamWin.getPosition().x , 12.f );

    ProgressBar targetRPMMeter( sf::Vector2f( 100.f , 19.f ) , sf::String( "" ) , sf::Color( 0 , 120 , 0 ) , sf::Color( 40 , 40 , 40 ) , sf::Color( 50 , 50 , 50 ) );
    targetRPMMeter.setPosition( streamWin.getPosition().x + 100.f /* width of prev. bar */ + 10.f , 12.f );

    ProgressBar rpmMeter( sf::Vector2f( 100.f , 19.f ) , sf::String( "" ) , sf::Color( 0 , 120 , 0 ) , sf::Color( 40 , 40 , 40 ) , sf::Color( 50 , 50 , 50 ) );
    rpmMeter.setPosition( streamWin.getPosition().x + streamWin.getSize().x - 100.f /* width of this bar */ , 12.f );

    StatusLight isLowGearLight( sf::Vector2f( 12.f , 129.f ) , "Low Gear" );
    StatusLight isHammerDownLight( sf::Vector2f( 12.f , 169.f ) , "Hammer Down" );

    StatusLight turretLockLight( sf::Vector2f( streamWin.getPosition().x + streamWin.getSize().x + 10.f , 110.f ) , "Lock" );
    StatusLight isAutoAimingLight( sf::Vector2f( streamWin.getPosition().x + streamWin.getSize().x + 10.f , 150.f ) , "Auto Aim" );
    StatusLight kinectOnlineLight( sf::Vector2f( streamWin.getPosition().x + streamWin.getSize().x + 10.f , 190.f ) , "Kinect" );
    StatusLight isShootingLight( sf::Vector2f( streamWin.getPosition().x + streamWin.getSize().x + 10.f , 230.f ) , "Shooter On" );
    StatusLight shooterManualLight( sf::Vector2f( streamWin.getPosition().x + streamWin.getSize().x + 10.f , 270.f ) , "Manual RPM" );

    sf::Text distanceText( "0 ft" , UIFont::getInstance()->segoeUI() , 14 );
    distanceText.setPosition( streamWin.getPosition().x + streamWin.getSize().x + 10.f , 66.f );
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
    std::string tempAutonName;
    std::vector<std::string> autonNames;

    ShowWindow( mainWindow , SW_SHOW ); // Makes sure this window is shown before continuing

    while ( drawWin.isOpen() ) {
        streamWin.bringToTop(); // bring window to top of other child windows

        if ( PeekMessage( &Message , NULL , 0 , 0 , PM_REMOVE ) ) {
            // If a message was waiting in the message queue, process it
            TranslateMessage( &Message );
            DispatchMessage( &Message );
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
                 *
                 * Autonomous Modes (contained in rest of packet):
                 * std::string: autonomous routine name
                 * ...
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

                autonNames.clear();
                while ( !dataPacket.endOfPacket() ) {
                    dataPacket >> tempAutonName;
                    autonNames.push_back( tempAutonName );
                }

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

            drawWin.clear( sf::Color( 87 , 87 , 87 ) );
            //drawWin.clear( sf::Color( 240 , 240 , 240 ) );

            drawWin.draw( drive1Meter );
            drawWin.draw( drive2Meter );
            drawWin.draw( turretMeter );
            drawWin.draw( isLowGearLight );
            drawWin.draw( isHammerDownLight );
            drawWin.draw( targetRPMMeter );
            drawWin.draw( rpmMeter );
            drawWin.draw( shooterManualLight );
            drawWin.draw( turretLockLight );
            drawWin.draw( isShootingLight );
            drawWin.draw( isAutoAimingLight );
            drawWin.draw( kinectOnlineLight );
            drawWin.draw( distanceText );

            drawWin.display();
            streamWin.display();

            Sleep( 30 );
        }
    }

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
            0 * ( 5 + 28 ) + 5,
            100,
            28,
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
            1 * ( 5 + 28 ) + 5,
            100,
            28,
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
            2 * ( 5 + 28 ) + 5,
            100,
            28,
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
            3 * ( 5 + 28 ) + 5,
            100,
            28,
            Handle,
            reinterpret_cast<HMENU>( IDC_EXIT_BUTTON ),
            GetModuleHandle( NULL ),
            NULL);

        SendMessage(exitButton,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        HWND comboBox = CreateWindowEx( 0,
            "COMBOBOX",
            "Autonomous Mode",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN,
            GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
            4 * ( 5 + 28 ) + 5,
            100,
            GetSystemMetrics(SM_CYSCREEN) - 240,
            Handle,
            reinterpret_cast<HMENU>( IDC_AUTON_COMBOBOX ),
            GetModuleHandle( NULL ),
            NULL );

        SendMessage(comboBox,
            WM_SETFONT,
            reinterpret_cast<WPARAM>( hfDefault ),
            MAKELPARAM( FALSE , 0 ) );

        const char* comboBoxItems[] = { "AutonShoot" , "AutonBridge" , "AutonFeed" };

        for ( unsigned int i = 0 ; i < 3 ; i++ ) {
            SendMessage(comboBox,
                CB_ADDSTRING,
                0,
                reinterpret_cast<LPARAM>( (LPCTSTR)comboBoxItems[i] ));
        }

        SendMessage( comboBox , CB_SETCURSEL , 0 , 0 );

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
                std::strcpy( data , "connect\r\n" );

                if ( gDataSocketPtr != NULL ) {
                    gDataSocketPtr->send( data , 16 , remoteIP , remotePort );
                }

                break;
            }

            // These other commands get sent to ALF rather than the robot
            case IDC_RELOAD_BUTTON: {
                std::strcpy( data , "reload\r\n" );

                if ( gCmdSocketPtr != NULL ) {
                    gCmdSocketPtr->connect( remoteIP , 3512 , sf::milliseconds( 500 ) );
                    gCmdSocketPtr->send( data , 16 );
                    gCmdSocketPtr->disconnect();
                }

                break;
            }

            case IDC_REBOOT_BUTTON: {
                std::strcpy( data , "reboot\r\n" );

                if ( gCmdSocketPtr != NULL ) {
                    gCmdSocketPtr->connect( remoteIP , 3512 , sf::milliseconds( 500 ) );
                    gCmdSocketPtr->send( data , 16 );
                    gCmdSocketPtr->disconnect();
                }

                break;
            }

            case IDC_EXIT_BUTTON: {
                if ( gDrawWinPtr != NULL ) {
                    gDrawWinPtr->close();
                }

                PostQuitMessage(0);
                break;
            }

            case IDC_AUTON_COMBOBOX: {
                switch ( HIWORD(WParam) ) {
                case CBN_SELCHANGE: {
                    // Get new Auton selection since it changed
                    int selection = SendMessage( (HWND)LParam , CB_GETCURSEL , 0 , 0 );

                    // If it's really a selection, tell the robot to change Autonomous
                    if ( selection != CB_ERR ) {
                        std::strcpy( data , "autonSelect\r\n" );
                        data[13] = 0;

                        if ( gDataSocketPtr != NULL ) {
                            gDataSocketPtr->send( data , 16 , remoteIP , remotePort );
                        }
                    }
                }
                }

                break;
            }

            case WM_DESTROY: {
                PostQuitMessage(0);

                break;
            }
        }

        std::free( data );

        break;
    }

    // Quit when we close the main window
    case WM_CLOSE: {
        if ( gDrawWinPtr != NULL ) {
            gDrawWinPtr->close();
        }

        PostQuitMessage(0);
        break;
    }

    default: {
        return DefWindowProc(Handle, Message, WParam, LParam);
    }
    }

    return 0;
}
