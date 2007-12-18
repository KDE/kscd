#ifndef EJECTBUTTON_H_
#define EJECTBUTTON_H_

#include <QWidget>
#include <QString>
#include <QRegion>
#include <QMouseEvent>
#include <QEvent>
#include <kdebug.h>
#include "kscdwidget.h"
#include "ihmnamespace.h"

using namespace IHM;

class EjectButton:public KscdWidget
{
	Q_OBJECT
public:
	EjectButton(QWidget * parent=0, QString sName="eject");
	virtual ~EjectButton();
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
signals:
	void clicked(QString);

};

#endif /*EJECTBUTTON_H_*/
