// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "alarmbar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

AlarmBar::AlarmBar(QWidget *parent) : QWidget(parent)
{
  setAutoFillBackground(true);
  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(10, 4, 6, 4);
  m_text = new QLabel(this);
  m_text->setWordWrap(true);
  QFont f = m_text->font();
  f.setBold(true);
  f.setPointSizeF(f.pointSizeF() * 1.2);
  m_text->setFont(f);
  layout->addWidget(m_text, 1);
  m_ack = new QPushButton(tr("&Acknowledge"), this);
  m_ack->setToolTip(tr("Hides the banner; the alarm stays active until the reading is back."));
  connect(m_ack, &QPushButton::clicked, this, &AlarmBar::acknowledged);
  layout->addWidget(m_ack);
  hide();
}

void AlarmBar::setAlarms(const QString &text, const QColor &color)
{
  if (text.isEmpty())
  {
    hide();
    return;
  }
  QPalette pal = palette();
  pal.setColor(QPalette::Window, color);
  // white or black text, whichever reads on the colour
  const bool dark = color.lightness() < 140;
  pal.setColor(QPalette::WindowText, dark ? Qt::white : Qt::black);
  setPalette(pal);
  m_text->setPalette(pal);
  m_text->setText(text);
  show();
}
