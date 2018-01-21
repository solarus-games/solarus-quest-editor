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
#ifndef SOLARUSEDITOR_DIALOGS_MODEL_H
#define SOLARUSEDITOR_DIALOGS_MODEL_H

#include "indexed_string_tree.h"

#include <solarus/core/DialogResources.h>
#include <QAbstractItemModel>
#include <QItemSelectionModel>

namespace SolarusEditor {

class Quest;

/**
 * @brief Model that wraps dialog resources.
 *
 * It makes the link between the editor and the dialogs data file of the
 * Solarus library.
 * Signals are sent when something changes in the wrapped strings.
 * This model can be used as a model for a tree view of dialogs.
 * It also stores the selection information.
 */
class DialogsModel : public QAbstractItemModel {
  Q_OBJECT

public:

  // Creation.
  DialogsModel(
      const Quest& quest, const QString& language_id,
      QObject* parent = nullptr);
  ~DialogsModel();

  const Quest& get_quest() const;
  QString get_language_id() const;

  // QStandardItemModel interface.
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex& model_index) const override;
  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex& model_index, int role) const override;

  // Dialogs.
  static bool is_valid_id(const QString& id);
  QString index_to_id(const QModelIndex& index) const;
  QModelIndex id_to_index(const QString& id) const;

  bool dialog_exists(const QString& id) const;
  bool dialog_property_exists(const QString& id, const QString& key) const;
  bool prefix_exists(const QString& prefix) const;
  QStringList get_ids(const QString& prefix) const;
  Solarus::DialogData get_dialog_data(const QString& id) const;
  QString get_dialog_text(const QString& id) const;
  QMap<QString, QString> get_dialog_properties(const QString& id) const;
  QString get_dialog_property(const QString& id, const QString& key) const;
  void create_dialog(const QString& id, const Solarus::DialogData& data);
  void create_dialog(
      const QString& id, const QString& text,
      const QMap<QString, QString> &properties);
  bool can_duplicate_dialogs(
      const QString& prefix, const QString& new_prefix, QString& id);
  void duplicate_dialogs(const QString& prefix, const QString& new_prefix);
  QString set_dialog_id(const QString& id, const QString& new_id);
  bool can_set_dialog_id_prefix(
      const QString& old_prefix, const QString& new_prefix, QString& id);
  QList<QPair<QString, QString> > set_dialog_id_prefix(
      const QString& old_prefix, const QString& new_prefix);
  void set_dialog_text(const QString& id, const QString& text);
  void set_dialog_property(
      const QString& id, const QString& key, const QString& value);
  void delete_dialog(const QString& id);
  void delete_dialog_property(const QString& id, const QString& key);
  QList<QPair<QString, Solarus::DialogData>> delete_prefix(
      const QString& prefix);

  // Selection.
  QItemSelectionModel& get_selection_model();
  bool is_selection_empty() const;
  QString get_selected_id() const;
  void set_selected_id(const QString& id);
  void clear_selection();

  // Translation.
  QString get_translation_id() const;
  void set_translation_id(const QString &language_id);
  void clear_translation();
  void reload_translation();

  bool translated_dialog_exists(const QString& id) const;
  QString get_translated_dialog_text(const QString& id) const;
  QMap<QString, QString> get_translated_dialog_properties(
      const QString& id) const;
  QString get_translated_dialog_property(
      const QString& id, const QString& key) const;

  QStringList get_translated_ids(const QString& prefix) const;
  bool has_missing_translation(const QString& id) const;

signals:

  void dialog_created(const QString& id);
  void dialog_deleted(const QString& id);
  void dialog_id_changed(const QString& id, const QString& new_id);
  void dialog_text_changed(const QString& id, const QString& new_text);
  void dialog_property_created(
      const QString& id, const QString& key, const QString& value);
  void dialog_property_deleted(const QString& id, const QString& key);
  void dialog_property_changed(
      const QString& id, const QString& key, const QString& value);

public slots:

  void save() const;

private:

  void build_dialog_tree();
  void clear_translation_from_tree();

  const Quest& quest;             /**< The quest the dialogs belongs to. */
  const QString language_id;      /**< Language of the dialogs. */
  Solarus::DialogResources
    resources;                    /**< Dialogs data wrapped by this model. */

  IndexedStringTree dialog_tree;  /**< Indexed tree of dialog ids.
                                   * The order is determined here. */

  QString translation_id;
  Solarus::DialogResources translation_resources;

  QItemSelectionModel
    selection_model;              /**< Dialog currently selected. */
};

}

#endif
