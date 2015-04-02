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
#ifndef SOLARUSEDITOR_DIALOGS_EDITOR_H
#define SOLARUSEDITOR_DIALOGS_EDITOR_H

#include "gui/editor.h"
#include "ui_dialogs_editor.h"

class DialogsModel;

/**
 * \brief A widget to edit graphically a dialogs file.
 */
class DialogsEditor : public Editor {
  Q_OBJECT

public:

  DialogsEditor(
      Quest& quest, const QString& language_id, QWidget* parent = nullptr);
  ~DialogsEditor();

  DialogsModel& get_model();

  void set_selected_property(const QString &key);

  virtual void save() override;

public slots:

  void update();

  void update_language_id_field();

  void update_description_to_gui();
  void set_description_from_gui();

  void update_selection();

  void create_dialog_requested();
  void change_dialog_id_requested();
  void delete_dialog_requested();

  void update_dialog_view();
  void update_dialog_id_field();
  void update_dialog_text_field();
  void change_dialog_text_requested();
  void update_translation_text_field();

  void update_properties_buttons();

  void create_dialog_property_requested();
  void delete_dialog_property_requested();
  void change_dialog_property_key_requested();
  void change_dialog_property_value_requested(
      const QString &key, const QString &value);
  void set_dialog_property_from_translation_requested();

  void translation_selector_activated();
  void translation_refresh_requested();

private:

  Ui::DialogsEditor ui;      /**< The dialogs editor widgets. */
  QString language_id;       /**< Id of the language of dialogs being edited. */
  DialogsModel* model;       /**< Dialogs model being edited. */
  Quest& quest;              /**< The quest. */

};

#endif
