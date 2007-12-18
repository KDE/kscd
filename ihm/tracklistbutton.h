#ifndef TRACKLISTBUTTON_H_
#define TRACKLISTBUTTON_H_

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

class TrackListButton:public KscdWidget
{
	Q_OBJECT
public:
	TrackListButton(QWidget * parent=0, QString sName="tracklist");
	virtual ~TrackListButton();
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);

signals:
	void clicked(QString);

};

#endif /*TRACKLISTBUTTON_H_*/
