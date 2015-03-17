#include <QtWidgets>
#include <QPushButton>
#include <QSpacerItem>
#include <QHBoxLayout>

#include "MainWindow.hpp"

#include <fstream>
#include <cstring>
#include <chrono>

#include "NetUpdate/ProgressBar.hpp"
#include "NetUpdate/StatusLight.hpp"
#include "NetUpdate/Text.hpp"
#include "NetUpdate/UIFont.hpp"
#include "Util.hpp"

MainWindow::MainWindow(int width, int height) : m_buffer(0xffff - 28) {
    setMinimumSize(width, height);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    m_settings = std::make_unique<Settings>("IPSettings.txt");

    m_client = new MjpegStream(m_settings->getString("streamHost"),
                               m_settings->getInt("streamPort"),
                               m_settings->getString("streamRequestPath"),
                               this,
                               320,
                               240,
                               &m_streamCallback,
                               [this] {},
                               [this] { m_button->setText("Stop Stream"); },
                               [this] { m_button->setText("Start Stream"); });
    m_client->setMaximumSize(320, 240);

    m_button = new QPushButton("Start Stream");
    connect(m_button, SIGNAL(released()), this, SLOT(toggleButton()));

    m_leftLayout = new QVBoxLayout;

    m_centerLayout = new QVBoxLayout;

    m_leftLayout->insertStretch(0);
    m_centerLayout->addWidget(m_client, 0, Qt::AlignTop);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_button, 0, Qt::AlignTop);
    buttonLayout->insertStretch(1);
    m_centerLayout->addLayout(buttonLayout);

    m_rightLayout = new QVBoxLayout;

    m_optionLayout = new QVBoxLayout;

    m_autoSelect = new QComboBox;
    connect(m_autoSelect,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            [this] (int index) {
        char data[16] = "autonSelect\r\n";
        data[13] = index;

        m_dataSocket->writeDatagram(data, sizeof(data), m_remoteIP, m_dataPort);
    });
    m_optionLayout->addWidget(m_autoSelect, 0, Qt::AlignTop);

    QCheckBox* colorBlind = new QCheckBox(tr("Color Blind Mode"));
    connect(colorBlind, &QAbstractButton::clicked, [] (bool clicked) {
        StatusLight::setColorBlind(clicked);
    });
    m_optionLayout->addWidget(colorBlind, 0, Qt::AlignTop);

    QPushButton* exitButton = new QPushButton("Exit");
    connect(exitButton, &QPushButton::released, [this] {
        this->close();
    });
    m_optionLayout->addWidget(exitButton, 0, Qt::AlignTop);

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->setColumnMinimumWidth(0, width / 4);
    mainLayout->setColumnMinimumWidth(1, width / 4);
    mainLayout->setColumnMinimumWidth(2, width / 4);
    mainLayout->setColumnMinimumWidth(3, width / 4);

    mainLayout->addLayout(m_leftLayout, 0, 0);
    mainLayout->addLayout(m_centerLayout, 0, 1);
    mainLayout->addLayout(m_rightLayout, 0, 2);
    mainLayout->addLayout(m_optionLayout, 0, 3);

    centralWidget->setLayout(mainLayout);

    createActions();
    createMenus();

    setUnifiedTitleAndToolBarOnMac(true);

    m_dataSocket = std::make_unique<QUdpSocket>(this);
    m_dataSocket->bind(m_settings->getInt("dsDataPort"));
    connect(m_dataSocket.get(), SIGNAL(readyRead()),
            this, SLOT(handleSocketData()));

    m_remoteIP = QString::fromUtf8(m_settings->getString("robotIP").c_str());
    m_dataPort = m_settings->getInt("robotDataPort");

    m_connectTimer = std::make_unique<QTimer>();
    connect(m_connectTimer.get(), &QTimer::timeout, [this] {
        if (!m_connectDlgOpen) {
            char data[16] = "connect\r\n";
            m_dataSocket->writeDatagram(data, sizeof(data), m_remoteIP,
                                        m_dataPort);
        }

        m_connectTimer->start(2000);
    });
    m_connectTimer->start(2000);
}

void MainWindow::startMJPEG() {
    m_client->start();
}

void MainWindow::stopMJPEG() {
    m_client->stop();
}

void MainWindow::about() {
    QMessageBox::about(this, tr("About DriverStationDisplay"),
                       tr("<br>DriverStationDisplay, Version 2.0<br>"
                          "Copyright &copy;2012-2015 FRC Team 3512<br>"
                          "FRC Team 3512<br>"
                          "All Rights Reserved"));
}

void MainWindow::toggleButton() {
    if (m_client->isStreaming()) {
        stopMJPEG();
    }
    else {
        startMJPEG();
    }
}

void MainWindow::handleSocketData() {
    while (m_dataSocket->hasPendingDatagrams()) {
        size_t packetPos = 0;
        m_buffer.resize(m_dataSocket->pendingDatagramSize());
        m_dataSocket->readDatagram(m_buffer.data(), m_buffer.size());

        std::string header;
        packetToVar(m_buffer, packetPos, header);

        if (header == "display\r\n") {
            /* Only allow keep-alive (resetting timer) if we have a valid
             * GUI; we need to connect and create the GUI before accepting
             * display data
             */
            if (m_connectedBefore) {
                updateGuiTable(m_buffer, packetPos);
                NetUpdate::updateElements();

                m_connectTimer->start(2000);
            }
        }
        else if (header == "guiCreate\r\n") {
            reloadGUI(m_buffer, packetPos);

            if (!m_connectedBefore) {
                m_connectedBefore = true;
            }

            m_connectTimer->start(2000);
        }
        else if (header == "autonList\r\n") {
            /* Unpacks the following variables:
             *
             * Autonomous Modes (contained in rest of packet):
             * std::string: autonomous routine name
             * <more autonomous routine names>...
             */

            std::vector<std::string> autoNames;
            std::string autoName;
            while (packetPos < m_buffer.size() &&
                   packetToVar(m_buffer, packetPos, autoName)) {
                autoNames.push_back(autoName);
            }

            m_autoSelect->clear();
            for (auto& str : autoNames) {
                m_autoSelect->addItem(str.c_str());
            }
        }
        else if (header == "autonConfirmed\r\n") {
            /* If a new autonomous mode was selected from the robot, it
             * sends back this packet as confirmation
             */
            std::string autoName = "Autonomous mode changed to\n";

            std::string tempName;
            packetToVar(m_buffer, packetPos, tempName);
            autoName += tempName;

            static QMessageBox connectDlg;
            connectDlg.setWindowTitle("Autonomous Change");
            connectDlg.setText(autoName.c_str());
            connectDlg.show();
        }
    }
}

void MainWindow::createActions() {
    m_startMJPEGAct = new QAction(tr("&Start"), this);
    connect(m_startMJPEGAct, SIGNAL(triggered()), this, SLOT(startMJPEG()));

    m_stopMJPEGAct = new QAction(tr("&Stop"), this);
    connect(m_stopMJPEGAct, SIGNAL(triggered()), this, SLOT(stopMJPEG()));

    m_exitAct = new QAction(tr("&Exit"), this);
    connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

    m_aboutAct = new QAction(tr("&About DriverStationDisplay"), this);
    connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus() {
    m_optionsMenu = menuBar()->addMenu(tr("&Options"));
    m_optionsMenu->addAction(m_startMJPEGAct);
    m_optionsMenu->addAction(m_stopMJPEGAct);
    m_optionsMenu->addSeparator();
    m_optionsMenu->addAction(m_exitAct);

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAct);
}

void MainWindow::clearGUI() {
    clearLayout(m_leftLayout);
    clearLayout(m_rightLayout);
}

void MainWindow::clearLayout(QLayout* layout) {
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
}

void MainWindow::reloadGUI(std::vector<char>& data, size_t& pos) {
    // Remove old elements before creating new ones
    clearGUI();

    resetAllTemps();

    // Read the file into a string
    std::string tmpbuf;
    packetToVar(data, pos, tmpbuf);

    std::vector<std::string> lines = split(tmpbuf, "\n");
    for (auto& str : lines) {
        parseLine(str);
    }
}

void MainWindow::reloadGUI(const std::string& fileName) {
    std::ifstream guiSettings(fileName);

    // Remove old elements before creating new ones
    clearGUI();

    if (guiSettings.is_open()) {
        resetAllTemps();

        while (!guiSettings.eof()) {
            std::string temp;
            std::getline(guiSettings, temp);
            parseLine(std::move(temp));
        }
    }
}

void MainWindow::updateGuiTable(std::vector<char>& data, size_t& pos) {
    NetUpdate::updateValues(data, pos);
}

std::vector<std::string> MainWindow::split(const std::string& s,
                                           const std::string& delim) {
    std::vector<std::string> elems;

    size_t pos = 0;
    size_t nextPos = 0;
    while (nextPos != std::string::npos) {
        nextPos = s.find_first_of(delim, pos);
        elems.emplace_back(s.substr(pos, nextPos - pos));
        pos = nextPos + delim.length();
    }

    return elems;
}

void MainWindow::parseLine(std::string line) {
    m_tempVarIds.clear();

    /* If the line doesn't have any characters in it, don't bother
     * parsing it
     */
    if (line.length() == 0) {
        return;
    }

    m_start = 0;

    // Get five arguments
    for (size_t i = 0; i < 5; ) {
        // First three arguments are delimited by a space
        if (i == 0) {
            m_delimiter = " ";
        }
        else if (i == 1) {
            m_delimiter = ", ";
        }
        else if (i == 3) {
            m_delimiter = "\"";
        }

        if (m_start == std::string::npos) {
            return;
        }

        // If not at the beginning of the string, advance to the next delimiter
        if (i > 0) {
            m_start = line.find_first_of(m_delimiter + " ", m_start);
        }

        if (m_start == std::string::npos) {
            return;
        }

        // Find next argument
        m_start = line.find_first_not_of(m_delimiter + " ", m_start);

        if (m_start == std::string::npos) {
            return;
        }

        // Find end of next argument
        m_end = line.find_first_of(m_delimiter, m_start);

        // Get current argument
        if (i == 0) {
            // lastType vars are updated when the column is found
            m_currentType = line.substr(m_start, m_end - m_start);
        }
        else if (i == 1) {
            /* Add the next variable ID to a storage vector for adding to the
             * element later
             */
            m_tempVarIds.push_back(line.substr(m_start, m_end - m_start));
        }
        else if (i == 2) {
            m_column = line.substr(m_start, m_end - m_start);
        }
        else if (i == 3) {
            std::string tempStr = line.substr(m_start, m_end - m_start);

            // Convert std::string to std::wstring
            m_startText.assign(tempStr.begin(), tempStr.end());
        }
        else if (i == 4) {
            std::string tempStr = line.substr(m_start, m_end - m_start);

            // Convert std::string to std::wstring
            m_updateText.assign(tempStr.begin(), tempStr.end());
        }

        // Move start past current argument
        m_start = m_end;

        // If there are more IDs to add, skip incrementing the field counter
        if (!(i == 1 && line[m_start] == ',')) {
            i++;
        }
    }

    // Replace all unicode escape characters in the string with their unicode equivalents
    m_startText = replaceUnicodeChars(m_startText);

    NetUpdate* netPtr = nullptr;
    QWidget* qPtr = nullptr;

    // Create element
    if (m_currentType == "TEXT") {
        Text* temp =  new Text(UIFont::getInstance().segoeUI14(), m_startText,
                               QColor(0, 0, 0),
                               true);
        netPtr = temp;
        qPtr = temp;
    }
    else if (m_currentType == "STATUSLIGHT") {
        StatusLight* temp = new StatusLight(m_startText,
                                            true);
        netPtr = temp;
        qPtr = temp;
    }
    else if (m_currentType == "PBAR") {
        ProgressBar* temp = new ProgressBar(m_startText,
                                            QColor(0, 120, 0),
                                            QColor(255, 255, 255),
                                            QColor(50, 50, 50), true);
        netPtr = temp;
        qPtr = temp;
    }

    /* Set update text and add the update variables to track if an element was
     * created
     */
    if (netPtr != nullptr) {
        netPtr->setUpdateText(m_updateText);
        netPtr->updateKeys(m_tempVarIds);
    }

    // Add widget to correct layout if it was created
    if (qPtr != nullptr) {
        if (m_column == "left") {
            m_leftLayout->addWidget(qPtr);
        }
        else if (m_column == "right") {
            m_rightLayout->addWidget(qPtr);
        }
    }
}

void MainWindow::resetAllTemps() {
    m_substring.clear();
    m_start = 0;

    m_delimiter.clear();

    m_currentType.clear();
    m_column.clear();
    m_startText.clear();
    m_updateText.clear();
    m_tempVarIds.clear();
}

