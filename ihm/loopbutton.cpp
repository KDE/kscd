#include "loopbutton.h"

LoopButton::LoopButton(QWidget * parent,QString sName):KscdWidget(sName,parent)
{
	m_region = new QRegion(x(),y(),x()+width(),y()+height(),QRegion::Ellipse);
	show();
}

LoopButton::~LoopButton()
{
}

void LoopButton :: mousePressEvent(QMouseEvent *event)
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

void LoopButton :: mouseReleaseEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
	{
		event->accept();
		m_state = Released;
		if(m_name == "loop")
		{
			m_name = "loopTrack";
			emit(changePicture(m_path + m_name + "_n.svg"));
		}
		else if(m_name == "loopTrack")
		{
			m_name = "loopDisc";
			emit(changePicture(m_path + m_name + "_n.svg"));
		}
		else
		{
			m_name = "loop";
			emit(changePicture(m_path + m_name + "_n.svg"));
		}
	}
}

void LoopButton :: enterEvent (QEvent * event )
{
	event->accept();
	m_state = Focused;
	emit(changePicture(m_path + m_name + "_o.svg"));
	setToolTip(m_name);
}

void LoopButton :: leaveEvent (QEvent * event )
{
	event->accept();
	m_state = Default;
	emit(changePicture(m_path + m_name + "_n.svg"));
}

