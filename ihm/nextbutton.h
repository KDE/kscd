#ifndef NEXTBUTTON_H_
#define NEXTBUTTON_H_

#include <QWidget>
#include <QString>
#include <QRegion>
#include <QMouseEvent>
#include <QEvent>
#include <kdebug.h>
#include "kscdwidget.h"
#include "ihmnamespace.h"

using namespace IHM;

class NextButton:public KscdWidget
{
	Q_OBJECT
public:
	NextButton(QWidget * parent=0, QString sName="next");
	virtual ~NextButton();
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
signals:
	void clicked(QString);

};

#endif /*NEXTBUTTON_H_*/
