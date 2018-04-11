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
#ifndef SOLARUSEDITOR_IMPORT_DIALOG_H
#define SOLARUSEDITOR_IMPORT_DIALOG_H

#include "quest.h"
#include "ui_import_dialog.h"
#include <QDialog>
#include <QMessageBox>

class QFileInfo;

namespace SolarusEditor {

/**
 * @brief Dialog to import files from another quest.
 */
class ImportDialog : public QDialog {
  Q_OBJECT

public:

  explicit ImportDialog(Quest& destination_quest, QWidget* parent = nullptr);

  const Quest& get_source_quest() const;
  Quest& get_destination_quest() const;

signals:

  void destination_quest_rename_file_requested(Quest& quest, const QString& path);

private slots:

  void browse_source_quest();
  void source_quest_root_path_changed();
  void source_quest_selected_path_changed();
  void update_find_missing_button();
  void find_missing_button_triggered();
  void update_import_button();
  void import_button_triggered();
  void select_recently_created_paths();

private:

  void import_path(const QString& source_path);
  void import_file(const QFileInfo& source_info);
  void import_dir(const QFileInfo& source_info);
  void import_path_meta_information(const QString& source_path, const QString& destination_path);
  QString source_to_destination_path(const QString& source_path);
  void find_source_paths_not_in_destination_quest(
      const QString& source_path,
      QStringList& missing_source_paths
  );

  Ui::ImportDialog ui;

  Quest source_quest;
  Quest& destination_quest;
  QMessageBox::StandardButton last_confirm_overwrite_file;
  QMessageBox::StandardButton last_confirm_overwrite_directory;
  QStringList paths_to_select;
};

}

#endif
