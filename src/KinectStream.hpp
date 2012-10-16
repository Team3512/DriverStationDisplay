//=============================================================================
//File Name: KinectStream.hpp
//Description: Receives an MJPEG stream and displays it in a child window with
//             the specified properties
//Author: Tyler Veness
//=============================================================================

#ifndef KINECT_STREAM_HPP
#define KINECT_STREAM_HPP

/* This class creates a child window that receives MJPEG images and displays
 * them from a separate thread.
 *
 * To start using this class, just create an instance of it; everything else
 * is handled in a spawned thread
 *
 * Call startStream() to start the MJPEG stream or stopStream() to stop it
 * manually.
 *
 * startStream() and stopStream() are called automatically in the constructor
 * and destructor respectively, but they can be called manually if desired.
 *
 */

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/System/Thread.hpp>

#include "ModSFML/ContHttp.hpp"

class KinectStream {
public:
	KinectStream( const std::string& hostName ,
			unsigned short port ,
			HWND parentWin ,
			int xPosition ,
			int yPosition ,
			int width ,
			int height ,
			HINSTANCE appInstance
			);
	~KinectStream();

	// Puts this window in the foreground in case there are other child windows
	void bringToTop();

	sf::Vector2i getPosition() const;
	void setPosition( const sf::Vector2i& position );

	sf::Vector2u getSize() const;
	void setSize( const sf::Vector2u size );

	// Starts Kinect stream (doesn't reopen window)
	void startStream();

	// Stops Kinect stream (doesn't close window)
	void stopStream();

private:
	std::string m_hostName;
	unsigned short m_port;

	HWND m_parentWin;

	HWND m_streamWin;
	sf::RenderWindow m_streamDisplay;

	// New HTTP client
	sf::ContHttp m_httpStream;

	// Request for the Kinect's image in "/"
	sf::ContHttp::Request m_imageRequest;

	// Stores the status of the request in "response"
	sf::ContHttp::Response m_serverResponse;

	// Stores the status of the image receive
	sf::ContHttp::Response::Status m_recvStatus;

	sf::Thread m_receiveThread;
	volatile bool m_closeThread;

	void receive();
};

#endif // KINECT_STREAM_HPP
