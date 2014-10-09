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
#ifndef SOLARUSEDITOR_TILESET_EDITOR_H
#define SOLARUSEDITOR_TILESET_EDITOR_H

#include "gui/editor.h"
#include "ui_tileset_editor.h"

/**
 * \brief A widget to edit graphically a tileset file.
 */
class TilesetEditor : public Editor {
  Q_OBJECT

public:

  TilesetEditor(Quest& quest, const QString& path, QWidget* parent = nullptr);

  virtual QString get_title() const override;
  virtual QIcon get_icon() const override;
  virtual bool is_modified() const override;
  virtual void save() override;
  virtual bool confirm_close() override;
  virtual void undo() override;
  virtual void redo() override;
  virtual void cut() override;
  virtual void copy() override;
  virtual void paste() override;

private:

  Ui::TilesetEditor ui;

};

#endif
