#ifndef PLAYBUTTON_H_
#define PLAYBUTTON_H_

#include <QWidget>
#include <QString>
#include <QRegion>
#include <QMouseEvent>
#include <QEvent>
#include <kdebug.h>
#include <QSize>
#include "kscdwidget.h"
#include "ihmnamespace.h"
#include <QSize>
using namespace IHM;

class PlayButton:public KscdWidget
{
	Q_OBJECT
public:
	PlayButton(QWidget * parent=0, QString sName="play");
	virtual ~PlayButton();
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
signals:
	void clicked(QString);

};

#endif /*PLAYBUTTON_H_*/
