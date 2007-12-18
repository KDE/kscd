#ifndef MUTEBUTTON_H_
#define MUTEBUTTON_H_

#include <QWidget>
#include <QString>
#include <QRegion>
#include <QMouseEvent>
#include <QEvent>
#include <kdebug.h>
#include "kscdwidget.h"
#include "ihmnamespace.h"
#include <QString>
using namespace IHM;

class MuteButton:public KscdWidget
{
	Q_OBJECT
public:
	MuteButton(QWidget * parent=0, QString sName="mute");
	virtual ~MuteButton();
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
signals:
	void clicked(QString);

};

#endif /*MUTEBUTTON_H_*/
