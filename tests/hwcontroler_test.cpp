/*
 * Kscd - A simple cd player for the KDE Project
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 * Copyright (c) 2002-2003 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
 * Copyright (c) 2003-2006 Richard Lärkäng <nouseforaname@home.se>
 *
 * --------------
 * ISI KsCD Team :
 * --------------
 * Stanislas KRZYWDA <stanislas.krzywda@gmail.com>
 * Sovanramy Var <mastasushi@gmail.com>
 * Bouchikhi Mohamed-Amine <bouchikhi.amine@gmail.com>
 * Gastellu Sylvain<sylvain.gastellu@gmail.com>
 * -----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "hwcontroler.h"
#include "audiocd.h"
#include "hwcontroler_test.h"

/*We will eject the CD and very if it is always inserted*/
void HWControler_test :: eject_test()
{
	devices = new HWControler();
	devices->eject();
	QVERIFY(! devices->getCD()->isCdInserted());
	delete(devices);


}

/*We will play the current track an check the state of phonon*/
void HWControler_test :: play_test()
{
	devices = new HWControler();
	if(devices->getCD()->isCdInserted())
	{
		devices->play();
		QCOMPARE(devices->getState(), Phonon::PlayingState);
	}
	else
	{
		QVERIFY(true);
	}
	delete(devices);
}

/*We will next the CD and verify if the remaining time has changed*/
void HWControler_test :: nextTrack_test()
{
	devices = new HWControler();
	if(devices->getCD()->isCdInserted())
	{
		devices->nextTrack();
		QCOMPARE(devices->getMedia()->remainingTime(), devices->getMedia()->currentTime());
		
	}	
	QVERIFY(true);
	delete(devices);
}

void HWControler_test :: prevTrack_test()
{
	devices = new HWControler();
	if(devices->getCD()->isCdInserted())
	{
		devices->nextTrack();
		QCOMPARE(devices->getMedia()->remainingTime(), devices->getMedia()->currentTime());
		
	}	
	QVERIFY(true);
	delete(devices);
}

void HWControler_test :: stop_test()
{
	devices = new HWControler();
	if(devices->getCD()->isCdInserted())
	{
		devices->stop();
		QCOMPARE(devices->getState(), Phonon::StoppedState);
	}
	else
	{
		QVERIFY(true);
	}
	delete(devices);
}

void HWControler_test :: pause_test()
{
	devices = new HWControler();
	if(devices->getCD()->isCdInserted())
	{
		devices->stop();
		QCOMPARE(devices->getState(), Phonon::PausedState);
	}
	else
	{
		QVERIFY(true);
	}
	delete(devices);
}

void HWControler_test :: mute_test()
{
	devices = new HWControler();
	if(devices->getCD()->isCdInserted())
	{
		devices->mute(true);
		QCOMPARE(devices->getVolume(), 0.0);
	}
	else
	{
		QVERIFY(true);
	}
	delete(devices);

}

void HWControler_test :: setVolume_test()
{
	devices = new HWControler();
	if(devices->getCD()->isCdInserted())
	{
		devices->setVolume(10.0);
		QCOMPARE(devices->getVolume(), 10.0);
	}
	else
	{
		QVERIFY(true);
	}
	delete(devices);

}

void HWControler_test ::configMedia_test()
{
	devices = new HWControler();
	if(devices->getCD()->isCdInserted())
	{
		QVERIFY(devices->isDiscValid());
	}else
	{
		QVERIFY(true);
	}
	delete(devices);
}

void HWControler_test ::replayTrack_test()
{
	devices = new HWControler();
	
	if(devices->getCD()->isCdInserted())
	{
		devices->play(1);
		devices->replayTrack(1);
		QCOMPARE(devices->getCurrentTrack(), 1);
		
	}	
	QVERIFY(true);
	delete(devices);
}

void HWControler_test ::replayDisk_test()
{
	devices = new HWControler();
	
	if(devices->getCD()->isCdInserted())
	{
		devices->replayDisk();
		QCOMPARE(devices->getCurrentTrack(), 1);
		
	}	
	QVERIFY(true);
	delete(devices);
}



QTEST_KDEMAIN_CORE(HWControler_test)
