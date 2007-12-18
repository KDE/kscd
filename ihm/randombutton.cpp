#include "randombutton.h"

RandomButton::RandomButton(QWidget * parent,QString sName):KscdWidget(sName,parent)
{
	m_region = new QRegion(x(),y(),x() + width(),y()+height(),QRegion::Ellipse);
}

RandomButton::~RandomButton()
{
}

void RandomButton :: mousePressEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
	{
		event->accept();
		emit(changePicture(m_path + m_name + "_p.svg"));
	}
	else
	{
		event->ignore();
	}
}

void RandomButton :: mouseReleaseEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
	{
		event->accept();
		if(m_state == Default)
		{
			m_state = Embedded;
			emit(changePicture(m_path + m_name + "_p.svg"));
		}
		else if(m_state == Embedded)
		{
			m_state = Default;
			emit(changePicture(m_path + m_name + "_n.svg"));
		}
	}
	else
	{
		event->ignore();
	}
}

void RandomButton :: enterEvent (QEvent * event )
{
	if(m_state == Embedded)
	{
		event->ignore();
	}
	else
	{
		event->accept();
		m_state = Default;
		emit(changePicture(m_path + m_name + "_o.svg"));
		setToolTip(m_name);
	}
}

void RandomButton :: leaveEvent (QEvent * event )
{
	if(m_state == Embedded)
	{
		event->ignore();
	}
	else
	{
		event->accept();
		m_state = Default;
		emit(changePicture(m_path + m_name + "_n.svg"));
	}
}
