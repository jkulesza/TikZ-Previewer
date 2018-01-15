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

#ifndef TIKZ_PREVIEWER_PREFERENCES_H
#define TIKZ_PREVIEWER_PREFERENCES_H

#include <QDir>
#include <QRect>
#include <QStandardPaths>

#include <stdlib.h>

#include "third-party/pugixml/pugixml.hpp"

class TikzPreviewerPreferences {
 public:
  TikzPreviewerPreferences( const std::string &prefs_location );
  ~TikzPreviewerPreferences();

  void SavePreferences();
  void ReadPreferences();

  bool GetAutoRecenterView();
  void SetAutoRecenterView( const bool &do_recenter );
  QString GetLatexExecutable();
  QString GetLastPath();
  void SetLastPath( const QString &last_path );
  double GetZoomScalingFactor();

 private:
  pugi::xml_document *doc;
  std::string prefs_filename;
};

#endif  // TIKZ_PREVIEWER_PREFERENCES_H
