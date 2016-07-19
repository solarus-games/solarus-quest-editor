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
#include "widgets/sound_chooser.h"
#include <QHBoxLayout>

namespace SolarusEditor {

/**
 * @brief Creates a sound chooser.
 * @param parent The parent object or nullptr.
 */
SoundChooser::SoundChooser(QWidget* parent) :
  QWidget(parent),
  sound_selector(),
  play_sound_button(),
  quest() {

  sound_selector.set_resource_type(ResourceType::SOUND);
  play_sound_button.setIconSize(QSize(24, 24));
  play_sound_button.setIcon(QIcon(":/images/icon_start.png"));
  play_sound_button.setToolTip(tr("Play sound"));

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setMargin(0);

  layout->addWidget(&sound_selector);
  layout->addWidget(&play_sound_button);

  update_play_button();
}

/**
 * @brief Sets the quest whose resources will be shown.
 * @param quest The quest.
 */
void SoundChooser::set_quest(Quest& quest) {

  this->quest = &quest;

  sound_selector.set_quest(quest);

  connect(&play_sound_button, SIGNAL(clicked(bool)),
          this, SLOT(play_sound_button_clicked()));
  connect(&sound_selector, SIGNAL(activated(QString)),
          this, SLOT(sound_selector_activated(QString)));

  update_play_button();
}

/**
 * @brief Returns the sound selector contained in this widget.
 * @return The sound selector.
 */
ResourceSelector& SoundChooser::get_selector() {
  return sound_selector;
}

/**
 * @brief Returns the sound id in the selector.
 * @return The selected sound id.
 */
QString SoundChooser::get_selected_id() {
  return sound_selector.get_selected_id();
}

/**
 * @brief Sets the selected sound.
 * @param sound_id The new sound id to select.
 */
void SoundChooser::set_selected_id(const QString& sound_id) {

  sound_selector.set_selected_id(sound_id);
  update_play_button();
}

/**
 * @brief Slot called when the user changes the sound in the selector.
 * @param sound_id The new selected sound id.
 */
void SoundChooser::sound_selector_activated(const QString& sound_id) {

  if (quest == nullptr) {
    return;
  }

  update_play_button();

  emit activated(sound_id);
}

/**
 * @brief Slot called when the user clicks the play sound button.
 */
void SoundChooser::play_sound_button_clicked() {

  if (quest == nullptr) {
    return;
  }

  const QString& selected_sound_id = sound_selector.get_selected_id();
  if (selected_sound_id.isEmpty()) {
    return;
  }

  Audio::play_sound(*quest, selected_sound_id);
}


/**
 * @brief Updates the play sound button depending on the current
 * selection.
 */
void SoundChooser::update_play_button() {

  if (quest == nullptr) {
    play_sound_button.setEnabled(false);
    return;
  }

  QString selected_sound_id = sound_selector.get_selected_id();
  QString path = quest->get_resource_element_path(
        ResourceType::SOUND, selected_sound_id
  );

  if (selected_sound_id.isEmpty() ||
      !quest->exists(path)) {
    play_sound_button.setEnabled(false);
  }
  else {
    play_sound_button.setEnabled(true);
  }
}

}
