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
#ifndef SOLARUSEDITOR_EDIT_ENTITY_DIALOG_H
#define SOLARUSEDITOR_EDIT_ENTITY_DIALOG_H

#include "ui_edit_entity_dialog.h"
#include "entities/entity_traits.h"

namespace SolarusEditor {

class MapModel;
enum class ResizeMode;

class EditEntityDialog : public QDialog {
  Q_OBJECT

public:

  explicit EditEntityDialog(EntityModel& entity_before, QWidget* parent = nullptr);

  Quest& get_quest() const;
  MapModel& get_map() const;
  EntityIndex get_entity_index() const;
  EntityModel& get_entity_before() const;
  EntityModelPtr get_entity_after();

private slots:

  void width_changed(int width);
  void height_changed(int height);
  void direction_changed();

  void add_user_property_requested();
  void change_user_property_key_requested();
  void delete_user_property_requested();
  void move_up_user_property_requested();
  void move_down_user_property_requested();

  void user_property_double_clicked(QTreeWidgetItem* item, int column);

  void update_user_property_buttons();

private:

  void initialize();
  void apply();
  void initialize_simple_booleans();
  void apply_simple_booleans();
  void initialize_simple_integers();
  void apply_simple_integers();
  void initialize_simple_strings();
  void apply_simple_strings();

  void initialize_behavior();
  void apply_behavior();
  void initialize_breed();
  void apply_breed();
  void initialize_damage_on_enemies();
  void apply_damage_on_enemies();
  void initialize_destination();
  void apply_destination();
  void initialize_destination_map();
  void apply_destination_map();
  void initialize_direction();
  void apply_direction();
  void initialize_font();
  void apply_font();
  void initialize_ground();
  void apply_ground();
  void initialize_layer();
  void apply_layer();
  void initialize_maximum_moves();
  void apply_maximum_moves();
  void initialize_model();
  void apply_model();
  void initialize_name();
  void apply_name();
  void initialize_opening_method();
  void apply_opening_method();
  void initialize_pattern();
  void apply_pattern();
  void initialize_savegame_variable();
  void apply_savegame_variable();
  void initialize_size();
  void apply_size();
  void initialize_sound();
  void apply_sound();
  void initialize_sprite();
  void apply_sprite();
  void initialize_starting_location_mode();
  void apply_starting_location_mode();
  void initialize_subtype();
  void apply_subtype();
  void initialize_transition();
  void apply_transition();
  void initialize_treasure();
  void apply_treasure();
  void initialize_type();
  void apply_type();
  void initialize_weight();
  void apply_weight();
  void initialize_xy();
  void apply_xy();
  void initialize_user_properties();
  void apply_user_properties();

  void initialize_possibly_optional_field(const QString& field_name,
                                          QLayout* label_layout,
                                          QWidget* label,
                                          QCheckBox* checkbox,
                                          QWidget* field);
  void remove_field(QWidget* label, QWidget* field);

  void hide_unexisting_opening_methods();
  QRadioButton* get_opening_method_radio_button(const QString& opening_method);
  QMap<QString, QRadioButton*> get_opening_method_radio_buttons();
  QString get_selected_opening_method();

  QValidator* create_name_validator();
  QValidator* create_dialog_id_validator();
  QValidator* create_savegame_variable_validator();

  void update_size_constraints();

  bool user_property_exists(const QString &key) const;

  Ui::EditEntityDialog ui;             /**< The widgets. */
  EntityModel& entity_before;          /**< The entity to edit (remains unchanged). */
  EntityModelPtr entity_after;         /**< A copy of the entity with the modified data. */
  ResizeMode resize_mode;              /**< The current resize mode. */

  /**
   * @brief Info about basic boolean fields represented by a checkbox.
   *
   * This is used for fields that do not need special code.
   */
  struct SimpleBooleanField {

    SimpleBooleanField(
        const QString& field_name,
        const QString& label_text,
        const QString& checkbox_text,
        QWidget* before_widget = nullptr) :
      field_name(field_name),
      label_text(label_text),
      checkbox_text(checkbox_text),
      checkbox(nullptr),
      before_widget(before_widget) {
    }

    QString field_name;
    QString label_text;
    QString checkbox_text;
    const QCheckBox* checkbox;
    QWidget* before_widget;
  };

  /**
   * @brief Info about basic integer fields represented by a spinbox.
   *
   * This is used for integer fields that do not need special code.
   */
  struct SimpleIntegerField {

    SimpleIntegerField(
        const QString& field_name,
        const QString& label_text,
        int minimum,
        int step,
        QWidget* before_widget = nullptr) :
      field_name(field_name),
      label_text(label_text),
      minimum(minimum),
      step(step),
      spinbox(nullptr),
      before_widget(before_widget) {
    }

    QString field_name;
    QString label_text;
    int minimum;
    int step;
    const QSpinBox* spinbox;
    QWidget* before_widget;
  };

  /**
   * @brief Info about basic string fields represented by a lineedit.
   *
   * This is used for string fields that do not need special code.
   */
  struct SimpleStringField {

    SimpleStringField(
        const QString& field_name,
        const QString& label_text,
        QValidator* validator,
        QWidget* before_widget = nullptr) :
      field_name(field_name),
      label_text(label_text),
      checkbox(nullptr),
      line_edit(nullptr),
      validator(validator),
      before_widget(before_widget) {
    }

    QString field_name;
    QString label_text;
    QCheckBox* checkbox;
    const QLineEdit* line_edit;
    QValidator* validator;
    QWidget* before_widget;
  };

  QList<SimpleBooleanField> simple_boolean_fields;
  QList<SimpleIntegerField> simple_integer_fields;
  QList<SimpleStringField> simple_string_fields;

};

}

#endif
