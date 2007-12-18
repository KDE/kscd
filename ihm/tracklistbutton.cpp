#include "tracklistbutton.h"

TrackListButton::TrackListButton(QWidget * parent,QString sName):KscdWidget(sName,parent)
{
	m_region = new QRegion(x(),y(),x()+width(),y()+height(),QRegion::Ellipse);
	show();
}

TrackListButton::~TrackListButton()
{
}

void TrackListButton :: mousePressEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
	{
		event->accept();
		m_state = Pressed;
		if(m_name== "mute")
		{
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

void TrackListButton :: mouseReleaseEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
	{
		event->accept();
		m_state = Released;
		emit(changePicture(m_path + m_name + "_n.svg"));
		emit(changePicture(m_path + m_name + "_n.svg"));
	}
}

void TrackListButton :: enterEvent (QEvent * event )
{
	event->accept();
	m_state = Focused;
	emit(changePicture(m_path + m_name + "_o.svg"));
	setToolTip(m_name);
}

void TrackListButton :: leaveEvent (QEvent * event )
{
	event->accept();
	m_state = Default;
	emit(changePicture(m_path + m_name + "_n.svg"));
}
