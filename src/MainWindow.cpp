// Copyright (c) 2012-2020 FRC Team 3512. All Rights Reserved.

#include "MainWindow.hpp"

#include <chrono>
#include <cstring>
#include <fstream>
#include <utility>

#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QtWidgets>

#include "MJPEG/MjpegClient.hpp"
#include "MJPEG/VideoStream.hpp"
#include "NetWidgets/ProgressBar.hpp"
#include "NetWidgets/StatusLight.hpp"
#include "NetWidgets/Text.hpp"
#include "Util.hpp"

std::wstring replaceUnicodeChars(std::wstring text) {
    size_t uPos = 0;

    /* Replace all "\uXXXX" strings with the unicode character corresponding
     * to the 32 bit code XXXX
     */
    while (uPos < text.length()) {
        if (uPos == 0) {
            uPos = text.find(L"\\u", uPos);
        } else {
            uPos = text.find(L"\\u", uPos + 1);
        }

        if (uPos < text.length() - 5) {
            wchar_t num[2];
            num[0] = std::stoi(text.substr(uPos + 2, 4), nullptr, 16);
            num[1] = '\0';

            text = text.replace(uPos, 6, num);
        }
    }

    return text;
}

MainWindow::MainWindow(int width, int height) : m_buffer(0xffff - 28) {
    setMinimumSize(width, height);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    m_settings = std::make_unique<Settings>("IPSettings.txt");

    m_client = new MjpegClient(m_settings->getString("streamHost"),
                               m_settings->getInt("mjpegPort"),
                               m_settings->getString("mjpegRequestPath"));

    constexpr int32_t videoX = 640;
    constexpr int32_t videoY = 480;

    m_stream = new VideoStream(
        m_client, this, videoX, videoY, &m_streamCallback, [this] {},
        [this] { m_button->setText("Stop Stream"); },
        [this] { m_button->setText("Start Stream"); });
    m_stream->setMaximumSize(videoX, videoY);

    m_button = new QPushButton("Start Stream");
    connect(m_button, SIGNAL(released()), this, SLOT(toggleButton()));

    m_leftWidgetLayout = new QVBoxLayout();

    m_centerWidgetLayout = new QVBoxLayout();

    m_centerWidgetLayout->addWidget(m_stream, 0,
                                    Qt::AlignHCenter | Qt::AlignTop);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_button, 0, Qt::AlignTop);
    buttonLayout->insertStretch(1);
    m_centerWidgetLayout->addLayout(buttonLayout, Qt::AlignHCenter);

    auto rightLayout = new QVBoxLayout();

    m_rightWidgetLayout = new QVBoxLayout();

    m_autoSelect = new QComboBox();
    connect(m_autoSelect,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            [this](int index) {
                char data[16] = "autonSelect\r\n";
                data[13] = index;

                m_dataSocket->writeDatagram(data, sizeof(data), m_remoteIP,
                                            m_dataPort);
            });
    rightLayout->addWidget(m_autoSelect);
    rightLayout->addLayout(m_rightWidgetLayout);

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->setColumnMinimumWidth(0, (width - videoX / 2) / 3);
    mainLayout->setColumnMinimumWidth(1, videoX);
    mainLayout->setColumnMinimumWidth(2, (width - videoX / 2) / 3);

    mainLayout->addLayout(m_leftWidgetLayout, 0, 0);
    mainLayout->setAlignment(m_leftWidgetLayout, Qt::AlignTop);

    mainLayout->addLayout(m_centerWidgetLayout, 0, 1);

    mainLayout->addLayout(rightLayout, 0, 2);
    mainLayout->setAlignment(rightLayout, Qt::AlignTop);

    centralWidget->setLayout(mainLayout);

    createActions();
    createMenus();

    setUnifiedTitleAndToolBarOnMac(true);

    m_dataSocket = std::make_unique<QUdpSocket>(this);
    m_dataSocket->bind(m_settings->getInt("dsDataPort"));
    connect(m_dataSocket.get(), SIGNAL(readyRead()), this,
            SLOT(handleSocketData()));

    m_remoteIP = QHostAddress{
        QString::fromUtf8(m_settings->getString("robotIP").c_str())};
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

void MainWindow::startMJPEG() { m_client->start(); }

void MainWindow::stopMJPEG() { m_client->stop(); }

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
    } else {
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

        /* If this instance has connected to the server before, receiving any
         * packet resets the timeout. This check is necessary in case a previous
         * instance caused packets to be redirected here.
         */
        if (m_connectedBefore) {
            m_connectTimer->start(2000);
        }

        if (header == "display\r\n") {
            /* Only allow keep-alive (resetting timer) if we have a valid
             * GUI; we need to connect and create the GUI before accepting
             * display data
             */
            if (m_connectedBefore) {
                updateGuiTable(m_buffer, packetPos);
                NetWidget::updateElements();
            }
        } else if (header == "guiCreate\r\n") {
            reloadGUI(m_buffer, packetPos);

            if (!m_connectedBefore) {
                m_connectedBefore = true;
            }
        } else if (header == "autonList\r\n") {
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
                autoNames.emplace_back(autoName);
            }

            m_autoSelect->clear();
            for (auto& str : autoNames) {
                m_autoSelect->addItem(str.c_str());
            }
        } else if (header == "autonConfirmed\r\n") {
            /* If a new autonomous mode was selected from the robot, it
             * sends back this packet as confirmation
             */
            std::string autoName = "Autonomous mode changed to\n";

            std::string tempName;
            packetToVar(m_buffer, packetPos, tempName);
            autoName += tempName;

            int idx = m_autoSelect->findText(QString::fromStdString(tempName));
            if (idx != -1) {
                m_autoSelect->setCurrentIndex(idx);
            }

            QMessageBox* connectDlg = new QMessageBox(this);
            connectDlg->setAttribute(Qt::WA_DeleteOnClose);
            connectDlg->setWindowTitle("Autonomous Change");
            connectDlg->setText(autoName.c_str());
            connectDlg->open();
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
    clearLayout(m_leftWidgetLayout);
    clearLayout(m_rightWidgetLayout);
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
    NetWidget::updateValues(data, pos);
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
    for (size_t i = 0; i < 5;) {
        // First three arguments are delimited by a space
        if (i == 0) {
            m_delimiter = " ";
        } else if (i == 1) {
            m_delimiter = ", ";
        } else if (i == 3) {
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
        } else if (i == 1) {
            /* Add the next variable ID to a storage vector for adding to the
             * element later
             */
            m_tempVarIds.push_back(line.substr(m_start, m_end - m_start));
        } else if (i == 2) {
            m_column = line.substr(m_start, m_end - m_start);
        } else if (i == 3) {
            std::string tempStr = line.substr(m_start, m_end - m_start);

            // Convert std::string to std::wstring
            m_startText.assign(tempStr.begin(), tempStr.end());
        } else if (i == 4) {
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

    /* Replace all unicode escape characters in the string with their unicode
     * equivalents
     */
    m_startText = replaceUnicodeChars(m_startText);
    m_updateText = replaceUnicodeChars(m_updateText);

    NetWidget* netPtr = nullptr;
    QWidget* qPtr = nullptr;

    // Create element
    if (m_currentType == "TEXT") {
        auto temp = new Text(true);
        temp->setString(m_startText);

        netPtr = temp;
        qPtr = temp;
    } else if (m_currentType == "STATUSLIGHT") {
        auto temp = new StatusLight(true);
        temp->setString(m_startText);

        netPtr = temp;
        qPtr = temp;
    } else if (m_currentType == "PBAR") {
        auto temp = new ProgressBar(true);
        temp->setString(m_startText);

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
            m_leftWidgetLayout->addWidget(qPtr);
        } else if (m_column == "right") {
            m_rightWidgetLayout->addWidget(qPtr);
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
