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

#include "qgraphicsview_gesture.h"
#include <iostream>

QGraphicsViewGesture::QGraphicsViewGesture( QWidget *parent )
  : QGraphicsView( parent ) {
  setAttribute( Qt::WA_AcceptTouchEvents );
  grabGesture( Qt::PinchGesture );
}

//
// Public
//

double QGraphicsViewGesture::GetZoomFactor() { return zoom_factor; }

// Reset the zoom factor to unity while fitting the scene within the viewport
// with some margin.
void QGraphicsViewGesture::ResetZoom() {
  zoom_factor = 1.0;
  return;
}

void QGraphicsViewGesture::Zoom( const double &frac_zoom_change ) {
  if ( this->isEnabled() ) {
    zoom_factor = zoom_factor * frac_zoom_change;
    this->scale( frac_zoom_change, frac_zoom_change );
  }
  return;
}

//
// Protected
//

bool QGraphicsViewGesture::event( QEvent *event ) {
  if ( event->type() == QEvent::Gesture ) {
    return GestureEvent( static_cast<QGestureEvent *>( event ) );
  }
  return QGraphicsView::event( event );
}

//
// Private
//

// Capture gesture and identify whether it was a "pinch".
bool QGraphicsViewGesture::GestureEvent( QGestureEvent *event ) {
  if ( QGesture *pinch = event->gesture( Qt::PinchGesture ) )
    PinchTriggered( static_cast<QPinchGesture *>( pinch ) );
  return true;
}

// Act upon pinching by adjusting zoom level.  TODO: also need to deal
// with pixmap position to avoid flickering.
void QGraphicsViewGesture::PinchTriggered( QPinchGesture *gesture ) {
  // Q_ASSERT( this->pixmap() );

  QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
  if ( changeFlags & QPinchGesture::ScaleFactorChanged && this->isEnabled() ) {
    zoom_factor_step = gesture->totalScaleFactor();
    // Ensure that the user doesn't blow out the zoom.
    if ( zoom_factor * zoom_factor_step > 0.25 &&
         zoom_factor * zoom_factor_step < 4.0 ) {
      zoom_factor *= zoom_factor_step;
      this->scale( zoom_factor_step, zoom_factor_step );
    }
  }
  // Update overall zoom factor.
  if ( gesture->state() == Qt::GestureFinished ) {
    zoom_factor_step = 1;
  }

  update();
}
