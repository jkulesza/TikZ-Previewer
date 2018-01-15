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

#include "tikz_previewer.h"
#include "ui_tikz_previewer.h"

#include <math.h>
#include <iostream>
#include <vector>

TikzPreviewer::TikzPreviewer( QWidget *parent )
  : QMainWindow( parent ),
    config_directory(
      QStandardPaths::standardLocations( QStandardPaths::AppDataLocation )
        .first() ),
    working_directory( config_directory + "/working_dir" ),
    ui( new Ui::TikzPreviewer ) {
  ui->setupUi( this );

  // Initialize resources.
  Q_INIT_RESOURCE( icons );

  // Restore previous window settings (chiefly, size & position).
  QSettings settings;
  restoreGeometry( settings.value( "mainWindowGeometry" ).toByteArray() );
  restoreState( settings.value( "mainWindowState" ).toByteArray() );

  if ( !FirstRun() ) {
    std::exit( 1 );
  }

  logger = new Logger( this, config_directory );
  CreateMenus();
  logger->write( tr( "Created menus and menu entries." ) );
  InitializeWidgets();
  logger->write( tr( "Initialized widgets." ) );

  // Uncomment to force loading a file by default.
  // LoadFile( "/Users/jkulesza/test.tikz.tex" );

  return;
}

TikzPreviewer::~TikzPreviewer() {
  delete preferences;
  delete logger;
  delete ui;
}

//
// Public
//

// Load a TikZ LaTeX file directly, without a dialog.
bool TikzPreviewer::LoadFile( const QString &infilename ) {
  fileinfo_input = QFileInfo( infilename );

  // Assume that the absolute filepath is given and, thus, the file should exist
  // (e.g., corresponding to a call from ActionOpenFile). If not, try
  // pre-pending the current directory which woudl likely occur if called from
  // the command line.
  if ( !fileinfo_input.exists() ) {
    // Assume only the file name was given with the current path assumed so the
    // path needs to be constructed.
    QDir openfilepath = QDir::currentPath();
    fileinfo_input = openfilepath.absolutePath() + "/" + infilename;
  }

  // Confirm that the path is valid and readable.  If not, log error and exit.
  if ( !fileinfo_input.exists() || !fileinfo_input.isReadable() ) {
    msg = tr( "Cannot find specified input file: \"%1\", exiting." )
            .arg( fileinfo_input.absoluteFilePath() );
    logger->write( msg, Logger::MessageCategory::ERROR );
    QMessageBox::critical( this, QGuiApplication::applicationDisplayName(),
                           msg );
    return false;
  }

  QString new_last_path = fileinfo_input.absoluteDir().path();
  preferences->SetLastPath( new_last_path );
  msg = tr( "Set new last path as \"%1\"." ).arg( new_last_path );
  logger->write( msg );

  msg = tr( "Opened \"%1\"." ).arg( fileinfo_input.absoluteFilePath() );
  logger->write( msg );
  ui->status_bar->showMessage( msg );
  ui->status_bar->repaint();

  // Setup filesystem watcher to see if/when source file changes to trigger
  // refreshes.
  filesystem_watcher = new QFileSystemWatcher( this );
  filesystem_watcher->addPath( fileinfo_input.absoluteFilePath() );
  connect( filesystem_watcher, SIGNAL( fileChanged( QString ) ), this,
           SLOT( SourceFileChanged() ) );

  // Perform initial render of image.
  ExecuteCopyAndRender();

  // Scale by user-specified adjustment factor.
  double s = preferences->GetZoomScalingFactor();
  ui->gv_output_image->scale( s, s );
  ui->gv_output_image->ResetZoom();

  return true;
}

//
// Private Slots
//

// Provide basic information about the application.
void TikzPreviewer::AboutDialog() {
  QString translatedTextAboutQtCaption;
  translatedTextAboutQtCaption = QMessageBox::tr( "<h3>About " ) + APP_NAME +
                                 QMessageBox::tr( ", version " ) + APP_VERSION +
                                 "</h3>";
  QString translatedTextAboutQtText;
  translatedTextAboutQtText =
    QMessageBox::tr(
      "<p>TikZ Previewer is a utility for rendering and (pre)viewing TikZ "
      "graphics.</p>"
      "<p>Copyright (C) %1 %2.</p>"
      "<p>This program is free software: you can redistribute it and/or "
      "modify "
      "it under the terms of the GNU General Public License as published "
      "by "
      "the Free Software Foundation, either version 3 of the License, or "
      "(at "
      "your option) any later version.</p>"
      "<p>This program is distributed in the hope that it will be useful, "
      "but "
      "WITHOUT ANY WARRANTY; without even the implied warranty of "
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
      "General Public License for more details.</p>"
      "<p>You should have received a copy of the GNU General Public "
      "License "
      "along with this program.  If not, see <a "
      "href=\"https://www.gnu.org/licenses/\">https://www.gnu.org/licenses/"
      "</"
      "a>.</p>"
      "<p>See <a href=\"https://%3/\">%3</a> for more information.</p>" )
      .arg( QStringLiteral( APP_COPYRIGHT_YEARS ),
            QStringLiteral( APP_CONTRIBUTORS ), QStringLiteral( APP_URL ) );

  QMessageBox *msgBox =
    new QMessageBox( this );  // To appear consistent with aboutQt().
  msgBox->setWindowTitle( tr( "About TikZ Previewer" ) );
  msgBox->setText( translatedTextAboutQtCaption );
  msgBox->setInformativeText( translatedTextAboutQtText );

  QPixmap pm( ":icons/icon_app_256.ico" );
  if ( !pm.isNull() )
    msgBox->setIconPixmap( pm.scaledToWidth( 64, Qt::SmoothTransformation ) );
  msgBox->exec();
  return;
}

// Get the location of the TikZ LaTeX file to operate on with a standard file
// open dialog.
void TikzPreviewer::ActionOpenFile() {
  // Attempt to open from the last directory that a file was opened from.  If
  // that directory doesn't exist, then try to use the user's home location.  If
  // that doesn't work, then use the current working directory (likely, the
  // exectuable's dirctory).
  QDir openfilepath = preferences->GetLastPath();
  logger->write(
    tr( "Last path used: \"%1\"." ).arg( openfilepath.absolutePath() ) );
  if ( !openfilepath.exists() ) {
    const QStringList home_locations =
      QStandardPaths::standardLocations( QStandardPaths::HomeLocation );
    const QString home_location =
      home_locations.isEmpty() ? QDir::currentPath() : home_locations.last();
    openfilepath = home_location;
  }

  logger->write(
    tr( "Using open path: \"%1\"." ).arg( openfilepath.absolutePath() ) );

  // Present open file dialog.
  QString infilename = QFileDialog::getOpenFileName(
    this, tr( "Open TikZ LaTeX File" ), openfilepath.absolutePath(),
    tr( "TikZ LaTeX Files (*.tex);;All Files (*)" ) );

  // Handle instances where a file is opened or the dialog is canceled.
  if ( infilename != "" ) {
    if ( !LoadFile( infilename ) ) {
      exit( 1 );
    }
  } else {
    // Open file dialog canceled.
  }

  return;
}

// Save PDF to the original files location, with the original file's prefix.
void TikzPreviewer::ActionSavePdf() {
  QString working_pdffilepath =
    working_directory + "/" + working_filename_prefix + ".pdf";
  QString input_pdffilepath = fileinfo_input.absoluteDir().absolutePath() +
                              "/" + fileinfo_input.fileName() + ".pdf";

  if ( QFile::exists( input_pdffilepath ) ) {
    QFile::remove( input_pdffilepath );
  }

  // Copy PDF file.
  QFile::copy( working_pdffilepath, input_pdffilepath );
  msg = tr( "Wrote \"%1\"." ).arg( input_pdffilepath );
  logger->write( msg );
  ui->status_bar->showMessage( msg );
  ui->status_bar->repaint();

  return;
}

bool TikzPreviewer::ActionSaveImage() {
  // Determine supported image filetypes.
  QStringList mimeTypeFilters;
  const QByteArrayList supportedMimeTypes = QImageWriter::supportedMimeTypes();
  foreach ( const QByteArray &mimeTypeName, supportedMimeTypes )
    mimeTypeFilters.append( mimeTypeName );
  mimeTypeFilters.sort();

  // Construct save file dialog and set options.
  QFileDialog dialog;
  dialog.setAcceptMode( QFileDialog::AcceptSave );
  dialog.setWindowTitle( "Save TikZ Image As..." );
  dialog.setDirectory( fileinfo_input.dir() );
  dialog.setMimeTypeFilters( mimeTypeFilters );
  dialog.selectMimeTypeFilter( "image/png" );
  dialog.setDefaultSuffix( "png" );

  // Execute dialog and ensure that the user selected to save the file.
  if ( dialog.exec() ) {
    QString outfilepath = dialog.selectedFiles().first();
    if ( !graphics_scene_pixmap->pixmap().save( outfilepath ) ) {
      QMessageBox::information(
        this, QGuiApplication::applicationDisplayName(),
        tr( "Cannot write \"%1\"." )
          .arg( QDir::toNativeSeparators( outfilepath ) ) );
      return false;
    }
    msg = tr( "Wrote \"%1\"." ).arg( QDir::toNativeSeparators( outfilepath ) );
    ui->status_bar->showMessage( msg );
    ui->status_bar->repaint();
    return true;
  } else {
    return false;
  }
  return false;
}

void TikzPreviewer::ActionPrint() {
  QPixmap pm( graphics_scene_pixmap->pixmap() );
  QPrintDialog dialog( &printer, this );
  if ( dialog.exec() ) {
    QPainter painter( &printer );
    QRect rect = painter.viewport();
    QSize size = pm.size();
    size.scale( rect.size(), Qt::KeepAspectRatio );
    painter.setViewport( rect.x(), rect.y(), size.width(), size.height() );
    painter.setWindow( pm.rect() );
    painter.drawPixmap( 0, 0, pm );
    msg = tr( "Drew pixmap and sent to printer." );
    logger->write( msg );
    ui->status_bar->showMessage( msg );
    ui->status_bar->repaint();
  }
  return;
}

void TikzPreviewer::ActionClose() {
  graphics_scene_pixmap->hide();
  ui->gv_output_image->fitInView( graphics_scene_pixmap, Qt::KeepAspectRatio );
  ui->gv_output_image->ResetZoom();
  ui->gv_output_image->setStyleSheet( gv_style_wait );
  ui->gv_output_image->setEnabled( false );
  ToggleWidgets( false );
  msg = tr( "Closed image." );
  logger->write( msg );
  ui->status_bar->showMessage( msg );
  ui->status_bar->repaint();
  return;
}

void TikzPreviewer::ActionQuit() {
  this->close();
  return;
}

// Copy image to system clipboard.
void TikzPreviewer::ActionCopyPixmap() {
#ifndef QT_NO_CLIPBOARD
  QApplication::clipboard()->setPixmap( graphics_scene_pixmap->pixmap() );
  msg = tr( "Copied pixmap to clipboard." );
  logger->write( msg );
  ui->status_bar->showMessage( msg );
  ui->status_bar->repaint();
#endif  // !QT_NO_CLIPBOARD
  return;
}

void TikzPreviewer::ActionRefresh() {
  ExecuteCopyAndRender();
  return;
}

void TikzPreviewer::ActionZoomActual() {
  double s = 1 / ui->gv_output_image->GetZoomFactor();
  ui->gv_output_image->scale( s, s );
  ui->gv_output_image->ResetZoom();
  act_zoom_out->setEnabled( true );
  act_zoom_in->setEnabled( true );
  return;
}

void TikzPreviewer::ActionZoomFit() {
  ui->gv_output_image->fitInView( graphics_scene_pixmap, Qt::KeepAspectRatio );
  ui->gv_output_image->ResetZoom();
  act_zoom_out->setEnabled( true );
  act_zoom_in->setEnabled( true );
  return;
}

void TikzPreviewer::ActionZoomIn() {
  ui->gv_output_image->Zoom( 1.25 );
  if ( ui->gv_output_image->GetZoomFactor() > 4 )
    act_zoom_in->setEnabled( false );
  if ( ui->gv_output_image->GetZoomFactor() > 0.25 )
    act_zoom_out->setEnabled( true );
  return;
}

void TikzPreviewer::ActionZoomOut() {
  ui->gv_output_image->Zoom( 0.80 );
  if ( ui->gv_output_image->GetZoomFactor() < 0.25 )
    act_zoom_out->setEnabled( false );
  if ( ui->gv_output_image->GetZoomFactor() < 4 )
    act_zoom_in->setEnabled( true );
  return;
}

void TikzPreviewer::ActionAutoRecenterView() {
  if ( act_auto_recenter_view->isChecked() ) {
    preferences->SetAutoRecenterView( true );
  } else {
    preferences->SetAutoRecenterView( false );
  }
  return;
}

void TikzPreviewer::ShowImageContextMenu( const QPoint &pos ) {
  QPoint global_pos = ui->gv_output_image->mapToGlobal( pos );

  QMenu menu;

  menu.addAction( tr( "&Quick Save PDF" ), this,
                  &TikzPreviewer::ActionSavePdf );

  menu.addAction( tr( "&Save Image As..." ), this,
                  &TikzPreviewer::ActionSaveImage );

  menu.addAction( tr( "&Print Image..." ), this, &TikzPreviewer::ActionPrint );
  menu.addSeparator();

  menu.addAction( tr( "&Copy Image" ), this, &TikzPreviewer::ActionCopyPixmap );
  menu.addSeparator();
  menu.addAction( tr( "Zoom &In (+25%)" ), this, &TikzPreviewer::ActionZoomIn );

  menu.addAction( tr( "Zoom &Out (-25%)" ), this,
                  &TikzPreviewer::ActionZoomOut );

  menu.addAction( tr( "&Actual Size" ), this, &TikzPreviewer::ActionZoomFit );

  menu.exec( global_pos );

  return;
}

void TikzPreviewer::SourceFileChanged() {
  // Change to a transient waiting color.  However, this is not done because
  // most renders happen so fast a flicker is observed that seems out of place.
  // ui->label_output_image->setStyleSheet( label_style_wait );
  ExecuteCopyAndRender();
  return;
}

void TikzPreviewer::CaptureStderr() {
  QByteArray byteArray = proc->readAllStandardError();
  QStringList strLines = QString( byteArray ).split( "\n" );
  bool errors_present = false;
  foreach ( QString line, strLines ) {
    errors_present = true;
    ui->textbox_tex_info_code->append( line );
  }
  if ( errors_present ) {
    ui->frame_tex_info_code->setHidden( false );
  }

  return;
}

void TikzPreviewer::CaptureStdout() {
  QByteArray byteArray = proc->readAllStandardOutput();
  QStringList strLines = QString( byteArray ).split( "\n" );
  foreach ( QString line, strLines ) {
    FindLatexErrors( line );
    ui->textbox_tex_info_log->append( line );
    // check_latex_errors( line );
  }
  return;
}

//
// Private
//

// Redefine closeEvent so that close (with the "X" button) or quit saves the
// window position to be used when executed next time.
void TikzPreviewer::closeEvent( QCloseEvent *event ) {
  QSettings settings;
  settings.setValue( "mainWindowGeometry", saveGeometry() );
  settings.setValue( "mainWindowState", saveState() );
  event->accept();
  return;
}

bool TikzPreviewer::ConvertPdfToPixmap() {
  QString pdf_filename =
    working_directory + "/" + working_filename_prefix + ".pdf";

  logger->write( "Loading PDF: " + pdf_filename + "." );
  Poppler::Document *document = Poppler::Document::load( pdf_filename );

  // Ignore the paper color to render to a transparent background rather than
  // white, by default.
  document->setRenderHint( Poppler::Document::IgnorePaperColor );

  // Attempt to improve appearance of render.
  document->setRenderHint( Poppler::Document::Antialiasing );
  document->setRenderHint( Poppler::Document::ThinLineShape );

  if ( !document || document->isLocked() ) {
    msg = tr( "Could not load PDF document." );
    logger->write( msg, Logger::MessageCategory::ERROR );
    QMessageBox::critical( this, QGuiApplication::applicationDisplayName(),
                           msg );
    delete document;
    return false;
  }

  // Safety check.
  if ( document == 0 ) {
    msg = tr( "PDF document doesn't exist." );
    logger->write( msg, Logger::MessageCategory::ERROR );
    QMessageBox::critical( this, QGuiApplication::applicationDisplayName(),
                           msg );
    return false;
  }

  // Access page of the (zero-indexed) PDF file.  It is expected that for a
  // standalone TikZ render there will only be one page.
  const int pageNumber = 0;
  Poppler::Page *pdfPage = document->page( pageNumber );
  if ( pdfPage == 0 ) {
    msg = tr( "Could not load PDF page." );
    logger->write( msg, Logger::MessageCategory::ERROR );
    QMessageBox::critical( this, QGuiApplication::applicationDisplayName(),
                           msg );
    return false;
  }

  logger->write( "Rendering first page of PDF to image." );

  QImage qi = pdfPage->renderToImage( 1000, 1000 );

  if ( qi.isNull() ) {
    msg = tr( "Could not render image." );
    logger->write( msg, Logger::MessageCategory::ERROR );
    QMessageBox::critical( this, QGuiApplication::applicationDisplayName(),
                           msg );
    return false;

  } else {
    QRectF old_rect = graphics_scene_pixmap->boundingRect();
    graphics_scene_pixmap->setPixmap( QPixmap::fromImage( qi ) );
    QRectF new_rect = graphics_scene_pixmap->boundingRect();
    ui->gv_output_image->setStyleSheet( gv_style_okay );
    ui->gv_output_image->setEnabled( true );

    // Recenter view if the size of the render changed.
    if ( new_rect != old_rect ) {
      // If the user wants, recenter to the center of the new bounding
      // rectangle.
      if ( preferences->GetAutoRecenterView() ) {
        double xmid = graphics_scene_pixmap->boundingRect().x() +
                      graphics_scene_pixmap->boundingRect().width() / 2;
        double ymid = graphics_scene_pixmap->boundingRect().y() +
                      graphics_scene_pixmap->boundingRect().height() / 2;

        ui->gv_output_image->setSceneRect(
          graphics_scene_pixmap->boundingRect() );
        ui->gv_output_image->centerOn( xmid, ymid );
      }
    }
    graphics_scene_pixmap->show();
    logger->write( "Successfully rendered image; displaying." );
  }
  return true;
}

bool TikzPreviewer::CopyTexFile() {
  QFile file( fileinfo_input.absoluteFilePath() );

  fileinfo_working =
    QFileInfo( working_directory + "/" + working_filename_prefix + ".tex" );

  // Cleanup working directory.
  msg = tr( "Cleaning up working directory." );
  ui->status_bar->showMessage( msg );
  ui->status_bar->repaint();
  QDir dir( working_directory );
  dir.setNameFilters( QStringList() << "*.*" );
  dir.setFilter( QDir::Files );
  foreach ( QString dirFile, dir.entryList() ) { dir.remove( dirFile ); }

  // Copy in new file.
  QFile::copy( fileinfo_input.absoluteFilePath(),
               fileinfo_working.absoluteFilePath() );
  msg = tr( "Copied \"%1\" to \"%2\"." )
          .arg( fileinfo_input.absoluteFilePath(),
                fileinfo_working.absoluteFilePath() );
  logger->write( msg );
  ui->status_bar->showMessage( msg );
  ui->status_bar->repaint();

  return true;
}

void TikzPreviewer::CreateMenus() {
  std::vector<QMenu *> menus;

  // File menu.
  QMenu *file_menu = ui->menu_bar->addMenu( tr( "&File" ) );
  menus.push_back( file_menu );

  act_open = file_menu->addAction( tr( "&Open..." ), this,
                                   &TikzPreviewer::ActionOpenFile );
  act_open->setShortcut( QKeySequence::Open );

  file_menu->addSeparator();

  act_save_pdf = file_menu->addAction( tr( "&Quick Save PDF" ), this,
                                       &TikzPreviewer::ActionSavePdf );
  act_save_pdf->setShortcut( QKeySequence::Save );

  act_save_image = file_menu->addAction( tr( "&Save Image As..." ), this,
                                         &TikzPreviewer::ActionSaveImage );
  act_save_image->setShortcut( QKeySequence::SaveAs );

  act_print = file_menu->addAction( tr( "&Print Image..." ), this,
                                    &TikzPreviewer::ActionPrint );
  act_print->setShortcut( QKeySequence::Print );

  file_menu->addSeparator();

  act_close =
    file_menu->addAction( tr( "&Close" ), this, &TikzPreviewer::ActionClose );
  act_close->setShortcut( QKeySequence::Close );

  QAction *act_exit =
    file_menu->addAction( tr( "E&xit" ), this, &TikzPreviewer::ActionQuit );
  act_exit->setShortcut( QKeySequence::Quit );

#ifndef QT_NO_CLIPBOARD
  // Edit menu.
  // The zero-width non-joiner character is added to prevent macOS from adding
  // its own menu entries automatically.  See
  // https://github.com/electron/electron/issues/3038.
  QMenu *edit_menu = ui->menu_bar->addMenu( tr( "&Edit\u200C" ) );
  menus.push_back( edit_menu );

  act_copy_pixmap = edit_menu->addAction( tr( "&Copy Image" ), this,
                                          &TikzPreviewer::ActionCopyPixmap );
  act_copy_pixmap->setShortcut( QKeySequence::Copy );
#endif  // !QT_NO_CLIPBOARD

  // View menu.
  // The zero-width non-joiner character is added to prevent macOS from adding
  // its own menu entries automatically.  See
  // https://github.com/electron/electron/issues/3038.
  QMenu *view_menu = ui->menu_bar->addMenu( tr( "&View\u200C" ) );
  menus.push_back( view_menu );

  act_refresh = view_menu->addAction( tr( "&Refresh Image" ), this,
                                      &TikzPreviewer::ActionRefresh );
  act_refresh->setShortcut( QKeySequence::Refresh );
  act_refresh->setEnabled( false );

  view_menu->addSeparator();

  act_zoom_actual = view_menu->addAction( tr( "&Actual Size" ), this,
                                          &TikzPreviewer::ActionZoomActual );
  act_zoom_actual->setShortcut( tr( "Ctrl+0" ) );

  act_zoom_fit = view_menu->addAction( tr( "&Zoom Fit to Window" ), this,
                                       &TikzPreviewer::ActionZoomFit );
  act_zoom_fit->setShortcut( tr( "Ctrl+9" ) );
  act_zoom_fit->setVisible( false );

  act_zoom_in = view_menu->addAction( tr( "Zoom &In (+25%)" ), this,
                                      &TikzPreviewer::ActionZoomIn );
  act_zoom_in->setShortcut( QKeySequence::ZoomIn );

  act_zoom_out = view_menu->addAction( tr( "Zoom &Out (-25%)" ), this,
                                       &TikzPreviewer::ActionZoomOut );
  act_zoom_out->setShortcut( QKeySequence::ZoomOut );

  view_menu->addSeparator();

  act_auto_recenter_view = view_menu->addAction(
    tr( "Auto Recenter View" ), this, &TikzPreviewer::ActionAutoRecenterView );
  act_auto_recenter_view->setCheckable( true );
  if ( preferences->GetAutoRecenterView() )
    act_auto_recenter_view->setChecked( true );
  act_auto_recenter_view->setToolTip(
    tr( "Toggle whether to recenter the view of the size of the overall image "
        "changes, regardless of zoom level." ) );

  // Help menu.
  QMenu *help_menu = ui->menu_bar->addMenu( tr( "&Help" ) );
  menus.push_back( help_menu );

  help_menu->addAction( tr( "&About" ), this, &TikzPreviewer::AboutDialog );
  help_menu->addAction( tr( "About &Qt" ), &QApplication::aboutQt );

  // Setup context menu on the output graphics view.
  ui->gv_output_image->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( ui->gv_output_image,
           SIGNAL( customContextMenuRequested( const QPoint & ) ), this,
           SLOT( ShowImageContextMenu( const QPoint & ) ) );

  // Configure behavior for all menus.
  for ( auto &m : menus ) {
    m->setToolTipsVisible( true );
  }

  return;
}

bool TikzPreviewer::ExecuteCopyAndRender() {
  // Hide TeX code frame with the assumption that the render will be successful.
  ui->textbox_tex_info_log->clear();
  ui->textbox_tex_info_code->clear();
  ui->frame_tex_info_code->setHidden( true );
  error_line_numbers.clear();

  if ( !CopyTexFile() ) {
    logger->write( "Failed to copy TeX file." );
    return false;
  }
  if ( !RenderTexFile() ) {
    logger->write( "Failed to render TeX file." );
    return false;
  }
  if ( !ConvertPdfToPixmap() ) {
    logger->write( "Failed to convert PDF to Image." );
    return false;
  }
  ToggleWidgets( true );
  return true;
}

// Check the LaTeX standard output for an error message, identifying which line
// the error is on.
void TikzPreviewer::FindLatexErrors( QString &line ) {
  QRegularExpression re( "l\\.(\\d+)" );
  QRegularExpressionMatch match = re.match( line );
  if ( match.hasMatch() ) {
    error_line_numbers.insert( match.captured( 1 ).toInt() );
    line = "<font color=\"#ff0000\">" + line + "</font>";
  }
  return;
}

// Execute to setup environment.
bool TikzPreviewer::FirstRun() {
  // Check to see if configuration file and log directory exists; if not, create
  // it.
  QString config_filename =
    config_directory + "/tikz_previewer_preferences.xml";
  QFileInfo config_file( config_filename );
  if ( !QDir( config_directory ).exists() ) {
    // Attempt to make configuration directory because it doesn't exist.  If it
    // fails to be made, then return false and, eventually, exit the program.
    if ( !QDir().mkdir( config_directory ) ) {
      std::cout
        << tr( "Failed to make configuration directory.\n" ).toStdString();
      return false;
    }
    // Create default configuration file.
    preferences = new TikzPreviewerPreferences( config_filename.toStdString() );
    preferences->SavePreferences();
  } else {
    preferences = new TikzPreviewerPreferences( config_filename.toStdString() );
    if ( config_file.exists() ) {
      preferences->ReadPreferences();
    } else {
      preferences->SavePreferences();
    }
  }
  if ( !QDir( working_directory ).exists() ) {
    if ( !QDir().mkdir( working_directory ) ) {
      std::cout << tr( "Failed to make working directory.\n" ).toStdString();
      return false;
    }
  }

  return true;
}

// Set menu entries and form widgets assuming the program has just opened.
void TikzPreviewer::InitializeWidgets() {
  // Set log font.
  QFont font( "Courier" );
  font.setStyleHint( QFont::Monospace );
  font.setPointSize( 12 );
  ui->textbox_tex_info_log->setFont( font );
  ui->textbox_tex_info_code->setFont( font );

  // Disable menu entries, disable graphics view, and hide TeX code frame at
  // startup.
  ToggleWidgets( false );
  ui->gv_output_image->setEnabled( false );
  ui->frame_tex_info_code->setHidden( true );

  // Setup graphics view.
  graphics_scene = new QGraphicsScene();
  graphics_scene_pixmap = new QGraphicsPixmapItem();
  graphics_scene->addItem( graphics_scene_pixmap );
  ui->gv_output_image->setScene( graphics_scene );

  msg = tr(
    "Welcome to TikZ Previewer!  To begin: click \"File → Open...\" and "
    "select a "
    "standalone TikZ LaTeX input file to preview." );
  logger->write( msg );
  ui->status_bar->showMessage( msg );
  ui->status_bar->repaint();

  return;
}

void TikzPreviewer::ToggleWidgets( const bool &widget_state_enabled ) {
  // Configure menu entries.
  act_save_pdf->setEnabled( widget_state_enabled );
  act_save_image->setEnabled( widget_state_enabled );
  act_print->setEnabled( widget_state_enabled );
  act_close->setEnabled( widget_state_enabled );

  act_copy_pixmap->setEnabled( widget_state_enabled );

  act_refresh->setEnabled( widget_state_enabled );
  act_zoom_in->setEnabled( widget_state_enabled );
  act_zoom_out->setEnabled( widget_state_enabled );
  act_zoom_fit->setEnabled( widget_state_enabled );

  return;
}

// Execute process to render TikZ LaTeX file to PDF.
bool TikzPreviewer::RenderTexFile() {
  // Check to see if the user-specified LaTeX executable exists.
  QFileInfo fi( preferences->GetLatexExecutable() );
  if ( !fi.exists() ) {
    msg = tr( "The executable specified at \"%1\" doesn't appear to exist." )
            .arg( preferences->GetLatexExecutable() );
    logger->write( msg, Logger::MessageCategory::ERROR );
    QMessageBox::critical( this, QGuiApplication::applicationDisplayName(),
                           msg );
    return false;
  }

  QString exec_cmd = preferences->GetLatexExecutable() +
                     " -interaction=nonstopmode " + working_filename_prefix +
                     ".tex";

  logger->write( "Running " + exec_cmd + "." );
  ui->status_bar->showMessage( tr( "Performing LaTeX render." ) );
  ui->status_bar->repaint();

  proc = new QProcess( this );
  proc->setWorkingDirectory( working_directory );

  connect( proc, SIGNAL( readyReadStandardOutput() ), this,
           SLOT( CaptureStdout() ) );
  connect( proc, SIGNAL( readyReadStandardError() ), this,
           SLOT( CaptureStderr() ) );

  proc->start( exec_cmd );
  proc->waitForFinished();

  delete proc;

  // Because the code frame is hidden, we know there was not an error to be
  // displayed (i.e., rendering was successful).
  if ( error_line_numbers.size() == 0 ) {
    // ui->tab_widget->setCurrentIndex( 0 );
    logger->write( "Finished " + exec_cmd + "." );
    ui->status_bar->showMessage( tr( "Done rendering image." ) );
    ui->status_bar->repaint();
    return true;
  } else {
    // Perform UI updates.
    ui->frame_tex_info_code->setHidden( false );
    ui->gv_output_image->setStyleSheet( gv_style_error );
    // ui->gv_output_image->setEnabled( false );

    ToggleWidgets( false );
    // ui->tab_widget->setCurrentIndex( 1 );
    ui->status_bar->showMessage(
      tr( "Failed to render image.  Please check that the input is valid.  "
          "Error possibly near line " ) +
      QString( "%0" ).arg( *error_line_numbers.begin() ) + "." );
    ui->status_bar->repaint();
    logger->write( "Failed to render image." );

    // Output input file with error lines highlighted.  This is sloppy and
    // wasteful, but the file sizes are (reasonably) expected to be small so the
    // overhead isn't significant.
    QFile infile( fileinfo_input.absoluteFilePath() );

    // First read file to determine number of lines.
    std::vector<QString> infile_lines;
    int total_lines = 0;
    if ( infile.open( QIODevice::ReadOnly ) ) {
      QTextStream in( &infile );
      while ( !in.atEnd() ) {
        ++total_lines;
        QString line = in.readLine();
        infile_lines.push_back( line );
      }
      infile.close();
    }

    // Determine how much to pad line numbers.
    int pad_width = int( ceil( log10( total_lines ) ) ) + 2;

    // Append lines to tex code window.
    for ( auto it = infile_lines.begin(); it != infile_lines.end(); ++it ) {
      int line_num = std::distance( infile_lines.begin(), it ) + 1;
      auto search = error_line_numbers.find( line_num );
      QString line_color;
      if ( search != error_line_numbers.end() ) {
        line_color = "#ff0000";
        logger->write( "LaTeX / TikZ error on input file line " +
                       QString( "%0" ).arg( line_num ) + "." );
      } else {
        line_color = "#000000";
      }

      QString file_line =
        QString( "%0" ).arg( line_num, pad_width, 10, QChar( QChar::Nbsp ) ) +
        " | <font color=\"" + line_color + "\">" + *it + "</font>";
      ui->textbox_tex_info_code->append( file_line );
    }
    return false;
  }
}
