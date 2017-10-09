/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus Quest Editor is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus Quest Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SOLARUSEDITOR_SOUND_CHOOSER_H
#define SOLARUSEDITOR_SOUND_CHOOSER_H

#include "widgets/resource_selector.h"
#include <QPointer>
#include <QToolButton>
#include <QWidget>

namespace SolarusEditor {

class Quest;

/**
 * @brief A sound selector with a play button.
 */
class SoundChooser: public QWidget{
  Q_OBJECT

public:

  SoundChooser(QWidget* parent);

  void set_quest(Quest& quest);
  ResourceSelector& get_selector();

  QString get_selected_id();
  void set_selected_id(const QString& sound_id);

signals:

  void activated(const QString& sound_id);

private slots:

  void sound_selector_activated(const QString& sound_id);
  void play_sound_button_clicked();

private:

  void update_play_button();

  ResourceSelector sound_selector;
  QToolButton play_sound_button;
  QPointer<Quest> quest;
};

}

#endif
