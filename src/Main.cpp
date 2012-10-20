//=============================================================================
//File Name: Main.cpp
//Description: Receives data from the robot and displays it in a GUI
//Author: Tyler Veness
//=============================================================================

#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/UdpSocket.hpp>

#include <sstream>

#include "ProgressBar.hpp"
#include "StatusLight.hpp"
#include "MJPEG/MjpegStream.hpp"

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstring>

// Define child-window identifiers for catching their window events
enum {
	IDC_EXIT_BUTTON = 101,
	IDC_STREAM_BUTTON = 102
};

// Global because the window is closed by a button in CALLBACK OnEvent
sf::RenderWindow drawWin;

// Allows manipulation of MjpegStream in CALLBACK OnEvent
MjpegStream* streamWinPtr;

template <class T>
std::wstring numberToString( T number ) {
	return static_cast<std::wostringstream*>( &(std::wostringstream() << number) )->str();
}

LRESULT CALLBACK OnEvent( HWND Handle , UINT Message , WPARAM WParam , LPARAM LParam );

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

	MSG Message;

	char name[256];
	DWORD size = 256;
	GetUserNameA( name , &size );

	int mainWinHeight;

	if ( std::strcmp( name , "Driver" ) == 0 ) {
		// There is no task bar in the "Driver" profile
		mainWinHeight = GetSystemMetrics(SM_CYSCREEN) - 200;
	}
	else {
		// We need to make room for a task bar
		mainWinHeight = GetSystemMetrics(SM_CYSCREEN) - 229;
	}

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

	MjpegStream streamWin( "germany.acfsys.net" , //"10.35.12.6" ,
			8080 ,
			mainWindow ,
			( GetSystemMetrics(SM_CXSCREEN) - 320 ) / 2 ,
			60 ,
			320 ,
			240 ,
			Instance );
	streamWinPtr = &streamWin;

	sf::UdpSocket robotData;
	robotData.bind( 5615 );
	robotData.setBlocking( false );

	sf::IpAddress receiveIP;
	unsigned short receivePort;

	sf::Packet dataPacket;

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

	StatusLight isHighGearLight( sf::Vector2f( 12.f , 129.f ) , "High Gear" );
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
	bool isHighGear = false;
	bool isHammerDown = false;
	unsigned int shooterRPM = 0;
	bool shooterIsManual = false;
	bool isShooting = false;
	bool isAutoAiming = false;
	bool turretLockedOn = false;
	unsigned char kinectOnline = sf::Socket::Error;
	unsigned int distanceToTarget;

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
			     * bool: drivetrain is in high gear
			     * bool: is hammer mechanism deployed
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
				>> isHighGear
				>> isHammerDown
				>> shooterRPM
				>> shooterIsManual
				>> isShooting
				>> isAutoAiming
				>> turretLockedOn
				>> kinectOnline
				>> distanceToTarget;

				/* ===== Adjust GUI interface to match data from robot ===== */
				if ( isHighGear ) {
					isHighGearLight.setActive( StatusLight::active );
				}
				else {
					isHighGearLight.setActive( StatusLight::inactive );
				}

				if ( isHammerDown ) {
					isHammerDownLight.setActive( StatusLight::active );
				}
				else {
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

			drawWin.draw( drive1Meter );
			drawWin.draw( drive2Meter );
			drawWin.draw( turretMeter );
			drawWin.draw( isHighGearLight );
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

			Sleep( 50 );
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

		HWND exitButton = CreateWindowEx( 0,
			"BUTTON",
			"Exit",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			GetSystemMetrics(SM_CXSCREEN) - 100 - 5,
			5,
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

		HWND streamButton = CreateWindowEx( 0,
			"BUTTON",
			"Toggle Stream",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			( GetSystemMetrics(SM_CXSCREEN) - 320 ) / 2 ,
			305,
			100,
			24,
			Handle,
			reinterpret_cast<HMENU>( IDC_STREAM_BUTTON ),
			GetModuleHandle( NULL ),
			NULL);

		SendMessage(streamButton,
			WM_SETFONT,
			reinterpret_cast<WPARAM>( hfDefault ),
			MAKELPARAM( FALSE , 0 ) );

		break;
	}

	case WM_COMMAND: {
		switch( LOWORD(WParam) ) {
			case IDC_EXIT_BUTTON: {
				drawWin.close();
				PostQuitMessage(0);
				break;
			}
			break;

			case IDC_STREAM_BUTTON: {
				if ( streamWinPtr->isStreaming() ) {
					// Stop streaming
					streamWinPtr->stopStream();
				}
				else {
					// Start streaming
					streamWinPtr->startStream();
				}
				break;
			}

			case WM_DESTROY: {
				PostQuitMessage(0);
				return 0;
			}
			break;
		}
		break;
	}

	// Quit when we close the main window
	case WM_CLOSE: {
		drawWin.close();
		PostQuitMessage(0);
		break;
	}

	default: {
		return DefWindowProc(Handle, Message, WParam, LParam);
	}
	}

	return 0;
}
