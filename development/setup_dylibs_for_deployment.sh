#!/bin/bash

# Determine build OS
platform=`uname`

# Perform dynamic library setup.
if [[ $platform == 'Darwin' ]]; then

    # Homebrew

    # poppler_prefix="/local/brew/opt/poppler/lib"
    # qt_prefix="/local/brew/opt/qt/lib"
    # libpoppler_name="libpoppler.73.0.0.dylib"
    # libpoppler_qt5_name="libpoppler-qt5.1.12.0.dylib"

    # Macports

    poppler_prefix="/sw/lib"
    qt_prefix="/local/brew/opt/qt/lib"
    libpoppler_name="libpoppler.68.dylib"
    libpoppler_qt5_name="libpoppler-qt5.1.dylib"

    echo "Using prefix for poppler: $poppler_prefix"

    # Copy in dynamic libraries.
    mkdir TikZ\ Previewer.app/Contents/Frameworks
    mkdir TikZ\ Previewer.app/Contents/Frameworks/libpoppler
    cp  $poppler_prefix/libpop*         TikZ\ Previewer.app/Contents/Frameworks/libpoppler

    # Fixups for libpoppler.
    chmod 744 TikZ\ Previewer.app/Contents/Frameworks/libpoppler/$libpoppler_name
    chmod 744 TikZ\ Previewer.app/Contents/Frameworks/libpoppler/$libpoppler_qt5_name

    install_name_tool -id @executable_path/../Frameworks/libpoppler/$libpoppler_qt5_name TikZ\ Previewer.app/Contents/Frameworks/libpoppler/$libpoppler_qt5_name
    install_name_tool -change $poppler_prefix/$libpoppler_qt5_name                       @executable_path/../Frameworks/libpoppler/$libpoppler_qt5_name TikZ\ Previewer.app/Contents/MacOS/TikZ\ Previewer

    # Recursive corrections.
    install_name_tool -change @rpath/$libpoppler_name                                    @executable_path/../Frameworks/libpoppler/$libpoppler_name     TikZ\ Previewer.app/Contents/Frameworks/libpoppler/$libpoppler_qt5_name

elif [[ $platform == 'Linux' ]]; then

  echo "No dynamic library support yet setup for Linux, stopping."

else

  echo "Unrecognized platform ($platform), stopping."

fi

