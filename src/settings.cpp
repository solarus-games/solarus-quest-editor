/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#include "include/settings.h"
#include <QSize>

// General keys.
const QString Settings::working_directory = "working_directory";
const QString Settings::no_audio = "no_audio";
const QString Settings::video_acceleration = "video_acceleration";
const QString Settings::win_console = "win_console";
const QString Settings::quest_size = "quest_size";

// Text editor keys.
const QString Settings::font_family = "text_editor/font_family";
const QString Settings::font_size = "text_editor/font_size";

const QMap<QString, QVariant> Settings::default_values = {

  // General.
  { Settings::working_directory, "" },
  { Settings::no_audio, false },
  { Settings::video_acceleration, true },
  { Settings::win_console, false },
  { Settings::quest_size, QSize() },

  // Text editor.
  { Settings::font_family, "DejaVu Sans Mono" },
  { Settings::font_size, 10 },
};

/**
 * @brief Creates settings.
 */
Settings::Settings() :
  settings("solarus", "solarus-quest-editor") {
}

/**
 * @brief Returns a settings value.
 * @param key The key of the setting.
 * @return The value of the setting.
 */
QVariant Settings::get_value(const QString& key) {

  if (default_values.contains(key)) {
    return settings.value(key, default_values[key]);
  }
  return settings.value(key);
}

/**
 * @brief Returns a settings boolean value.
 * @param key The key of the setting.
 * @return The boolean value of the setting.
 */
bool Settings::get_value_bool(const QString& key) {

  return get_value(key).toBool();
}

/**
 * @brief Returns a settings integer value.
 * @param key The key of the setting.
 * @return The integer value of the setting.
 */
int Settings::get_value_int(const QString& key) {

  return get_value(key).toInt();
}

/**
 * @brief Returns a settings string value.
 * @param key The key of the setting.
 * @return The string value of the setting.
 */
QString Settings::get_value_string(const QString& key) {

  return get_value(key).toString();
}

/**
 * @brief Returns a settings size value.
 * @param key The key of the setting.
 * @return The size value of the setting.
 */
QSize Settings::get_value_size(const QString& key) {

  return get_value(key).toSize();
}

/**
 * @brief Returns a settings default value.
 * @param key The key of the setting.
 * @return The default value of the setting.
 */
QVariant Settings::get_default(const QString& key) {

  if (!default_values.contains(key)) {
    return QVariant();
  }
  return default_values[key];
}

/**
 * @brief Returns a settings default boolean value.
 * @param key The key of the setting.
 * @return The default boolean value of the setting.
 */
bool Settings::get_default_bool(const QString& key) {

  return get_default(key).toBool();
}

/**
 * @brief Returns a settings default integer value.
 * @param key The key of the setting.
 * @return The default integer value of the setting.
 */
int Settings::get_default_int(const QString& key) {

  return get_default(key).toInt();
}

/**
 * @brief Returns a settings default string value.
 * @param key The key of the setting.
 * @return The default string value of the setting.
 */
QString Settings::get_default_string(const QString& key) {

  return get_default(key).toString();
}

/**
 * @brief Returns a settings default size value.
 * @param key The key of the setting.
 * @return The default size value of the setting.
 */
QSize Settings::get_default_size(const QString& key) {

  return get_default(key).toSize();
}

/**
 * @brief Changes a settings value.
 * @param key The key of the setting.
 * @param value The new value of the setting.
 */
void Settings::set_value(const QString& key, const QVariant& value) {

  settings.setValue(key, value);
}

/**
 * @brief Restores all default values.
 */
void Settings::restore_default() {

  for (QString key : default_values.keys()) {
    settings.setValue(key, default_values[key]);
  }
}
