#ifndef PREVIOUSBUTTON_H_
#define PREVIOUSBUTTON_H_

#include <QWidget>
#include <QString>
#include <QRegion>
#include <QMouseEvent>
#include <QEvent>
#include <kdebug.h>
#include "kscdwidget.h"
#include "ihmnamespace.h"

using namespace IHM;

class PreviousButton:public KscdWidget
{
	Q_OBJECT
public:
	PreviousButton(QWidget * parent=0, QString sName="previous");
	virtual ~PreviousButton();
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
signals:
	void clicked(QString);

};

#endif /*PREVIOUSBUTTON_H_*/
