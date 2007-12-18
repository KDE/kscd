#include "playbutton.h"

PlayButton::PlayButton(QWidget * parent,QString sName):KscdWidget(sName,parent)
{
	m_region = new QRegion(x(),y(),x()+width(),y()+height(),QRegion::Ellipse);
	show();
}

PlayButton::~PlayButton()
{
}

void PlayButton :: mousePressEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
	{
		event->accept();
		m_state = Pressed;
		if(m_name== "play")
		{
			emit(clicked("play"));
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

void PlayButton :: mouseReleaseEvent(QMouseEvent *event)
{
	if(m_region->contains(event->pos()))
	{
		event->accept();
		m_state = Released;
		if(m_name=="play")
		{
			m_name = "pause";
			emit(changePicture(m_path + m_name + "_n.svg"));
		}
		else
		{
			m_name = "play";
			emit(changePicture(m_path + m_name + "_n.svg"));
		}
	}
}

void PlayButton :: enterEvent (QEvent * event )
{
	event->accept();
	m_state = Focused;
	emit(changePicture(m_path + m_name + "_o.svg"));
	setToolTip(m_name);
}
void PlayButton :: leaveEvent (QEvent * event )
{
	event->accept();
	m_state = Default;
	emit(changePicture(m_path + m_name + "_n.svg"));
}
