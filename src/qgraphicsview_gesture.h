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

#ifndef QGRAPHICSVIEW_GESTURE_H
#define QGRAPHICSVIEW_GESTURE_H

#include <QDebug>
#include <QtWidgets>

class QGraphicsViewGesture : public QGraphicsView {
  Q_OBJECT
 public:
  QGraphicsViewGesture( QWidget *parent = 0 );

  double GetZoomFactor();
  void ResetZoom();
  void SetImage( QImage input_image );
  void Zoom( const double &frac_zoom_change );

 protected:
  bool event( QEvent *event ) override;

 signals:

 public slots:

 private:
  double zoom_factor = 1;       // Scaling / zoom factor for output image.
  double zoom_factor_step = 1;  // Zoom stepping factor for zoom pinching.

  bool GestureEvent( QGestureEvent *event );
  void PinchTriggered( QPinchGesture * );
};

#endif  // QGRAPHICSVIEW_GESTURE_H
