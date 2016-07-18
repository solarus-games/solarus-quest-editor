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
#include "quest.h"
#include "sound.h"
#include <solarus/lowlevel/QuestFiles.h>
#include <solarus/lowlevel/Sound.h>
#include <solarus/Arguments.h>
#include <QApplication>
#include <QDebug>
#include <QTimer>

namespace SolarusEditor {

namespace {

bool initialized = false;

/**
 * @brief Initializes the sound features.
 */
void initialize() {

  Solarus::Sound::initialize(Solarus::Arguments());
  initialized = true;

  // Cleanup Solarus sound system at exit.
  QObject::connect(
        QCoreApplication::instance(),
        &QApplication::aboutToQuit,
        Solarus::Sound::quit
  );

  // Update the Solarus sound system from time to time.
  QTimer* update_timer = new QTimer(QCoreApplication::instance());
  QObject::connect(
        update_timer,
        &QTimer::timeout,
        Solarus::Sound::update
  );
  update_timer->setSingleShot(false);
  update_timer->start(10);
}

/**
 * @brief Opens a quest on the engine's side, making it the current one.
 * @param quest The quest to open.
 * @return @c true in case of success.
 */
bool open_quest(const Quest& quest) {

  // TODO factorize this more.
  QStringList arguments = QApplication::arguments();
  QString program_name = arguments.isEmpty() ? QString() : arguments.first();
  return Solarus::QuestFiles::open_quest(program_name.toStdString(),
                                         quest.get_root_path().toStdString());
}

/**
 * @brief Closes the current quest on the engine's side.
 */
void close_quest() {
  Solarus::QuestFiles::close_quest();
}

}  // Anonymous namespace.

namespace Sound {

/**
 * @brief Plays a sound of the given quest.
 * @param quest The quest.
 * @param sound_id Id of the sound to play: filename without extension,
 * relative to the sounds directory.
 */
void play_sound(const Quest& quest, const QString& sound_id) {

  if (!initialized) {
    initialize();
  }

  if (!open_quest(quest)) {
    qWarning() << "Failed to open quest " << quest.get_root_path();
    return;
  }

  if (!Solarus::Sound::exists(sound_id.toStdString())) {
    qWarning() << "Cannot open sound file " << sound_id;
    return;
  }
  Solarus::Sound::play(sound_id.toStdString());

  close_quest();
}

/**
 * @brief Plays a music of the given quest.
 * @param quest The quest.
 * @param music_id Id of the music to play: filename without extension,
 * relative to the musics directory.
 */
void play_music(const Quest& quest, const QString& music_id) {

  if (!initialized) {
    initialize();
  }

  Q_UNUSED(quest);
  Q_UNUSED(music_id);
}

/**
 * @brief Stops playing any music.
 */
void stop_music() {

  if (!initialized) {
    initialize();
  }
}

}  // namespace Sound

}  // namespace SolarusEditor
