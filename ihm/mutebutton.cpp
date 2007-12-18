#include "mutebutton.h"

MuteButton::MuteButton(QWidget * parent,QString sName):KscdWidget(sName,parent)
{
	m_region = new QRegion(x(),y(),x()+width(),y()+height(),QRegion::Ellipse);
	show();
}

MuteButton::~MuteButton()
{
}

void MuteButton :: mousePressEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
	{
		event->accept();
		m_state = Pressed;
		if(m_name== "mute")
		{
			emit(clicked("mute"));
			emit(changePicture(m_path + m_name + "_p.svg"));
		}
		else
		{
			emit(changePicture(m_path + m_name + "_p.svg"));
		}
	}
	else
	{
		event->ignore();
	}
}

void MuteButton :: mouseReleaseEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
	{
		event->accept();
		m_state = Released;
		if(m_name=="mute")
		{
			m_name = "unmute";
			emit(changePicture(m_path + m_name + "_n.svg"));
		}
		else
		{
			m_name = "mute";
			emit(changePicture(m_path + m_name + "_n.svg"));
		}
	}
}

void MuteButton :: enterEvent (QEvent * event )
{
	event->accept();
	m_state = Focused;
	emit(changePicture(m_path + m_name + "_o.svg"));
	setToolTip(m_name);
}

void MuteButton :: leaveEvent (QEvent * event )
{
	event->accept();
	m_state = Default;
	emit(changePicture(m_path + m_name + "_n.svg"));
}

