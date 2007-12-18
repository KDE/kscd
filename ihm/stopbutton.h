#ifndef STOPBUTTON_H_
#define STOPBUTTON_H_

#include <QWidget>
#include <QString>
#include <QRegion>
#include <QMouseEvent>
#include <QEvent>
#include <QString>

#include <kdebug.h>
#include "kscdwidget.h"
#include "ihmnamespace.h"

using namespace IHM;

class StopButton:public KscdWidget
{
	Q_OBJECT
public:
	StopButton(QWidget * parent=0, QString sName="stop");
	virtual ~StopButton();
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);

signals:
	void clicked(QString);

};

#endif /*STOPBUTTON_H_*/
