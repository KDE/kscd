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
#include "tracklistdlg.h"
#include <QHeaderView>
#include <kdebug.h>
#include <QVBoxLayout>

TrackListDlg::TrackListDlg(QWidget * parent)
    :KDialog(parent)
{
    QWidget *page = new QWidget( this );
    QVBoxLayout* vlay = new QVBoxLayout( page );

    m_ui = new trackListDlgUI( this );
    vlay->addWidget( m_ui );


    setMainWidget( page );
    setButtons( KDialog::Close );
    trackTableView = m_ui->trackTable;
    trackModel = trackTableView->model();
    trackTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    trackTableView->verticalHeader()->hide();

    setSizeIncrement ( 0, 50 );
    connect(m_ui->trackTable,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            this,SLOT(valueDoubleCliked(QTableWidgetItem*)));
}

TrackListDlg::~TrackListDlg()
{
}

void TrackListDlg::closeEvent( QCloseEvent * )
{
    emit trackListClosed();
}

void TrackListDlg::setAlbumLbl(const QString& album)
{
	m_ui->albumLbl->setText(album);
}

void TrackListDlg::setYearLbl(const QString& year)
{
	m_ui->yearLbl->setText(year);
}

void TrackListDlg::addItemTrackTable(int row,int column,const QString& item)
{
	trackModel->setData(trackModel->index(row,column), item);
}

int TrackListDlg::numberColumnTrackTable() const
{
	return m_ui->trackTable->columnCount();
}

void TrackListDlg::addRowTrackTable(int row)
{
	trackModel->insertRow(row);
}

bool TrackListDlg::removeRowsTrackTable(int count)
{
 	return trackModel->removeRows(0,count);
}

void TrackListDlg::valueDoubleCliked(QTableWidgetItem* item)
{
	emit(itemClicked(item->row()+ 1));
}

void TrackListDlg::moveTrackDialog(int x, int y)
{
	move(x,y);
}
