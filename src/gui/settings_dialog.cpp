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
#include "include/gui/settings_dialog.h"
#include "include/settings.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

/**
 * @brief Creates a settings dialog.
 * @param parent Parent widget or nullptr.
 */
SettingsDialog::SettingsDialog(QWidget *parent) :
  QDialog(parent) {

  ui.setupUi(this);

  ui.quest_size_field->config("x", 0, 99999, 80);
  ui.map_grid_size_field->config("x", 8, 99999, 8);
  ui.sprite_grid_size_field->config("x", 8, 99999, 8);

  reset();

  connect(ui.button_box->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
          this, SLOT(reset()));
  connect(ui.button_box->button(QDialogButtonBox::RestoreDefaults),
          SIGNAL(clicked()), this, SLOT(restore_default()));
  connect(ui.button_box->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
          this, SLOT(apply()));

  // General.
  connect(ui.working_directory_field, SIGNAL(textChanged(QString)),
          this, SLOT(change_working_directory()));
  connect(ui.working_directory_button, SIGNAL(clicked()),
          this, SLOT(browse_working_directory()));
  connect(ui.save_files_field, SIGNAL(currentIndexChanged(int)),
          this, SLOT(change_save_files()));
  connect(ui.no_audio_field, SIGNAL(toggled(bool)),
          this, SLOT(change_no_audio()));
  connect(ui.video_acceleration_field, SIGNAL(toggled(bool)),
          this, SLOT(change_video_acceleration()));
  connect(ui.win_console_field, SIGNAL(toggled(bool)),
          this, SLOT(change_win_console()));
  connect(ui.quest_size_check_box, SIGNAL(toggled(bool)),
          this, SLOT(change_quest_size()));
  connect(ui.quest_size_field, SIGNAL(value_changed(int,int)),
          this, SLOT(change_quest_size()));

  // Text editor.
  connect(ui.font_family_field, SIGNAL(currentTextChanged(QString)),
          this, SLOT(change_font_family()));
  connect(ui.font_size_field, SIGNAL(valueChanged(int)),
          this, SLOT(change_font_size()));
  connect(ui.tab_length_field, SIGNAL(valueChanged(int)),
          this, SLOT(change_tab_length()));
  connect(ui.replace_tab_by_spaces, SIGNAL(toggled(bool)),
          this, SLOT(change_replace_tab_by_spaces()));

  // Map editor.
  connect(ui.map_background_field, SIGNAL(color_changed(QColor)),
          this, SLOT(change_map_background()));
  connect(ui.map_grid_show_at_opening_field, SIGNAL(clicked()),
          this, SLOT(change_map_grid_show_at_opening()));
  connect(ui.map_grid_size_field, SIGNAL(value_changed(int,int)),
          this, SLOT(change_map_grid_size()));
  connect(ui.map_grid_style_field, SIGNAL(currentIndexChanged(int)),
          this, SLOT(change_map_grid_style()));
  connect(ui.map_grid_color_field, SIGNAL(color_changed(QColor)),
          this, SLOT(change_map_grid_color()));

  // Sprite editor.
  connect(ui.sprite_main_background_field, SIGNAL(color_changed(QColor)),
          this, SLOT(change_sprite_main_background()));
  connect(ui.sprite_grid_show_at_opening_field, SIGNAL(clicked()),
          this, SLOT(change_sprite_grid_show_at_opening()));
  connect(ui.sprite_grid_size_field, SIGNAL(value_changed(int,int)),
          this, SLOT(change_sprite_grid_size()));
  connect(ui.sprite_grid_style_field, SIGNAL(currentIndexChanged(int)),
          this, SLOT(change_sprite_grid_style()));
  connect(ui.sprite_grid_color_field, SIGNAL(color_changed(QColor)),
          this, SLOT(change_sprite_grid_color()));
  connect(ui.sprite_auto_detect_grid_field, SIGNAL(clicked()),
          this, SLOT(change_sprite_auto_detect_grid()));
  connect(ui.sprite_previewer_background_field, SIGNAL(color_changed(QColor)),
          this, SLOT(change_sprite_previewer_background()));
  connect(ui.sprite_origin_show_at_opening_field, SIGNAL(clicked()),
          this, SLOT(change_sprite_origin_show_at_opening()));
  connect(ui.sprite_origin_color_field, SIGNAL(color_changed(QColor)),
          this, SLOT(change_sprite_origin_color()));
}

/**
 * @brief Closes the dialog and apply changes.
 * @param result Result code of the dialog.
 */
void SettingsDialog::done(int result) {

  if (result == QDialog::Accepted) {
    apply();
  } else {
    reset();
  }

  QDialog::done(result);
}

/**
 * @brief Slot called when the user want to reset settings (reload from file).
 */
void SettingsDialog::reset() {

  update();
  edited_settings.clear();
  update_buttons();
}

/**
 * @brief Slot called when the user want to restore default settings.
 */
void SettingsDialog::restore_default() {

  int result = QMessageBox::question(
    this, tr("Restore default settings"),
    tr("Do you really want to restore default settings?"),
    QMessageBox::Ok | QMessageBox::Cancel);

  if (result == QMessageBox::Ok) {
    settings.restore_default();
    reset();
    emit settings_changed();
  }
}

/**
 * @brief Slot called when the user want to apply settings.
 */
void SettingsDialog::apply() {

  for (QString key : edited_settings.keys()) {
    settings.set_value(key, edited_settings[key]);
  }
  reset();
  emit settings_changed();
}

/**
 * @brief Updates everything in the gui.
 */
void SettingsDialog::update() {

  // General.
  update_working_directory();
  update_save_files();
  update_no_audio();
  update_video_acceleration();
  update_win_console();
  update_quest_size();

  // Text editor.
  update_font_family();
  update_font_size();
  update_tab_length();
  update_replace_tab_by_spaces();

  // Map editor.
  update_map_background();
  update_map_grid_show_at_opening();
  update_map_grid_size();
  update_map_grid_style();
  update_map_grid_color();

  // Sprite editor.
  update_sprite_main_background();
  update_sprite_grid_show_at_opening();
  update_sprite_grid_size();
  update_sprite_grid_style();
  update_sprite_grid_color();
  update_sprite_auto_detect_grid();
  update_sprite_previewer_background();
  update_sprite_origin_show_at_opening();
  update_sprite_origin_color();
}

/**
 * @brief Updates buttons state.
 */
void SettingsDialog::update_buttons() {

  // Check if values are changed.
  bool changed = false;
  for (QString key : edited_settings.keys()) {

    QVariant value = edited_settings[key];
    if (value != settings.get_value(key)) {
      changed = true;
      break;
    }
  }

  // Get buttons to update & focused button.
  QPushButton* apply_button = ui.button_box->button(QDialogButtonBox::Apply);
  QPushButton* reset_button = ui.button_box->button(QDialogButtonBox::Reset);
  QWidget* focused = ui.button_box->focusWidget();

  // Check if the focus need to be changed.
  if (!changed && (focused == apply_button || focused == reset_button)) {
    ui.button_box->button(QDialogButtonBox::Ok)->setFocus();
  }

  // Update the state of buttons.
  apply_button->setEnabled(changed);
  reset_button->setEnabled(changed);
}

/**
 * @brief Updates the working directory field.
 */
void SettingsDialog::update_working_directory() {

  ui.working_directory_field->setText(
    settings.get_value_string(Settings::working_directory));
}

/**
 * @brief Slot called when the user changes the working directory.
 */
void SettingsDialog::change_working_directory() {

  edited_settings[Settings::working_directory] =
    ui.working_directory_field->text();
  update_buttons();
}

/**
 * @brief Slot called when the user click on the browse button.
 */
void SettingsDialog::browse_working_directory() {

  QString new_working_directory = QFileDialog::getExistingDirectory(
    this, tr("Working directory"),
    ui.working_directory_field->text());

  if (new_working_directory.isEmpty()) {
    return; // Canceled.
  }

  ui.working_directory_field->setText(new_working_directory);
  edited_settings[Settings::working_directory] = new_working_directory;
  update_buttons();
}

/**
 * @brief Updates the save files before running field.
 */
void SettingsDialog::update_save_files() {

  const QString& value = settings.get_value_string(Settings::save_files_before_running);
  int index = 0;
  if (value == "ask") {
    index = 0;
  }
  else if (value == "yes") {
    index = 1;
  }
  else if (value == "no") {
    index = 2;
  }
  ui.save_files_field->setCurrentIndex(index);
}

/**
 * @brief Slot called when the user changes the save files before running combobox.
 */
void SettingsDialog::change_save_files() {

  int index = ui.save_files_field->currentIndex();
  const QString values[] = { "ask", "yes", "no" };
  Q_ASSERT(index >= 0 && index < 3);
  edited_settings[Settings::save_files_before_running] = values[index];
  update_buttons();
}

/**
 * @brief Updates the no audio field.
 */
void SettingsDialog::update_no_audio() {

  ui.no_audio_field->setChecked(settings.get_value_bool(Settings::no_audio));
}

/**
 * @brief Slot called when the user changes the no audio.
 */
void SettingsDialog::change_no_audio() {

  edited_settings[Settings::no_audio] = ui.no_audio_field->isChecked();
  update_buttons();
}

/**
 * @brief Updates the video acceleration field.
 */
void SettingsDialog::update_video_acceleration() {

  ui.video_acceleration_field->setChecked(
    settings.get_value_bool(Settings::video_acceleration));
}

/**
 * @brief Slot called when the user changes the video acceleration.
 */
void SettingsDialog::change_video_acceleration() {

  edited_settings[Settings::video_acceleration] =
    ui.video_acceleration_field->isChecked();
  update_buttons();
}

/**
 * @brief Updates the win console field.
 */
void SettingsDialog::update_win_console() {

  ui.win_console_field->setChecked(
    settings.get_value_bool(Settings::win_console));
}

/**
 * @brief Slot called when the user changes the win console.
 */
void SettingsDialog::change_win_console() {

  edited_settings[Settings::win_console] = ui.win_console_field->isChecked();
  update_buttons();
}

/**
 * @brief Updates the quest size field.
 */
void SettingsDialog::update_quest_size() {

  QSize size = settings.get_value_size(Settings::quest_size);
  bool enable = size.isValid();

  ui.quest_size_check_box->setChecked(enable);
  ui.quest_size_field->setEnabled(enable);

  if (!enable) {
    size = QSize(320, 240);
  }

  ui.quest_size_field->set_size(size);
}

/**
 * @brief Slot called when the user changes the quest size.
 */
void SettingsDialog::change_quest_size() {

  bool enable = ui.quest_size_check_box->isChecked();

  QSize new_size;
  if (enable) {
    new_size = ui.quest_size_field->get_size();
  }

  ui.quest_size_field->setEnabled(enable);

  edited_settings[Settings::quest_size] = new_size;
  update_buttons();
}

/**
 * @brief Updates the font family field.
 */
void SettingsDialog::update_font_family() {

  ui.font_family_field->setCurrentText(
    settings.get_value_string(Settings::font_family));
}

/**
 * @brief Slot called when the user changes the font family.
 */
void SettingsDialog::change_font_family() {

  edited_settings[Settings::font_family] = ui.font_family_field->currentText();
  update_buttons();
}

/**
 * @brief Updates the font size field.
 */
void SettingsDialog::update_font_size() {

  ui.font_size_field->setValue(settings.get_value_int(Settings::font_size));
}

/**
 * @brief Slot called when the user changes the font size.
 */
void SettingsDialog::change_font_size() {

  edited_settings[Settings::font_size] = ui.font_size_field->value();
  update_buttons();
}

/**
 * @brief Updates the tab length field.
 */
void SettingsDialog::update_tab_length() {

  ui.tab_length_field->setValue(settings.get_value_int(Settings::tab_length));
}

/**
 * @brief Slot called when the user changes the tab length.
 */
void SettingsDialog::change_tab_length() {

  edited_settings[Settings::tab_length] = ui.tab_length_field->value();
  update_buttons();
}

/**
 * @brief Updates the replace tab by spaces field.
 */
void SettingsDialog::update_replace_tab_by_spaces() {

  ui.replace_tab_by_spaces->setChecked(
    settings.get_value_bool(Settings::replace_tab_by_spaces));
}

/**
 * @brief Slot called when the user changes the replace tab by spaces.
 */
void SettingsDialog::change_replace_tab_by_spaces() {

  edited_settings[Settings::replace_tab_by_spaces] =
    ui.replace_tab_by_spaces->isChecked();
}

/**
 * @brief Updates the map background field.
 */
void SettingsDialog::update_map_background() {

  ui.map_background_field->set_color(
    settings.get_value_color(Settings::map_background));
}

/**
 * @brief Slot called when the user changes the map background.
 */
void SettingsDialog::change_map_background() {

  edited_settings[Settings::map_background] =
    ui.map_background_field->get_color().name();
  update_buttons();
}

/**
 * @brief Updates the map grid show at opening field.
 */
void SettingsDialog::update_map_grid_show_at_opening() {

  ui.map_grid_show_at_opening_field->setChecked(
    settings.get_value_bool(Settings::map_grid_show_at_opening));
}

/**
 * @brief Slot called when the user changes the map grid show at opening.
 */
void SettingsDialog::change_map_grid_show_at_opening() {

  edited_settings[Settings::map_grid_show_at_opening] =
    ui.map_grid_show_at_opening_field->isChecked();
  update_buttons();
}

/**
 * @brief Updates the map grid size field.
 */
void SettingsDialog::update_map_grid_size() {

  ui.map_grid_size_field->set_size(
    settings.get_value_size(Settings::map_grid_size));
}

/**
 * @brief Slot called when the user changes the map grid size.
 */
void SettingsDialog::change_map_grid_size() {

  edited_settings[Settings::map_grid_size] = ui.map_grid_size_field->get_size();
  update_buttons();
}

/**
 * @brief Updates the map grid style field.
 */
void SettingsDialog::update_map_grid_style() {

  ui.map_grid_style_field->set_selected_value(static_cast<GridStyle>(
    settings.get_value_int(Settings::map_grid_style)));
}

/**
 * @brief Slot called when the user changes the map grid style.
 */
void SettingsDialog::change_map_grid_style() {

  edited_settings[Settings::map_grid_style] =
    static_cast<int>(ui.map_grid_style_field->get_selected_value());
  update_buttons();
}

/**
 * @brief Updates the map grid color field.
 */
void SettingsDialog::update_map_grid_color() {

  ui.map_grid_color_field->set_color(
    settings.get_value_color(Settings::map_grid_color));
}

/**
 * @brief Slot called when the user changes the map grid color.
 */
void SettingsDialog::change_map_grid_color() {

  edited_settings[Settings::map_grid_color] =
    ui.map_grid_color_field->get_color().name();
  update_buttons();
}

/**
 * @brief Updates the sprite main background field.
 */
void SettingsDialog::update_sprite_main_background() {

  ui.sprite_main_background_field->set_color(
    settings.get_value_color(Settings::sprite_main_background));
}

/**
 * @brief Slot called when the user changes the sprite main background.
 */
void SettingsDialog::change_sprite_main_background() {

  edited_settings[Settings::sprite_main_background] =
    ui.sprite_main_background_field->get_color().name();
  update_buttons();
}

/**
 * @brief Updates the sprite grid show at opening field.
 */
void SettingsDialog::update_sprite_grid_show_at_opening() {

  ui.sprite_grid_show_at_opening_field->setChecked(
    settings.get_value_bool(Settings::sprite_grid_show_at_opening));
}

/**
 * @brief Slot called when the user changes the sprite grid show at opening.
 */
void SettingsDialog::change_sprite_grid_show_at_opening() {

  edited_settings[Settings::sprite_grid_show_at_opening] =
    ui.sprite_grid_show_at_opening_field->isChecked();
  update_buttons();
}

/**
 * @brief Updates the sprite grid size field.
 */
void SettingsDialog::update_sprite_grid_size() {

  ui.sprite_grid_size_field->set_size(
    settings.get_value_size(Settings::sprite_grid_size));
}

/**
 * @brief Slot called when the user changes the sprite grid size.
 */
void SettingsDialog::change_sprite_grid_size() {

  edited_settings[Settings::sprite_grid_size] =
    ui.sprite_grid_size_field->get_size();
  update_buttons();
}

/**
 * @brief Updates the sprite grid style field.
 */
void SettingsDialog::update_sprite_grid_style() {

  ui.sprite_grid_style_field->set_selected_value(static_cast<GridStyle>(
    settings.get_value_int(Settings::sprite_grid_style)));
}

/**
 * @brief Slot called when the user changes the sprite grid style.
 */
void SettingsDialog::change_sprite_grid_style() {

  edited_settings[Settings::sprite_grid_style] =
    static_cast<int>(ui.sprite_grid_style_field->get_selected_value());
  update_buttons();
}

/**
 * @brief Updates the sprite grid color field.
 */
void SettingsDialog::update_sprite_grid_color() {

  ui.sprite_grid_color_field->set_color(
    settings.get_value_color(Settings::sprite_grid_color));
}

/**
 * @brief Slot called when the user changes the sprite grid color.
 */
void SettingsDialog::change_sprite_grid_color() {

  edited_settings[Settings::sprite_grid_color] =
    ui.sprite_grid_color_field->get_color().name();
  update_buttons();
}

/**
 * @brief Updates the sprite auto detect grid field.
 */
void SettingsDialog::update_sprite_auto_detect_grid() {

  ui.sprite_auto_detect_grid_field->setChecked(
    settings.get_value_bool(Settings::sprite_auto_detect_grid));
}

/**
 * @brief Slot called when the user changes the sprite auto detect grid.
 */
void SettingsDialog::change_sprite_auto_detect_grid() {

  edited_settings[Settings::sprite_auto_detect_grid] =
    ui.sprite_auto_detect_grid_field->isChecked();
  update_buttons();
}

/**
 * @brief Updates the sprite previewer background field.
 */
void SettingsDialog::update_sprite_previewer_background() {

  ui.sprite_previewer_background_field->set_color(
    settings.get_value_color(Settings::sprite_previewer_background));
}

/**
 * @brief Slot called when the user changes the sprite previewer background.
 */
void SettingsDialog::change_sprite_previewer_background() {

  edited_settings[Settings::sprite_previewer_background] =
    ui.sprite_previewer_background_field->get_color().name();
  update_buttons();
}

/**
 * @brief Updates the sprite origin show at opening field.
 */
void SettingsDialog::update_sprite_origin_show_at_opening() {

  ui.sprite_origin_show_at_opening_field->setChecked(
    settings.get_value_bool(Settings::sprite_origin_show_at_opening));
}

/**
 * @brief Slot called when the user changes the sprite origin show at opening.
 */
void SettingsDialog::change_sprite_origin_show_at_opening() {

  edited_settings[Settings::sprite_origin_show_at_opening] =
    ui.sprite_origin_show_at_opening_field->isChecked();
  update_buttons();
}

/**
 * @brief Updates the sprite origin color field.
 */
void SettingsDialog::update_sprite_origin_color() {

  ui.sprite_origin_color_field->set_color(
    settings.get_value_color(Settings::sprite_origin_color));
}

/**
 * @brief Slot called when the user changes the sprite origin color.
 */
void SettingsDialog::change_sprite_origin_color() {

  edited_settings[Settings::sprite_origin_color] =
    ui.sprite_origin_color_field->get_color().name();
  update_buttons();
}
