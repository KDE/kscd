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

void HWControler_test::testEject()
{
	QCOMPARE(1, 1);
}
/*
void HWControler_test :: selectSpeaker_test()
{
}

void HWControler_test :: eject_test()
{
	
}

void HWControler_test :: play_test()
{
}

void HWControler_test :: nextTrack_test()
{
	
}

void HWControler_test :: prevTrack_test()
{

}

void HWControler_test :: stop_test()
{

}

void HWControler_test :: pause_test()
{

}

void HWControler_test :: mute_test()
{

}

void HWControler_test :: isCdInserted_test()
{

}*/

QTEST_KDEMAIN_CORE(HWControler_test)
