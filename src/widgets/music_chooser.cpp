/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
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
#include "audio.h"
#include "quest.h"
#include "widgets/music_chooser.h"
#include <QHBoxLayout>

namespace SolarusEditor {

/**
 * @brief Creates a music chooser.
 * @param parent The parent object or nullptr.
 */
MusicChooser::MusicChooser(QWidget* parent) :
  QWidget(parent),
  music_selector(),
  play_music_button(),
  previous_selected_music_id(),
  quest() {

  music_selector.set_resource_type(ResourceType::MUSIC);
  play_music_button.setIconSize(QSize(24, 24));

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setMargin(0);

  layout->addWidget(&music_selector);
  layout->addWidget(&play_music_button);

  update_play_button();
}

/**
 * @brief Sets the quest whose resources will be shown.
 * @param quest The quest.
 */
void MusicChooser::set_quest(Quest& quest) {

  this->quest = &quest;

  music_selector.set_quest(quest);

  connect(&play_music_button, SIGNAL(clicked(bool)),
          this, SLOT(play_music_button_clicked()));
  connect(&quest, SIGNAL(current_music_changed(QString)),
          this, SLOT(quest_music_changed(QString)));
  connect(&music_selector, SIGNAL(activated(QString)),
          this, SLOT(music_selector_activated(QString)));

  update_play_button();
}

/**
 * @brief Returns the music selector contained in this widget.
 * @return The music selector.
 */
ResourceSelector& MusicChooser::get_selector() {
  return music_selector;
}

/**
 * @brief Returns the music id in the selector.
 * @return The selected music id.
 */
QString MusicChooser::get_selected_id() {
  return music_selector.get_selected_id();
}

/**
 * @brief Sets the selected music.
 * @param music_id The new music id to select.
 */
void MusicChooser::set_selected_id(const QString& music_id) {

  music_selector.set_selected_id(music_id);
  update_play_button();
}

/**
 * @brief Slot called when the user changes the music in the selector.
 * @param music_id The new selected music id.
 */
void MusicChooser::music_selector_activated(const QString& music_id) {

  if (quest == nullptr) {
    return;
  }

  const QString& old_music_id = previous_selected_music_id;
  const QString& new_music_id = music_selector.get_selected_id();
  if (new_music_id != old_music_id) {
    // There is a change.
    previous_selected_music_id = new_music_id;

    // Stop playing any music if there is a change.
    Audio::stop_music(*quest);

    update_play_button();
  }

  emit activated(music_id);
}

/**
 * @brief Slot called when the user clicks the play/stop music button.
 */
void MusicChooser::play_music_button_clicked() {

  if (quest == nullptr) {
    return;
  }

  const QString& selected_music_id = music_selector.get_selected_id();
  if (selected_music_id.isEmpty() ||
      selected_music_id == "none" ||
      selected_music_id == "same") {
    return;
  }

  const QString& current_music_id = Audio::get_current_music_id(*quest);
  if (current_music_id == selected_music_id) {
    // This music is already playing, stop it.
    Audio::stop_music(*quest);
  }
  else {
    Audio::play_music(*quest, selected_music_id);
  }
}

/**
 * @brief Slot called when a music is being played or stopped.
 * @param music_id The new current music or an empty string.
 */
void MusicChooser::quest_music_changed(const QString& music_id) {

  Q_UNUSED(music_id);
  update_play_button();
}

/**
 * @brief Updates the play/stop music button depending on the current
 * selection and the music currently playing if any.
 */
void MusicChooser::update_play_button() {

  play_music_button.setIcon(QIcon(":/images/icon_start.png"));
  play_music_button.setToolTip(tr("Play music"));

  if (quest == nullptr) {
    play_music_button.setEnabled(false);
    return;
  }

  QString selected_music_id = music_selector.get_selected_id();
  QString path = quest->get_resource_element_path(ResourceType::MUSIC, selected_music_id);

  if (selected_music_id.isEmpty() ||
      selected_music_id == "none" ||
      selected_music_id == "same" ||
      quest == nullptr ||
      !quest->exists(path)) {
    play_music_button.setEnabled(false);
  }
  else {
    play_music_button.setEnabled(true);
    const QString& music_playing_id = Audio::get_current_music_id(*quest);
    if (music_playing_id == selected_music_id) {
      play_music_button.setIcon(QIcon(":/images/icon_stop.png"));
      play_music_button.setToolTip(tr("Stop music"));
    }
  }
}

}
