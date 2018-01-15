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

#include "preferences.h"
#include <iostream>

// Create data structure to hold preferences in hierarchical format and populate
// with default values.
TikzPreviewerPreferences::TikzPreviewerPreferences(
  const std::string &prefs_location ) {
  prefs_filename = prefs_location;
  doc = new pugi::xml_document();

  pugi::xml_node preferences = doc->append_child( "Preferences" );

  // Set a temporary LaTeX executable path.
  pugi::xml_node latexpath =
    preferences.append_child( "LaTeX_Executable_Path" );
  latexpath.append_child( pugi::node_pcdata )
    .set_value( "/Library/TeX/texbin/pdflatex" );

  // Initialize the last path that a user drew a TikZ input file from to
  // something sane.
  const QStringList home_locations =
    QStandardPaths::standardLocations( QStandardPaths::HomeLocation );
  const QString home_location =
    home_locations.isEmpty() ? QDir::currentPath() : home_locations.last();
  pugi::xml_node lastpath = preferences.append_child( "Last_Path" );
  lastpath.append_child( pugi::node_pcdata )
    .set_value( home_location.toStdString().c_str() );

  // Initialize a zoom scaling fudge factor to make "100%" appear reasonable.
  pugi::xml_node zoom_scaling_factor =
    preferences.append_child( "Zoom_Scaling_Factor" );
  zoom_scaling_factor.append_child( pugi::node_pcdata ).set_value( "0.5" );

  // Toggle whether to automatically recenter the view of the size changes.
  pugi::xml_node auto_recenter_view =
    preferences.append_child( "Auto_Recenter_View" );
  auto_recenter_view.append_child( pugi::node_pcdata ).set_value( "1" );

  return;
}

TikzPreviewerPreferences::~TikzPreviewerPreferences() { delete doc; }

// Save preferences to the stated file using two spaces to indent.
void TikzPreviewerPreferences::SavePreferences() {
  doc->save_file( prefs_filename.c_str(), "  " );
  return;
}

// Update the XML document by reading from the specified file.
void TikzPreviewerPreferences::ReadPreferences() {
  doc->load_file( prefs_filename.c_str() );
  return;
}

//
// Accessors
//

QString TikzPreviewerPreferences::GetLatexExecutable() {
  return doc->child( "Preferences" )
    .child( "LaTeX_Executable_Path" )
    .child_value();
}

// Update last path that a file was opened from and save the preferences file.
void TikzPreviewerPreferences::SetLastPath( const QString &last_path ) {
  doc->child( "Preferences" )
    .child( "Last_Path" )
    .last_child()
    .set_value( last_path.toStdString().c_str() );
  SavePreferences();
  return;
}

QString TikzPreviewerPreferences::GetLastPath() {
  return doc->child( "Preferences" ).child( "Last_Path" ).child_value();
}

double TikzPreviewerPreferences::GetZoomScalingFactor() {
  return atof(
    doc->child( "Preferences" ).child( "Zoom_Scaling_Factor" ).child_value() );
}

bool TikzPreviewerPreferences::GetAutoRecenterView() {
  if ( atoi( doc->child( "Preferences" )
               .child( "Auto_Recenter_View" )
               .child_value() ) == 1 ) {
    return true;
  } else {
    return false;
  }
}

void TikzPreviewerPreferences::SetAutoRecenterView( const bool &do_recenter ) {
  if ( do_recenter ) {
    doc->child( "Preferences" )
      .child( "Auto_Recenter_View" )
      .last_child()
      .set_value( "1" );
  } else {
    doc->child( "Preferences" )
      .child( "Auto_Recenter_View" )
      .last_child()
      .set_value( "0" );
  }
  SavePreferences();
  return;
}
