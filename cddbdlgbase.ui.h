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
  emit play(item->text(0).toUInt()-1);
}

void CDDBDlgBase::slotNextTrack()
{
  if (m_trackList->currentItem())
  {
    QListViewItem *item = m_trackList->currentItem()->nextSibling();
    m_trackList->setSelected(item, true);
    m_trackList->ensureItemVisible(item);
  }
}

void CDDBDlgBase::slotTrackDoubleClicked( QListViewItem *item, const QPoint &, int column)
{
    m_trackList->rename(item, column);
}


