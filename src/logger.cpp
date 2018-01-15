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

// Reproduced from https://wiki.qt.io/Simple-logger with modification.

#include "logger.h"

Logger::Logger( QObject *parent, QString log_directory ) : QObject( parent ) {
  QString logfilename = log_directory + "/tikz_previewer_log.txt";

  file = new QFile;
  file->setFileName( logfilename );
  file->open( QIODevice::WriteOnly | QIODevice::Text );

  this->write( tr( "Created log file." ) );
}

void Logger::write( const QString &value,
                    const MessageCategory &message_category ) {
  QString text =
    QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss " );
  switch ( message_category ) {
    case MessageCategory::INFO:
      text = text + tr( "INFO    " );
      break;
    case MessageCategory::WARNING:
      text = text + tr( "WARNING " );
      break;
    case MessageCategory::ERROR:
      text = text + tr( "ERROR   " );
      break;
    default:
      text = text + tr( "INFO    " );
      break;
  }

  text = text + value + "\n";

  QTextStream out( file );
  out.setCodec( "UTF-8" );
  if ( file != 0 ) {
    out << text;
  }
}

Logger::~Logger() {
  if ( file != 0 ) file->close();
}
