/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_QUEST_FILES_MODEL_H
#define SOLARUSEDITOR_QUEST_FILES_MODEL_H

#include "quest_resources.h"
#include <QSet>
#include <QSortFilterProxyModel>
#include <array>

class QFileSystemModel;

namespace SolarusEditor {

class Quest;

/**
 * @brief A hierarchical model of files, resources and scripts of a quest.
 *
 * This models acts as a proxy to a filesytem model.
 * It filters, modifies and extends the files representation to present them
 * as quest resources.
 *
 * Resources that are declared in the resource list but missing on the
 * filesystem appear in the model with a warning icon.
 *
 * Similarly, existing files whose name looks like a resource but that are not
 * declared in the resource list appear in the model with an interrogation mark
 * icon.
 */
class QuestFilesModel : public QSortFilterProxyModel {
  Q_OBJECT

public:

  static constexpr int FILE_COLUMN = 0;          /**< Column index of the file in the model. */
  static constexpr int DESCRIPTION_COLUMN = 1;   /**< Column index of the resource description in the model. */
  static constexpr int TYPE_COLUMN = 2;          /**< Column index of the type info in the model. */
  static constexpr int NUM_COLUMNS = 3;          /**< Number of columns of the model. */

  explicit QuestFilesModel(Quest& quest);

  Quest& get_quest();
  QModelIndex get_quest_root_index() const;
  QString get_file_path(const QModelIndex& index) const;
  QModelIndex get_file_index(const QString& path) const;

  // The proxy changes the underlying filesystem model a lot:
  // rows and columns are added and removed.
  // We have to reimplement a lot of functions, in particular because
  // adding extra rows breaks an assumption of QSortFilterProxyModel:
  // the fact that a valid proxy index can always be mapped to a valid
  // source index.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  QModelIndex sibling(int row, int column, const QModelIndex& idx) const override;
  bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex mapToSource(const QModelIndex& proxy_index) const override;
  QItemSelection mapSelectionToSource(const QItemSelection& proxy_selection) const override;

  // Data.
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual QVariant headerData(
      int section, Qt::Orientation orientation, int role = Qt::DisplayRole
      ) const override;
  virtual QVariant data(
      const QModelIndex& index, int role = Qt::DisplayRole
      ) const override;
  virtual bool setData(
      const QModelIndex& index, const QVariant& value, int role = Qt::EditRole
      ) override;

protected:

  virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
  virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private slots:

  void resource_element_added(
      ResourceType resource_type, const QString& element_id, const QString& description);
  void resource_element_removed(
      ResourceType resource_type, const QString& element_id);
  void resource_element_renamed(
      ResourceType resource_type, const QString& old_id, const QString& new_id);
  void resource_element_description_changed(
      ResourceType resource_type, const QString& element_id, const QString& description);

  void source_model_rows_inserted(const QModelIndex& source_parent, int first, int last);
  void source_model_rows_about_to_be_removed(const QModelIndex& source_parent, int first, int last);

private:

  using ExtraPathColumnPtrs = std::array<QString*, NUM_COLUMNS>;

  /**
   * @brief For a directory, list of the paths added by this model but that
   * do not exist on the filesystem.
   *
   * These paths correspond to resource elements that are declared in the quest
   * but that are missing on the filesystem.
   * There is a lot of redundancy for performance.
   */
  struct ExtraPaths {
    QList<ExtraPathColumnPtrs> paths;       /**< Paths stored as pointers
                                             * in order to use QModelIndex::internalPointer().
                                             * This class has exclusive ownership of the pointers. */
    QMap<QString, int> path_indexes;        /**< Fast access to the index of each
                                             * element in paths (redundant information). */
    QStringList element_ids;                /**< The corresponding resource element ids
                                             * (redundant information). */

    ~ExtraPaths();
    void rebuild_index_cache();
  };

  QIcon get_quest_file_icon(const QModelIndex& index) const;
  QString get_quest_file_tooltip(const QModelIndex& index) const;
  bool is_quest_data_index(const QModelIndex& index) const;

  bool is_dir_on_filesystem(const QModelIndex& index) const;
  bool is_extra_path(const QModelIndex& index, QString& path) const;
  int get_num_extra_paths(const QModelIndex& parent) const;
  ExtraPaths* get_extra_paths(const QModelIndex& parent) const;
  void compute_extra_paths(const QModelIndex& parent) const;
  void insert_extra_path(const QModelIndex& parent, const QString& path);
  void remove_extra_path(const QModelIndex& parent, const QString& path);
  void rebuild_extra_path_indexes_cache(const QModelIndex& parent);

  Quest& quest;                        /**< The quest represented by this model. */
  QFileSystemModel* source_model;      /**< The underlying file model. */

  mutable QMap<QString, ExtraPaths>
      extra_paths_by_dir;              /**< For each directory, extra paths added by
                                        * this model but that don't exist in the source model. */
  mutable QSet<const QString*>
      all_extra_paths;                 /**< List of all paths stored in extra_paths
                                        * (redundant info for performance). */

};

}

#endif
