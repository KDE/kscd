/////////////////////////////////////////////////////////////////////
//
//  Example Program for BW_LED_Number
//
//  Copyright: Bernd Johannes Wuebben, wuebben@math.cornell.edu
//
//
//  $Id$
//

#include <qdialog.h>
#include <qapplication.h>
#include <qframe.h>
#include <qlabel.h>
#include "bwlednum.h"
#include <qpushbutton.h>
#include <qtooltip.h>

#include <qlcdnumber.h>

class LEDDemo : public QDialog{

   Q_OBJECT

public:

  LEDDemo( QWidget *parent=0, const char *name=0 );

private slots:
  void	timerevent();
  void buttonclicked();
  void run();
private:

  QPushButton *advButton;
  QPushButton *runButton;
  bool on;
  QTimer *timer;
  int counter ;
  void  setLEDColor( BW_LED_Number *lcd, const QColor& fgColor, const QColor& bgColor );
  BW_LED_Number     *led;
  //  QLCDNumber *led2;

};


LEDDemo::LEDDemo( QWidget *parent, const char *name )
    : QDialog( parent, name )
{

    counter = 0;
    on = false;
    setCaption( "BW_LED_Number" );

    QPushButton *quitButton = new QPushButton( this, "quitButton" );
    quitButton->setText( "Quit" );
    quitButton->setGeometry( 120,20, 60,30 );
    connect( quitButton, SIGNAL(clicked()), qApp, SLOT(quit()) );

    runButton = new QPushButton( this, "runButton" );
    runButton->setText( "Run" );
    runButton->setGeometry( 120,120, 60,30 );
    connect(runButton , SIGNAL(clicked()), this, SLOT(run()) );

    advButton = new QPushButton( this, "advanceButton" );
    advButton->setText( "Next" );
    advButton->setGeometry( 120,70, 60,30 );
    connect( advButton, SIGNAL(clicked()), this, SLOT(buttonclicked()) );

    QToolTip::add( quitButton,"Enough ..." );

    led = new BW_LED_Number(this,"led");


    //    led->setLEDColor(red,blue);
    //    led->setSmallLED(TRUE);
    
    //    QColorGroup colGrp(red, gray, black, black, black, black, black);
    /*
    QColorGroup colGrp(red, gray, red, red, red, red, white); 
    led2 = new QLCDNumber(this,"led2");

    led2->setPalette(QPalette(colGrp,colGrp,colGrp));
    led2->setSegmentStyle(QLCDNumber::Filled);
    led2->display(8);
    led2->setNumDigits(1);
    led2->setGeometry(70,30,30,70);

    */

    led->setGeometry(30,30,30,40);

    //    led->display(' ');

    led->display(0);
    //led->setLEDoffColor(blue);

    //    led->showOffColon(TRUE);

    timer = new QTimer(this,"timer");
    connect( timer, SIGNAL(timeout()), this, SLOT(timerevent()) );

    
}

void LEDDemo::buttonclicked(){

timerevent();

}


void LEDDemo::run(){

  on = !on;

  if(on){
    runButton->setText(i18n("Stop"));

    timer->start(100,FALSE);
  }
  else{

    runButton->setText(i18n("Run"));
    timer->stop();

  }
}



void LEDDemo::timerevent(){

 if (counter == 12)
   counter = 0;

  if(counter >= 9){
    if(counter +2 == 11)
      led->display(':');

    if(counter +2 == 12)
      led->display('-');

    if(counter +2 == 13)
      led->display('.');
    counter ++;
  }
  else{
    led->display(++counter );
  }
}



#include "example.moc"

int main( int argc, char **argv ) {

    QApplication a( argc, argv );
    LEDDemo  *w = new LEDDemo;
    a.setMainWidget( w );
    w->show();
    return a.exec();
}
