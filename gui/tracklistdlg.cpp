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

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>

#include <KConfigGroup>

TrackListDlg::TrackListDlg(QWidget * parent)
    :QDialog(parent)
{
    QWidget *page = new QWidget( this );
    QVBoxLayout* vlay = new QVBoxLayout( page );

    m_ui = new trackListDlgUI( this );
    vlay->addWidget( m_ui );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(page);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);
    m_ui->trackTable->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    m_ui->trackTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    m_ui->trackTable->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
    m_ui->trackTable->verticalHeader()->hide();

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
	m_ui->trackTable->setItem(row,column, new QTableWidgetItem(item));
}

int TrackListDlg::numberColumnTrackTable() const
{
	return m_ui->trackTable->columnCount();
}

void TrackListDlg::setRowCount(int nRows)
{
	m_ui->trackTable->setRowCount(nRows);
}

void TrackListDlg::removeRowsTrackTable()
{
 	m_ui->trackTable->setRowCount(0);
}

void TrackListDlg::valueDoubleCliked(QTableWidgetItem* item)
{
	emit(itemClicked(item->row()+ 1));
}

void TrackListDlg::moveTrackDialog(int x, int y)
{
	move(x,y);
}
