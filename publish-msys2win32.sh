#!/bin/bash

PROJECT=DriverStationDisplay
DATE=`date +"%Y%m%d"`

# Build project
rm -rf build
mkdir -p build && cd build
qmake ..
make -j$(nproc)
cd ..

# Create destination folder
rm -rf $PROJECT-$DATE $PROJECT-$DATE.zip
mkdir -p $PROJECT-$DATE/platforms

# Copy files into folder
cp build/$PROJECT.exe /mingw64/bin/{Qt5Core,Qt5Gui,Qt5PrintSupport,Qt5Network,Qt5Widgets,libbz2-1,libfreetype-6,libgcc_s_seh-1,libglib-2.0-0,libgraphite2,libharfbuzz-0,libiconv-2,libintl-8,libjpeg-8,libpcre-1,libpcre2-16-0,libpng16-16,libstdc++-6,libwinpthread-1,zlib1}.dll IPSettings.txt $PROJECT-$DATE
cp /mingw64/share/qt5/plugins/platforms/qwindows.dll $PROJECT-$DATE/platforms
strip $PROJECT-$DATE/$PROJECT.exe

# Create archive
zip -r $PROJECT-$DATE.zip $PROJECT-$DATE
