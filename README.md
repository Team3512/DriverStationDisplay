# DriverStationDisplay

The DriverStationDisplay is a program we wrote and a protocol we invented to provide a means for us to create custom GUIs on a Driver Station dashboard while making it extremely easy to support multiple FRC robots at once.

## Dependencies

If one is building on Windows, we recommend using the [MSYS2 compiler](https://msys2.github.io/). The following libraries are required as dependencies and should be installed using either your package manager of choice or MSYS2's pacman:

* GCC 7 or newer for some C++17 features
* Qt 5

For MSYS2, use the MINGW64 terminal for building and install the following packages:

* mingw-w64-x86_64-gcc
* mingw-w64-x86_64-qt5

## Building DriverStationDisplay

To build the project, first run `qmake dir` within a terminal from the desired build directory, where "dir" is the relative location of the DriverStationDisplay.pro file. This will generate three makefiles. If a debug build is desired, run `make -f Makefile.Debug`. The default behavior when simply running `make` is to perform a release build.

To cross-compile from Linux to Windows, install the MinGW-w64 toolchain (`mingw-w64-gcc` and a MinGW-w64 build of Qt 5 (`mingw-w64-qt5-base`), then run `publish-win32.sh` to build LiveGrapher and create a .zip of the application binary and necessary files.

## Robot Setup

To use this program with a new robot, copy the DSDisplay folder in the [host folder](host) into the source tree and #include DSDisplay.hpp.

A GUISettings.txt file should be placed in `/home/lvuser` on the roboRIO. Its contents inform the DriverStationDisplay what GUI elements it should create. The format of this file will be described later.

## DriverStation Client Setup

Since this project was designed to link to libstdc++ and libgcc statically, DriverStationDisplay.exe and IPSettings.txt are the only required files on the Driver Station.

### IPSettings.txt

#### MJPEG Stream

The required entries in IPSettings.txt are as follows:

#### `streamHost`

IP address of the MJPEG stream to display in the DriverStationDisplay

#### `streamPort`

Port from which the MJPEG stream is served

#### `streamRequestPath`

Path to the MJPEG stream at the given IP address

Note: If any one of these settings is incorrect, no MJPEG stream will be displayed, but the rest of the DriverStationDisplay will work.

#### Robot-related Settings

#### `alfCmdPort`

Port to which to send ALF commands, such as "reboot" or "reload"

#### `dsDataPort`

Port on which the DriverStationDisplay receives data from the robot

#### `robotIP`

Robot's IP address

#### `robotDataPort`

Port to which to send connection packets and autonomous mode selections

###### Example IPSettings.txt

    streamHost        = 10.35.12.11
    streamPort        = 80
    streamRequestPath = /mjpg/video.mjpg

    alfCmdPort        = 3512

    #the DS binds to this
    dsDataPort        = 1130

    #the DS sends to this
    robotIP           = 10.35.12.2
    robotDataPort     = 1130

## Usage

#### Sending HUD Data

1. Call DriverStationDisplay::GetInstance() to create an instance of this class
    * The argument to GetInstance() should be the port specified as "dsDataPort" in IPSettings.txt
2. Call Clear() to empty the internal packet
    * If Clear() isn't called first, undefined behavior may result.
      (The header "display\r\n" isn't inserted when the packet isn't empty.)
3. Call AddData() as many times as necessary to add HUD data.
4. Call SendToDS() to send the data to the DriverStationDisplay.

##### Notes:
* It doesn't matter in which order the data is packed before sending the data.
* The packets are always sent to 10.35.12.42 for testing purposes.

### Autonomous Routines

The DriverStationDisplay supports selection of an autonomous mode without needing to rebuild code or reboot the roboRIO. To leverage this functionality, the appropriate autonomous functions must be made available to the DriverStationDisplay.

#### Interface

##### `void AddAutonMethod(const std::string& methodName, std::function<void()> func)`

Add the appropriate autonomous functions to the list available on the DriverStationDisplay

##### `void DeleteAllMethods()`

Erases the current list of autonomous functions stored

##### `void ExecAutonomous()`

Executes the currently selected autonomous routine

* Call this in the Autonomous() function of the Simple Robot Template

## GUISettings.txt Format

### General Format

    `[Element Name] [ID String 1],[ID String 2] [Column] ["Start Text"] ["Update Text"]`

#### `[Element Name]`

This represents the name of the GUI element to create on the display. The possible types at this time are:

1. TEXT
2. STATUSLIGHT
3. PBAR

#### `[ID String 1]`

Every GUI element declaration must have at least one ID string. This may be any combination of characters that doesn't contain a newline (\n), a carriage return (\r), or a space.

#### `[ID String 2]`

GUI elements may have more than one ID string to represent parts of the element which may be manipulated. PBAR is an example of one.

#### `[Column]`

This may be one of two values: "left" or "right" (no quotes). It determines which side on which to place the GUI element in the DriverStationDisplay's window.

Note: The first element in the column will be placed at the top of the window with the rest placed below it in descending order of declaration. The DriverStationDisplay handles vertical spacing between the different elements automatically as they stack up.

#### `["Start Text"]`

This will be the text intially used by the GUI element from its creation until it receives new data, at which point the "Replacement Text" will be used instead.

#### `["Replacement Text"]`

After new data has arrived for the GUI element, this string will be used to update the element's text. When updating, all instances of `%s` will be replaced with the received data as a string. All other instances of `%` are ignored.

### Unicode Literals

The start text and replacement text both support Unicode character insertions while still only transferring an ASCII text file. If one puts a "\uXXXX" in either "Start Text" or "Replacement Text", where XXXX is the UTF-16 identifier of the character, it will be converted to its Unicode equivalent before being displayed. It's especially useful for displaying symbols like the degrees sign and others which would otherwise be unavailable.

###### Example:

    Gyro: 0\u00b0

    becomes

    Gyro: 0º

Warning: The identifiers must belong to UTF-16 because Windows wide strings use UTF-16.

## Type Formats

#### `TEXT [ID String] [Column] ["Initial Text"] ["Replacement Text"]`

This element displays a line of text.

#### `STATUSLIGHT [ID String] [Column] ["Initial Text"] ["Replacement Text"]`

This element displays a line of text next to an indicator light. The indicator will be green, yellow, or red depending on the value sent in association with "ID String".

###### Color Codes

    Green   0
    Yellow  1
    Red     2

Warning: "Replacement Text" isn't used with this type.

#### `PBAR [Value ID],[Bar Fill ID] [Column] ["Initial Text"] ["Replacement Text"]`

This element displays a progress bar with a line of text below it.
* [Value ID] refers to the value which replaces %s in "Initial Text" and "Replacement Text"
* [Bar Fill ID] refers to the value which determines the percentage from 0 to 100

###### GUISettings.txt from https://github.com/Team3512/Robot-2013

    TEXT MODE left "Mode: Unknown" "Mode: %s"
    TEXT GYRO_VAL left "Gyro: 0\u00b0" "Gyro: %s\u00b0"
    STATUSLIGHT GYRO_ON left "Gyro Enabled" "Gyro Enabled"
    STATUSLIGHT ROTATE left "Slow Rotation" "Slow Rotation"

    PBAR RPM_MAN_DISP,RPM_MAN right "Manual: 0%" "Manual: %s%"
    PBAR RPM_SET_DISP,RPM_SET right "RPM \u2192 0" "RPM \u2192 %s"
    PBAR RPM_REAL_DISP,RPM_REAL right "RPM: 0" "RPM: %s"
    STATUSLIGHT SHOOT_READY right "Shooter Ready" "Shooter Ready"
    STATUSLIGHT SHOOT_ON right "Shooter On" "Shooter On"
    STATUSLIGHT SHOOT_MAN right "Shooter Manual" "Shooter Manual"
    STATUSLIGHT ARMS_DOWN right "Arms Down" "Arms Down"
