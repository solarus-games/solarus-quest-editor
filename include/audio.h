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
#ifndef SOLARUSEDITOR_AUDIO_H
#define SOLARUSEDITOR_AUDIO_H

class QString;

namespace SolarusEditor {

class Quest;

namespace Audio {

void play_sound(const Quest& quest, const QString& sound_id);
void play_music(Quest& quest, const QString& music_id);
void stop_music(Quest& quest);
QString get_current_music_id(const Quest& quest);
bool is_playing_music(const Quest& quest);
bool is_playing_music(const Quest& quest, const QString& music_id);

}

}

#endif
