#ifndef KSCDWINDOW_H_
#define KSCDWINDOW_H_

#include <QWidget>
#include <QGridLayout>
#include <QLayout>
#include <QSlider>
#include <QLineEdit>

#include "kscdwidget.h"
#include "stopbutton.h"
#include "playbutton.h"
#include "nextbutton.h"
#include "previousbutton.h"
#include "ejectbutton.h"
#include "mutebutton.h"
#include "randombutton.h"
#include "loopbutton.h"
#include "tracklistbutton.h"

class KscdWindow:public QWidget
{
	Q_OBJECT

private:
	QGridLayout *m_layout;
	KscdWidget *m_stopB;
	KscdWidget *m_playB;
	KscdWidget *m_prevB;
	KscdWidget *m_nextB;
	KscdWidget *m_ejectB;
	KscdWidget *m_muteB;
	KscdWidget *m_randB;
	KscdWidget *m_loopB;
	KscdWidget *m_trackB;

	QString m_skinPath;
public:
	KscdWindow(QString sPath="./skin/default");
	virtual ~KscdWindow();
	QString getSkinPath();
	void setSkinPath(QString sPath);

};

#endif /*KSCDWINDOW_H_*/
