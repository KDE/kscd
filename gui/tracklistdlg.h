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

#ifndef TRACKLISTDLG_H
#define TRACKLISTDLG_H

#include "ui_trackListDlgUI.h"

#include <KDialog>

#include <kdebug.h>

class trackListDlgUI : public QWidget, public Ui::trackListDlgUI
{
   public:
      trackListDlgUI( QWidget *parent ) : QWidget( parent ) {
         setupUi( this );
      }
};

class TrackListDlg : public KDialog
{
    Q_OBJECT

private:
    /** Table view instance for the trackTable */
    QTableView* trackTableView ;

    QAbstractItemModel* trackModel;
public:
    /** Create an instance of TrackListDlg */
    TrackListDlg(QWidget* parent = 0);

    /** Destroy an instance of TrackListDlg */
    ~TrackListDlg();

    /** Modify the text of the album label
     * @return void
     **/
    void setAlbumLbl(const QString&);

    /** Modify the text of the year label
     * @return void
     **/
    void setYearLbl(const QString&);

    /** Add a row to the track table
     * @return void
     **/
    void addItemTrackTable(int,int,const QString&);

    /** Return the number of column of the track table
     * @return int
     **/
    int numberColumnTrackTable() const;

    /** Add a row to the track table
     * @return void
     **/
    void addRowTrackTable(int);

    /** Remove all rows to the track table
     * @return void
     **/
    bool removeRowsTrackTable(int count);

    /** Move the track dialog
     * @return void
     **/
    void moveTrackDialog(int, int);

private:
    void closeEvent( QCloseEvent * event );
public slots:
    /** Mouse double click event on a row of the track table
     * @return void
     **/
    void valueDoubleCliked(QTableWidgetItem*);

signals:
    /** Send the postion of the clicked item**/
    void itemClicked(int);
    void trackListClosed();
private:
    trackListDlgUI *m_ui;

};

#endif // TRACKLISTDLG_H
