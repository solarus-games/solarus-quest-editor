/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_MUSIC_CHOOSER_H
#define SOLARUSEDITOR_MUSIC_CHOOSER_H

#include "widgets/resource_selector.h"
#include <QPointer>
#include <QToolButton>
#include <QWidget>

namespace SolarusEditor {

class Quest;

/**
 * @brief A music selector with a play/stop button.
 */
class MusicChooser: public QWidget{
  Q_OBJECT

public:

  MusicChooser(QWidget* parent);

  void set_quest(Quest& quest);
  ResourceSelector& get_selector();

  QString get_selected_id();
  void set_selected_id(const QString& music_id);

signals:

  void activated(const QString& music_id);

private slots:

  void music_selector_activated(const QString& music_id);
  void play_music_button_clicked();
  void quest_music_changed(const QString& music_id);

private:

  void update_play_button();

  ResourceSelector music_selector;
  QToolButton play_music_button;
  QString previous_selected_music_id;
  QPointer<Quest> quest;
};

}

#endif
