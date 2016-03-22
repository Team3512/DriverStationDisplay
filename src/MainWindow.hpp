// =============================================================================
// Description: Creates application's main window
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <memory>

#include <QComboBox>
#include <QHostAddress>
#include <QMainWindow>
#include <QTimer>
#include <QUdpSocket>
#include <QVBoxLayout>

#include "MJPEG/mjpeg_sck.hpp"
#include "MJPEG/WindowCallbacks.hpp"
#include "Settings.hpp"

class ClientBase;
class QAction;
class QMenu;
class QPushButton;
class QSlider;
class VideoStream;

class MainWindow : public QMainWindow {
    Q_OBJECT
    QWidget* centralWidget;

public:
    MainWindow(int width, int height);

private slots:
    void startMJPEG();
    void stopMJPEG();
    void about();

    void toggleButton();
    void handleSocketData();

private:
    void createActions();
    void createMenus();

    // Clears left and right layout
    void clearGUI();

    // Clears a specific layout
    void clearLayout(QLayout* layout);

    // Updates list of elements from packet and recreates them
    void reloadGUI(std::vector<char>& data, size_t& pos);

    // Updates list of elements from file and recreates them
    void reloadGUI(const std::string& fileName);

    // Updates values of elements from packet
    void updateGuiTable(std::vector<char>& data, size_t& pos);

    std::unique_ptr<Settings> m_settings;

    WindowCallbacks m_streamCallback;
    ClientBase* m_client;
    VideoStream* m_stream;
    QPushButton* m_button;

    // Allows the user to select which autonomous mode the robot shoud run
    QComboBox* m_autoSelect;

    // Holds dynamically created widgets in left column
    QVBoxLayout* m_leftLayout;

    // Holds dynamically created widgets in center column
    QVBoxLayout* m_centerLayout;

    // Holds dynamically created widgets in right column
    QVBoxLayout* m_rightLayout;

    // Holds robot configuration options on right edge of window
    QVBoxLayout* m_optionLayout;

    QMenu* m_optionsMenu;
    QMenu* m_helpMenu;
    QAction* m_startMJPEGAct;
    QAction* m_stopMJPEGAct;
    QAction* m_exitAct;
    QAction* m_aboutAct;

    std::unique_ptr<QUdpSocket> m_dataSocket;
    QHostAddress m_remoteIP;
    unsigned short m_dataPort;
    bool m_connectDlgOpen{false};
    bool m_connectedBefore{false};

    std::vector<char> m_buffer;

    std::unique_ptr<QTimer> m_connectTimer;

    // DisplaySettings
    std::string m_substring;
    size_t m_start;
    size_t m_end;

    std::string m_delimiter;

    std::string m_currentType;
    std::string m_column;
    std::wstring m_startText;
    std::wstring m_updateText;
    std::vector<std::string> m_tempVarIds;

    std::vector<std::string> split(const std::string& s,
                                   const std::string& delim);

    /* Parses a given settings line, adding an element if applicable and
     * incrementing the global position variables for the column to which
     * the element was added
     */
    void parseLine(std::string line);

    // Resets all temporary variables to be reused during a call to update(1)
    void resetAllTemps();
};

#endif // MAIN_WINDOW_HPP
