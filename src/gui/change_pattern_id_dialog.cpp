#include "gui/change_pattern_id_dialog.h"
#include "gui/gui_tools.h"
#include "tileset_model.h"

/**
 * @brief Creates a change pattern id dialog.
 * @param initial_pattern_id Initial value of the pattern id.
 * @param parent Parent object or nullptr.
 */
ChangePatternIdDialog::ChangePatternIdDialog(
    const QString& initial_pattern_id, QWidget* parent) :
  QDialog(parent) {

  ui.setupUi(this);

  ui.pattern_id_label->setText(tr("New id for pattern '%1':").arg(initial_pattern_id));
  set_pattern_id(initial_pattern_id);
  ui.pattern_id_field->selectAll();
}

/**
 * @brief Returns pattern id entered by the user.
 * @return The pattern id.
 */
QString ChangePatternIdDialog::get_pattern_id() const {

  return ui.pattern_id_field->text();
}

/**
 * @brief Sets the pattern id displayed in the text edit.
 * @param value The value to set.
 */
void ChangePatternIdDialog::set_pattern_id(const QString& pattern_id) {

  ui.pattern_id_field->setText(pattern_id);
}

/**
 * @brief Returns whether the "update references in existing maps" option is checked.
 * @return @c true if the user wants to update refences.
 */
bool ChangePatternIdDialog::get_update_references() const {

  return ui.update_references_checkbox->isChecked();
}

/**
 * @brief Sets whether "update references in existing maps" is checked.
 * @param update_references @c true to check the option.
 */
void ChangePatternIdDialog::set_update_references(bool update_references) {

  ui.update_references_checkbox->setChecked(update_references);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void ChangePatternIdDialog::done(int result) {

  if (result == QDialog::Accepted) {

    if (get_pattern_id().isEmpty()) {
      GuiTools::error_dialog("Empty pattern id");
      return;
    }

    if (!TilesetModel::is_valid_pattern_id(get_pattern_id())) {
      GuiTools::error_dialog("Invalid tile pattern id");
      return;
    }
  }

  QDialog::done(result);
}
