#include "ejectbutton.h"

EjectButton::EjectButton(QWidget * parent,QString sName):KscdWidget(sName,parent)
{
	m_region = new QRegion(x(),y(),x() + width(),y()+height(),QRegion::Ellipse);
}

EjectButton::~EjectButton()
{
}

void EjectButton :: mousePressEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
		{
			event->accept();
			m_state = Pressed;
			emit(changePicture(m_path + m_name + "_p.svg"));
		}
		else
		{
			event->ignore();
		}
}

void EjectButton :: mouseReleaseEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
		{
			event->accept();
			m_state = Released;
			emit(changePicture(m_path + m_name + "_n.svg"));
		}
		else
		{
			event->ignore();
		}
}

void EjectButton :: enterEvent (QEvent * event )
{
	event->accept();
	m_state = Focused;
	emit(changePicture(m_path + m_name + "_o.svg"));
	setToolTip(m_name);
}
void EjectButton :: leaveEvent (QEvent * event )
{
	event->accept();
	m_state = Default;
	emit(changePicture(m_path + m_name + "_n.svg"));
}
