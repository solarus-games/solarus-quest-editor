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
#ifndef SOLARUSEDITOR_STRINGS_EDITOR_H
#define SOLARUSEDITOR_STRINGS_EDITOR_H

#include "gui/editor.h"
#include "ui_strings_editor.h"

class StringsModel;

/**
 * \brief A widget to edit graphically a strings file.
 */
class StringsEditor : public Editor {
  Q_OBJECT

public:

  StringsEditor(
      Quest& quest, const QString& language_id, QWidget* parent = nullptr);
  ~StringsEditor();

  StringsModel& get_model();

  virtual void save() override;

public slots:

  void update();

  void update_language_id_field();

  void update_description_to_gui();
  void set_description_from_gui();

  void update_selection();
  void create_string_requested();
  void change_string_key_requested();
  void delete_string_requested();

  void set_value_requested(const QString& key, const QString& value);

  void translation_selector_activated();
  void translation_refresh_requested();

private:

  Ui::StringsEditor ui;      /**< The strings editor widgets. */
  QString language_id;       /**< Id of the language of dialogs being edited. */
  StringsModel* model;       /**< Strings model being edited. */
  Quest& quest;              /**< The quest. */

};

#endif
