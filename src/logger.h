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

#ifndef LOGGER_H
#define LOGGER_H

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QObject>
#include <QTextStream>

#include <QStandardPaths>

class Logger : public QObject {
  Q_OBJECT
 public:
  explicit Logger( QObject *parent, QString log_directory );
  enum class MessageCategory { INFO, WARNING, ERROR };
  ~Logger();

 private:
  QFile *file;

 signals:

 public slots:
  void write( const QString &value,
              const MessageCategory &message_category = MessageCategory::INFO );
};

#endif  // LOGGER_H
