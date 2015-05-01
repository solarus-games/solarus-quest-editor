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
  connect(ui.no_audio_field, SIGNAL(toggled(bool)),
          this, SLOT(change_no_audio()));
  connect(ui.video_acceleration_field, SIGNAL(toggled(bool)),
          this, SLOT(change_video_acceleration()));
  connect(ui.win_console_field, SIGNAL(toggled(bool)),
          this, SLOT(change_win_console()));
  connect(ui.quest_size_field, SIGNAL(toggled(bool)),
          this, SLOT(change_quest_size()));
  connect(ui.quest_size_width_field, SIGNAL(valueChanged(int)),
          this, SLOT(change_quest_size()));
  connect(ui.quest_size_height_field, SIGNAL(valueChanged(int)),
          this, SLOT(change_quest_size()));

  // Text editor.
  connect(ui.font_family_field, SIGNAL(currentTextChanged(QString)),
          this, SLOT(change_font_family()));
  connect(ui.font_size_field, SIGNAL(valueChanged(int)),
          this, SLOT(change_font_size()));
}

/**
 * @brief Closes the dialog and apply changes.
 * @param result Result code of the dialog.
 */
void SettingsDialog::done(int result) {

  if (result == QDialog::Accepted) {
    apply();
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
  update_no_audio();
  update_video_acceleration();
  update_win_console();
  update_quest_size();

  // Text editor.
  update_font_family();
  update_font_size();
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

  ui.quest_size_field->setChecked(enable);
  ui.quest_size_width_field->setEnabled(enable);
  ui.quest_size_height_field->setEnabled(enable);

  if (!enable) {
    size = QSize(320, 240);
  }

  ui.quest_size_width_field->setValue(size.width());
  ui.quest_size_height_field->setValue(size.height());
}

/**
 * @brief Slot called when the user changes the quest size.
 */
void SettingsDialog::change_quest_size() {

  bool enable = ui.quest_size_field->isChecked();

  QSize new_size;
  if (enable) {
    new_size.setWidth(ui.quest_size_width_field->value());
    new_size.setHeight(ui.quest_size_height_field->value());
  }

  ui.quest_size_width_field->setEnabled(enable);
  ui.quest_size_height_field->setEnabled(enable);

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
