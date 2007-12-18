#include "kscdwidget.h"

KscdWidget::KscdWidget(QString sName,QWidget * parent):QSvgWidget(parent)
{
	m_state = Default;
	m_name = sName;

	m_path = "./skin/default/" ;
	
	connect(this,SIGNAL(changePicture(QString)),this,SLOT(load(QString)));
	setMouseTracking ( true );
	emit(changePicture(m_path+sName+"_n.svg"));
	show();
}

KscdWidget::~KscdWidget()
{
}

QString KscdWidget::getName()
{
	return m_name;
}
void KscdWidget::setName(QString sName)
{
	m_name = sName;
}
