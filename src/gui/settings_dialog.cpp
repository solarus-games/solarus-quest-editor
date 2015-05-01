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
