#ifndef CHANGE_PATTERN_ID_DIALOG_H
#define CHANGE_PATTERN_ID_DIALOG_H

#include "ui_change_pattern_id_dialog.h"
#include <QDialog>

/**
 * @brief A dialog to rename a pattern in the tileset editor.
 *
 * This dialog is similar to a standard QInputDialog with a line edit,
 * but has an additional checkbox to let the user choose if she wants
 * to update references in existing maps.
 */
class ChangePatternIdDialog : public QDialog {
  Q_OBJECT

public:

  ChangePatternIdDialog(
      const QString& initial_pattern_id, QWidget* parent = 0);

  QString get_pattern_id() const;
  void set_pattern_id(const QString& pattern_id);

  bool get_update_references() const;
  void set_update_references(bool update_references);

public slots:

  virtual void done(int result) override;

private:

  Ui::ChangePatternIdDialog ui;

};

#endif
