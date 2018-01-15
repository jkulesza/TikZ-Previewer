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

#ifndef TIKZ_PREVIEWER_H
#define TIKZ_PREVIEWER_H

#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QMainWindow>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QRegion>
#include <QSettings>
#include <QStandardPaths>
#include <QStyle>

#ifndef QT_NO_PRINTER
#include <QtPrintSupport/qtprintsupportglobal.h>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#endif

#include <algorithm>
#include <set>

#include <defined_values.h>
#include <logger.h>
#include <preferences.h>
#include <qgraphicsview_gesture.h>

#include <poppler-qt5.h>

namespace Ui {
class TikzPreviewer;
}

class TikzPreviewer : public QMainWindow {
  Q_OBJECT

 public:
  explicit TikzPreviewer( QWidget *parent = 0 );
  ~TikzPreviewer();
  bool LoadFile( const QString &infilename );

 private slots:
  // Menu actions.
  void AboutDialog();

  // File menu.
  void ActionOpenFile();
  void ActionSavePdf();
  bool ActionSaveImage();
  void ActionPrint();
  void ActionClose();
  void ActionQuit();

  // Edit menu.
  void ActionCopyPixmap();

  // View menu.
  void ActionRefresh();
  void ActionZoomActual();
  void ActionZoomFit();
  void ActionZoomIn();
  void ActionZoomOut();
  void ActionAutoRecenterView();

  void ShowImageContextMenu( const QPoint &pos );

  // Signaled slot actions.
  void SourceFileChanged();
  void CaptureStdout();
  void CaptureStderr();

 private:
  const QString config_directory;
  const QString working_directory;
  const QString working_filename_prefix = "tikz_previewer";
  const QString gv_style_okay = "QGraphicsView { background-color : #ffffff; }";
  const QString gv_style_error =
    "QGraphicsView { background-color : #ffe8e8; }";
  const QString gv_style_wait = "QGraphicsView { background-color : #ececec; }";

  Ui::TikzPreviewer *ui;
  QProcess *proc;
  QGraphicsScene *graphics_scene;
  QGraphicsPixmapItem *graphics_scene_pixmap;

  Logger *logger;
  TikzPreviewerPreferences *preferences;

  QFileSystemWatcher *filesystem_watcher;

  // Private actions to allow post-construction enable/disable.
  QAction *act_open;
  QAction *act_save_pdf;
  QAction *act_save_image;
  QAction *act_print;
  QAction *act_close;
  QAction *act_copy_pixmap;
  QAction *act_refresh;
  QAction *act_zoom_actual;
  QAction *act_zoom_fit;
  QAction *act_zoom_in;
  QAction *act_zoom_out;

  QAction *act_auto_recenter_view;

  QFileInfo fileinfo_input;    // User-specified TikZ LaTeX file.
  QFileInfo fileinfo_working;  // Working TikZ LaTeX file (for rendering).

  QString msg;  // Message for logging / UI status bar.

#ifndef QT_NO_PRINTER
  QPrinter printer;
#endif

  std::set<int> error_line_numbers;

  void closeEvent( QCloseEvent *event );

  bool ConvertPdfToPixmap();
  bool CopyTexFile();
  void CreateMenus();
  bool ExecuteCopyAndRender();
  void FindLatexErrors( QString &line );
  bool FirstRun();
  void InitializeWidgets();
  void ToggleWidgets( const bool &widget_state_enabled );
  bool RenderTexFile();
};

#endif  // TIKZ_PREVIEWER_H
