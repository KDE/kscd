#ifndef KSCDWIDGET_H_
#define KSCDWIDGET_H_

#include <QSvgWidget>
#include <QWidget>
#include <QRegion>
#include <QMouseEvent>
#include <QString>
#include <kdebug.h>
#include "ihmnamespace.h"

using namespace IHM;

class KscdWidget:public QSvgWidget
{
	Q_OBJECT
protected:
	QRegion *m_region;
	StateButton m_state;
	QString m_name;
	QString m_path;

public:
	KscdWidget(QString sName,QWidget * parent=0);
	virtual ~KscdWidget();
	QString getName();
	void setName(QString);
signals:
	void changePicture(QString);

};

#endif /*KSCDWIDGET_H_*/
