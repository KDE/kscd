#include "titlePopUp.h"

TitlePopUp::TitlePopUp( QWidget *parent ) : QWidget( parent, Qt::FramelessWindowHint )
{
	m_layout = new QGridLayout;
	
//	resize(296, 64);
	resize(300,64);

	kDebug() << "size : " << size();

	move(600,400);
//	setAutoFillBackground(false);
	m_lengthLbl = new QLabel(this);
//	m_lengthLbl->setGeometry(QRect(20, 30, 241, 22));
	m_titleLbl = new QLabel(this);
//	m_titleLbl->setGeometry(QRect(20, 10, 201, 22));

	m_layout->addWidget(m_titleLbl, 0, 0, Qt::AlignVCenter);
	m_layout->addWidget(m_lengthLbl, 1, 0, Qt::AlignVCenter);


	setLayout(m_layout);
	raise();

}

TitlePopUp::~TitlePopUp()
{
	delete m_layout;
	delete m_lengthLbl;
	delete m_titleLbl;
}

/**
* show a popup containning curent track title an his length
*/
void TitlePopUp::showTitlePopUp(QString trackTitle, QString trackLength)
{
	kDebug() << "!!!!!!!!!!!!!!!!!!allalalala!!!!!!!!!!!" ;
	m_lengthLbl->setText(trackLength);
	m_titleLbl->setText(trackTitle);
	show();
}
