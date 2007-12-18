#include "kscdwindow.h"

KscdWindow::KscdWindow(QString skinPath):QWidget()
{
 	//setFixedSize ( 650,200 );
 	m_layout = new QGridLayout;
	
	m_stopB = new StopButton(this);
	m_playB = new PlayButton(this);
	m_nextB = new NextButton(this);
	m_prevB = new PreviousButton(this);
	m_ejectB = new EjectButton(this);
	m_muteB = new MuteButton(this);
	m_randB = new RandomButton(this);
	m_loopB = new LoopButton(this);
	m_trackB = new TrackListButton(this);

 	m_layout->addWidget(m_ejectB, 0, 1);
 	m_layout->addWidget(m_prevB, 1, 0);
 	m_layout->addWidget(m_playB, 1, 1);
 	m_layout->addWidget(m_nextB, 1, 2);
 	m_layout->addWidget(m_stopB, 2, 1);
	m_layout->addWidget(m_randB, 3, 0,Qt::AlignCenter);
	m_layout->addWidget(m_loopB, 3, 2,Qt::AlignCenter);
	m_layout->addWidget(m_muteB, 3, 1,Qt::AlignCenter);
	m_layout->addWidget(m_trackB, 3, 3,Qt::AlignCenter);
 	setLayout(m_layout);

	show();
}

KscdWindow::~KscdWindow()
{
	delete m_playB;
	delete m_stopB;
	delete m_nextB;
	delete m_prevB;
	delete m_ejectB;
	delete m_muteB;
	delete m_layout;
}

QString KscdWindow::getSkinPath()
{
	return m_skinPath;
}
void KscdWindow::setSkinPath(QString sPath)
{
	m_skinPath = sPath;
}
