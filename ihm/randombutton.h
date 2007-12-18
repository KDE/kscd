#ifndef RANDOMBUTTON_H_
#define RANDOMBUTTON_H_

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

class RandomButton:public KscdWidget
{
	Q_OBJECT
public:
	RandomButton(QWidget * parent=0, QString sName="random");
	virtual ~RandomButton();
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
signals:
	void clicked(QString);

};

#endif /*RANDOMBUTTON_H_*/
