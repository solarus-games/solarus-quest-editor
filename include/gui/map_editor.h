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
#ifndef SOLARUSEDITOR_MAP_EDITOR_H
#define SOLARUSEDITOR_MAP_EDITOR_H

#include "gui/editor.h"
#include "ui_map_editor.h"

class MapModel;

/**
 * \brief A widget to edit graphically a map file.
 */
class MapEditor : public Editor {
  Q_OBJECT

public:

  MapEditor(Quest& quest, const QString& path, QWidget* parent = nullptr);

  MapModel& get_model();

  virtual void save() override;

private:

  Ui::MapEditor ui;         /**< The map editor widgets. */
  QString map_id;           /**< Id of the map being edited. */

};

#endif
