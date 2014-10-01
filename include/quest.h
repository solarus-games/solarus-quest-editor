/*
 * Copyright (C) 2014 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_QUEST_H
#define SOLARUSEDITOR_QUEST_H

#include <solarus/QuestResourceList.h>
#include <QObject>

/**
 * @brief A Solarus project that can be open with the editor.
 */
class Quest: public QObject {
  Q_OBJECT

public:

  Quest();
  explicit Quest(const QString& root_path);

  QString get_root_path() const;
  void set_root_path(const QString& root_path);

  bool is_valid() const;
  bool exists() const;

  QString get_name() const;
  QString get_data_path() const;
  QString get_main_script_path() const;
  QString get_resource_path(Solarus::ResourceType resource_type) const;
  bool is_resource_path(const QString& path, Solarus::ResourceType& resource_type) const;
  bool is_in_resource_path(const QString& path, Solarus::ResourceType& resource_type) const;
  bool is_resource_element(const QString& path, Solarus::ResourceType& resource_type) const;

signals:

  // TODO
  // resource_element_added
  // resource_element_removed
  // resource_element_moved
  // resource_element_renamed

private:

  QString root_path;    /**< Root path of this quest.
                         * An empty string means no quest. */

};

#endif
