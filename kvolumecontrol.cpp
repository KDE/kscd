/*
 * KVolumeControl - a volume slider
 *
 * Copyright (c) 2003 Aaron J. Seigo
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qpushbutton.h>
#include <qslider.h>
#include <qtimer.h>

#include "kvolumecontrol.h"

KVolumeControl::KVolumeControl(QPushButton* surrogate, QWidget* parent, const char* name)
    : QHBox(parent, name),
      m_surrogate(surrogate)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(hide()));

    setFrameStyle(Panel | Raised);
    setLineWidth(1);

    m_surrogate->setToggleButton(true);
    m_surrogate->installEventFilter(this);
    setShown(m_surrogate->isOn());
    connect(m_surrogate, SIGNAL(toggled(bool)), this, SLOT(setShown(bool)));
    m_volumeSlider = new QSlider(0, 100, 5,  50, QSlider::Vertical, this);
    setMargin((surrogate->width() - m_volumeSlider->sizeHint().width()) / 2);
    m_volumeSlider->setFixedSize(surrogate->width() - lineWidth() * 2 - margin() * 2, 100);
    m_volumeSlider->installEventFilter(this);
    connect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(valueFlip(int)));
    adjustSize();
}

bool KVolumeControl::eventFilter(QObject* watched, QEvent* e)
{
    if (watched == m_surrogate && e->type() == QEvent::Wheel)
    {
        if (!m_surrogate->isOn())
        {
            m_surrogate->setOn(true);
        }

        if (static_cast<QWheelEvent*>(e)->delta() > 0)
        {
            m_volumeSlider->subtractStep();
        }
        else
        {
            m_volumeSlider->addStep();
        }

        return true;
    }
    else if (watched == m_volumeSlider && e->type() == KeyPress)
    {
        if (static_cast<QKeyEvent*>(e)->key() == Key_Escape)
        {
            m_surrogate->setOn(false);
            hide();
        }

        return true;
    }

    return false;
}

void KVolumeControl::setValue(int value)
{
    m_volumeSlider->setValue(100 - value);
}

void KVolumeControl::show()
{
    m_timer->start(2000, true);
    move(m_surrogate->pos() + QPoint(0, m_surrogate->height()));
    QHBox::show();
}

void KVolumeControl::hide()
{
    m_timer->stop();
    m_surrogate->setOn(false);
    QHBox::hide();
}

void KVolumeControl::valueFlip(int value)
{
    m_timer->start(2000, true);
    emit valueChanged(100 - value);
}

void KVolumeControl::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Key_Escape)
    {
        m_surrogate->setOn(false);
    }
}

void KVolumeControl::enterEvent(QEvent * e)
{
    m_timer->start(2000, true);
    QHBox::enterEvent(e);
}

void KVolumeControl::leaveEvent(QEvent * e)
{
    m_timer->start(500, true);
    QHBox::leaveEvent(e);
}

#include <kvolumecontrol.moc>

