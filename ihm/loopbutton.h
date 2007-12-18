#ifndef LOOPBUTTON_H_
#define LOOPBUTTON_H_

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

class LoopButton:public KscdWidget
{
	Q_OBJECT
public:
	LoopButton(QWidget * parent=0, QString sName="loop");
	virtual ~LoopButton();
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
signals:
	void clicked(QString);

};

#endif /*LOOPBUTTON_H_*/
