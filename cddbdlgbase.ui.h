/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void CDDBDlgBase::slotTrackSelected( QListViewItem *item )
{
  pb_trackInfo->setEnabled(true);
  le_trackTitle->setEnabled(true);
  lb_trackTitle->setEnabled(true);
  le_trackTitle->setText(item->text(2));
  le_trackTitle->setFocus();

  emit play(item->text(0).toInt()-1);
}


void CDDBDlgBase::slotTrackChanged( const QString &text )
{
  if (lv_trackList->currentItem())
    lv_trackList->currentItem()->setText(2, text);
}


void CDDBDlgBase::slotNextTrack()
{
  if (lv_trackList->currentItem())
  {
    QListViewItem *item = lv_trackList->currentItem()->nextSibling();
    lv_trackList->setSelected(item, true);
    lv_trackList->ensureItemVisible(item);
  }
}


void CDDBDlgBase::slotDiscInfoClicked()
{
  emit discInfoClicked();
}


void CDDBDlgBase::slotTrackInfoClicked()
{
  if (lv_trackList->currentItem())
  {
    int trackNum = lv_trackList->currentItem()->text(0).toInt();
    emit trackInfoClicked(trackNum);
  }
}
