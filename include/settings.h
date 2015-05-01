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
#ifndef SOLARUSEDITOR_SETTINGS_H
#define SOLARUSEDITOR_SETTINGS_H

#include <QSettings>

/**
 * @brief Manages settings of the editor.
 */
class Settings {

public:

  // General keys.
  static const QString working_directory;

  // Text editor keys.
  static const QString font_family;
  static const QString font_size;

  Settings();

  QVariant get_value(const QString& key);
  int get_value_int(const QString& key);
  QString get_value_string(const QString& key);

  QVariant get_default(const QString& key);
  int get_default_int(const QString& key);
  QString get_default_string(const QString& key);

  void set_value(const QString& key, const QVariant& value);

  void restore_default();

private:

  QSettings settings;  /**< The settings. */

  static const QMap<QString, QVariant>
    default_values;    /**< The default values. */

};

#endif
