#include "nextbutton.h"

NextButton::NextButton(QWidget * parent,QString sName):KscdWidget(sName,parent)
{
	m_region = new QRegion(x(),y(),x() + width(),y()+height(),QRegion::Ellipse);
}

NextButton::~NextButton()
{
}

void NextButton :: mousePressEvent(QMouseEvent *event)
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

void NextButton :: mouseReleaseEvent(QMouseEvent *event)
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

void NextButton :: enterEvent (QEvent * event )
{
	event->accept();
	m_state = Focused;
	emit(changePicture(m_path + m_name + "_o.svg"));
	setToolTip(m_name);
}
void NextButton :: leaveEvent (QEvent * event )
{
	event->accept();
	m_state = Default;
	emit(changePicture(m_path + m_name + "_n.svg"));
}
