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

// General keys.
const QString Settings::working_directory = "working_directory";

// Text editor keys.
const QString Settings::font_family = "text_editor/font_family";
const QString Settings::font_size = "text_editor/font_size";

const QMap<QString, QVariant> Settings::default_values = {

  // General.
  { Settings::working_directory, "" },

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
