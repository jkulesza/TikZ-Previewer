#!/bin/bash

# Determine build OS
platform='unknown'
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
   platform='darwin'
elif [[ "$unamestr" == 'Linux' ]]; then
   platform='linux'
fi

# Perform dynamic library setup.
if [[ $platform == 'linux' ]]; then

    mkdir TikZ\ Previewer.app/Contents/Frameworks

    cp -R /local/brew/opt/poppler/lib                     TikZ\ Previewer.app/Contents/Frameworks/libpoppler
    cp -R /local/brew/opt/qt/lib/QtCore.framework         TikZ\ Previewer.app/Contents/Frameworks
    cp -R /local/brew/opt/qt/lib/QtGui.framework          TikZ\ Previewer.app/Contents/Frameworks
    cp -R /local/brew/opt/qt/lib/QtPrintSupport.framework TikZ\ Previewer.app/Contents/Frameworks
    cp -R /local/brew/opt/qt/lib/QtWidgets.framework      TikZ\ Previewer.app/Contents/Frameworks

    # Fixups for libpoppler.

    chmod 744 TikZ\ Previewer.app/Contents/Frameworks/libpoppler/libpoppler.73.dylib
    chmod 744 TikZ\ Previewer.app/Contents/Frameworks/libpoppler/libpoppler.73.0.0.dylib
    chmod 744 TikZ\ Previewer.app/Contents/Frameworks/libpoppler/libpoppler-qt5.1.dylib
    chmod 744 TikZ\ Previewer.app/Contents/Frameworks/libpoppler/libpoppler-qt5.1.12.0.dylib

    install_name_tool -id @executable_path/../Frameworks/libpoppler/libpoppler-qt5.1.dylib      TikZ\ Previewer.app/Contents/Frameworks/libpoppler/libpoppler-qt5.1.dylib
    install_name_tool -id @executable_path/../Frameworks/libpoppler/libpoppler-qt5.1.12.0.dylib TikZ\ Previewer.app/Contents/Frameworks/libpoppler/libpoppler-qt5.1.12.0.dylib

    install_name_tool -change /local/brew/opt/poppler/lib/libpoppler-qt5.1.dylib                @executable_path/../Frameworks/libpoppler/libpoppler-qt5.1.dylib      TikZ\ Previewer.app/Contents/MacOS/TikZ\ Previewer
    install_name_tool -change /local/brew/opt/poppler/lib/libpoppler-qt5.1.12.0.dylib           @executable_path/../Frameworks/libpoppler/libpoppler-qt5.1.12.0.dylib TikZ\ Previewer.app/Contents/MacOS/TikZ\ Previewer

    # Recursive corrections.
    install_name_tool -change @rpath/libpoppler.73.dylib                                        @executable_path/../Frameworks/libpoppler/libpoppler.73.dylib         TikZ\ Previewer.app/Contents/Frameworks/libpoppler/libpoppler-qt5.1.dylib
    install_name_tool -change @rpath/libpoppler.73.0.0.dylib                                    @executable_path/../Frameworks/libpoppler/libpoppler.73.0.0.dylib     TikZ\ Previewer.app/Contents/Frameworks/libpoppler/libpoppler-qt5.1.12.0.dylib

    # Fixups for Qt (unnecessary in testing, may be necessary later).

    # chmod 744 TikZ\ Previewer.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
    # chmod 744 TikZ\ Previewer.app/Contents/Frameworks/QtGui.framework/Versions/5/QtGui
    # chmod 744 TikZ\ Previewer.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport
    # chmod 744 TikZ\ Previewer.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets

    # Setup linkages for execuable.

    # install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore            TikZ\ Previewer.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
    # install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui              TikZ\ Previewer.app/Contents/Frameworks/QtGui.framework/Versions/5/QtGui
    # install_name_tool -id @executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtWidgets TikZ\ Previewer.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport
    # install_name_tool -id @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets      TikZ\ Previewer.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets

    # install_name_tool -change /local/brew/opt/qt/lib/QtCore.framework/Versions/5/QtCore                 @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore                 TikZ\ Previewer.app/Contents/MacOS/TikZ\ Previewer
    # install_name_tool -change /local/brew/opt/qt/lib/QtGui.framework/Versions/5/QtGui                   @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui                   TikZ\ Previewer.app/Contents/MacOS/TikZ\ Previewer
    # install_name_tool -change /local/brew/opt/qt/lib/QtPrintSupport.framework/Versions/5/QtPrintSupport @executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport TikZ\ Previewer.app/Contents/MacOS/TikZ\ Previewer
    # install_name_tool -change /local/brew/opt/qt/lib/QtWidgets.framework/Versions/5/QtWidgets           @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets           TikZ\ Previewer.app/Contents/MacOS/TikZ\ Previewer

elif [[ $platform == 'linux' ]]; then

  echo "No dynamic library support yet setup for Linux, stopping."

fi

