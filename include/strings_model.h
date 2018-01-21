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
#ifndef SOLARUSEDITOR_STRINGS_MODEL_H
#define SOLARUSEDITOR_STRINGS_MODEL_H

#include "indexed_string_tree.h"

#include <solarus/core/StringResources.h>
#include <QAbstractItemModel>
#include <QItemSelectionModel>

namespace SolarusEditor {

class Quest;

/**
 * @brief Model that wraps string resources.
 *
 * It makes the link between the editor and the strings data file of the
 * Solarus library.
 * Signals are sent when something changes in the wrapped strings.
 * This model can be used as a model for a tree view of strings.
 * It also stores the selection information.
 */
class StringsModel : public QAbstractItemModel {
  Q_OBJECT

public:

  // Creation.
  StringsModel(
      const Quest& quest, const QString& language_id,
      QObject* parent = nullptr);
  ~StringsModel();

  const Quest& get_quest() const;
  QString get_language_id() const;

  // QStandardItemModel interface.
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex& model_index) const override;
  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex& model_index, int role) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual bool setData(
      const QModelIndex& index, const QVariant& value,
      int role = Qt::EditRole) override;

  // Strings.
  static bool is_valid_key(const QString& key);
  QString index_to_key(const QModelIndex& index) const;
  QModelIndex key_to_index(const QString& key, int column = 0) const;

  bool string_exists(const QString& key) const;
  bool prefix_exists(const QString& prefix) const;
  QStringList get_keys(const QString& prefix) const;
  QString get_string(const QString& key) const;
  void create_string(const QString& key, const QString& value);
  bool can_duplicate_strings(
      const QString& prefix, const QString& new_prefix, QString& key);
  void duplicate_strings(const QString& prefix, const QString& new_prefix);
  void set_string(const QString& key, const QString& value);
  QString set_string_key(const QString& key, const QString& new_key);
  bool can_set_string_key_prefix(
      const QString& old_prefix, const QString& new_prefix, QString& key);
  QList<QPair<QString, QString> > set_string_key_prefix(
      const QString& old_prefix, const QString& new_prefix);
  void delete_string(const QString& key);
  QList<QPair<QString, QString>> delete_prefix(const QString& prefix);

  // Selection.
  QItemSelectionModel& get_selection_model();
  bool is_selection_empty() const;
  QString get_selected_key() const;
  void set_selected_key(const QString& key);
  void clear_selection();

  // Translation.
  QString get_translation_id() const;
  void set_translation_id(const QString &language_id);
  void clear_translation();
  void reload_translation();

  bool translated_string_exists(const QString& key) const;
  QString get_translated_string(const QString& key) const;

  QStringList get_translated_keys(const QString& prefix) const;
  bool has_missing_translation(const QString& key) const;

signals:

  void string_created(const QString& key);
  void string_deleted(const QString& key);
  void string_key_changed(const QString& key, const QString& new_key);
  void string_value_changed(const QString& key, const QString& new_value);

  void set_value_requested(const QString& key, const QString& value);

public slots:

  void save() const;

private:

  void build_string_tree();
  void clear_translation_from_tree();

  const Quest& quest;             /**< The quest the strings belongs to. */
  const QString language_id;      /**< Language of the strings. */
  Solarus::StringResources
    resources;                    /**< Strings data wrapped by this model. */

  IndexedStringTree string_tree;  /**< Indexed tree of string keys.
                                   * The order is determined here. */

  QString translation_id;
  Solarus::StringResources translation_resources;

  QItemSelectionModel
    selection_model;              /**< String currently selected. */
};

}

#endif
