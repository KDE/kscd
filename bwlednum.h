/*
 *
 *             BW_LED_Number a very very primitive LED
 *
 * Copyright: Bernd Johannes Wuebben, wuebben@math.cornell.edu
 *
 *
 * $Id$
 *
 */


#ifndef BW_LED_NUM_H
#define BW_LED_NUM_H

#include <QFrame>
#include "qbitarray.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <QResizeEvent>


class BW_LED_Number : public QFrame
{
    Q_OBJECT

public:

    BW_LED_Number( QWidget *parent=0 );
   ~BW_LED_Number();

    void    setSmallLED(bool ); // if you LED is small it might look better
                                // if you call setSmallLED(TRUE)

            // this sets the fore and  background color of the LED
            // the forground defaults to yellow, the background defaults
            // to black

    void    setLEDColor( const QColor& foregroundColor, const QColor& backgroundColor );


            // this sets the color of the segments that are not iluminated
            // the default is a rather dark red.

    void    setLEDoffColor(QColor color);

	    // calling showOffColon(TRUE) will show the colon if not illuminated
	    // this is rather ugly -- the default is that they are not shown.

    void    showOffColon(bool off);

signals:
    void	clicked();

public slots:

            // display one of the characters " 0 1 2 3 4 5 6 7 8 9 . : - "
    void    display( char c );

            // display on e of the numbers   " 0 1 2 3 4 5 6 7 8 9"
    void    display( int i );

protected:

    void    resizeEvent( QResizeEvent * );
    void	mouseReleaseEvent ( QMouseEvent * e );
    void    paintEvent( QPaintEvent * );

private:

    bool    seg_contained_in( char c, char* seg);
    void    drawSegment( const QPoint &, char, QPainter &, int, bool = FALSE );
    void    drawSymbol( QPainter & p,char s ,bool repaint);

    char* old_segments;
    char* current_segments;

    char   current_symbol;
    char    old_symbol;
    QColor  offcolor;
    QColor  fgcolor;
    QColor  bgcolor;


    bool    smallLED;
    bool    show_off_colon;

private:	// Disabled copy constructor and operator=

    BW_LED_Number( const BW_LED_Number & );
    BW_LED_Number &operator=( const BW_LED_Number & );

};


#endif // BW_LED_NUM_H
