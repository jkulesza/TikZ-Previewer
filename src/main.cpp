// clang-format off
/*

TikZ Previewer is a utility for rendering and (pre)viewing TikZ graphics.

Copyright (C) 2018 Joel A. Kulesza and other contributors.

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <https://www.gnu.org/licenses/>.

Report bugs and download new versions at <https://github.com/jkulesza/TikZ-Previewer/>

*/
// clang-format on

#include <QApplication>
#include <QCommandLineParser>

#include "tikz_previewer.h"

int main( int argc, char *argv[] ) {
  QApplication a( argc, argv );

  a.setApplicationName( APP_NAME );
  a.setApplicationVersion( APP_VERSION );
  QCoreApplication::setApplicationName( APP_NAME );

  QCommandLineParser commandLineParser;
  commandLineParser.addHelpOption();
  commandLineParser.addPositionalArgument(
    TikzPreviewer::tr( "[file]" ),
    TikzPreviewer::tr( "TikZ LaTeX .tex source file to open." ) );
  commandLineParser.process( QCoreApplication::arguments() );

  TikzPreviewer w;
  w.setWindowTitle( APP_NAME );

  if ( !commandLineParser.positionalArguments().isEmpty() &&
       !w.LoadFile( commandLineParser.positionalArguments().front() ) ) {
    return -1;
  }

  // Support pinch-to-zoom gesture (panning already supported by the scroll area
  // around the image label).
  //  QList<Qt::GestureType> gestures;
  //  gestures << Qt::PinchGesture;
  //  w.GetGestures(gestures);

  w.show();

  return a.exec();
}
