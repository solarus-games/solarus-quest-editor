#include "include/gui/find_text_dialog.h"
#include <QPushButton>

/**
 * @brief Creates a find text dialog.
 * @param parent The parent object or nullptr.
 */
FindTextDialog::FindTextDialog(QWidget* parent) :
  QDialog(parent),
  ui() {

  ui.setupUi(this);

  QPushButton* find_button = new QPushButton(tr("Find"), this);
  ui.button_box->addButton(find_button, QDialogButtonBox::ApplyRole);

  connect(find_button, &QPushButton::pressed, [&]() {
    emit find_text_requested(ui.find_field->text());
  });
}
