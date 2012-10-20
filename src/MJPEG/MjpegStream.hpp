//=============================================================================
//File Name: KinectStream.hpp
//Description: Receives an MJPEG stream and displays it in a child window with
//             the specified properties
//Author: Tyler Veness
//=============================================================================

#ifndef MJPEG_STREAM_HPP
#define MJPEG_STREAM_HPP

/* This class creates a child window that receives MJPEG images and displays
 * them from a separate thread.
 *
 * To start using this class, just create an instance of it; everything else
 * is handled in a spawned thread
 *
 * Call startStream() to start the MJPEG stream or stopStream() to stop it
 * manually. This won't open or close the window.
 *
 * startStream() and stopStream() are called automatically in the constructor
 * and destructor respectively, but they can be called manually if desired.
 *
 */

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Thread.hpp>

extern "C" {
#include "mjpegrx.h"
}

class MjpegStream {
public:
	MjpegStream( const std::string& hostName ,
			unsigned short port ,
			HWND parentWin ,
			int xPosition ,
			int yPosition ,
			int width ,
			int height ,
			HINSTANCE appInstance
			);
	virtual ~MjpegStream();

	// Puts this window in the foreground in case there are other child windows
	void bringToTop();

	sf::Vector2i getPosition();
	void setPosition( const sf::Vector2i& position );

	sf::Vector2u getSize();
	void setSize( const sf::Vector2u size );

	// Starts Kinect stream
	void startStream();

	// Stops Kinect stream
	void stopStream();

	// Returns true if streaming is on
	bool isStreaming();

	// Displays the stream or a message if the stream isn't working
	void display();

protected:
	static void doneCallback( void* optarg );
	static void readCallback( char* buf , int bufsize , void* optarg );

private:
	std::string m_hostName;
	unsigned short m_port;

	HWND m_parentWin;

	HWND m_streamWin;
	sf::RenderWindow m_streamDisplay;

	// Contains "Connecting" message
	sf::RenderTexture m_connectTxtr;
	sf::Text m_connectMsg;

	// Contains "Disconnected" message
	sf::RenderTexture m_disconnectTxtr;
	sf::Text m_disconnectMsg;

	// Holds image most recently received from the host
	sf::Sprite m_imageSprite;
	sf::Mutex m_imageMutex;

	// Used for streaming MJPEG frames from host
	struct mjpeg_callbacks_t m_callbacks;
	struct mjpeg_inst_t* m_streamInst;

	// Determines when a video frame is old
	sf::Clock m_imageAge;

	// Locks display so only one thread can access or draw to it at a time
	sf::Mutex m_displayMutex;

	/* If true:
	 *     Lets receive threads run and closes disconnect display thread
	 * If false:
	 *     Lets disconnect display thread run and closes receive threads
	 */
	volatile bool m_stopReceive;
};

#endif // MJPEG_STREAM_HPP
