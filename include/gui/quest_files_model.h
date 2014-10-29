/*
 * Copyright (C) 2014 Christopho, Solarus - http://www.solarus-games.org
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
#include <QSortFilterProxyModel>

class Quest;
class QFileSystemModel;

/**
 * @brief A hierarchical model of files, resources and scripts of a quest.
 *
 * This models acts as a proxy to a filesytem model, filtering and modifying
 * the files representation to present them as quest resources.
 *
 * Resources that are declared in the resource list but missing on the
 * filesystem appear in the model with a warning icon.
 * Similarly, existing files whose name looks like a resource but that are no
 * declared in the resource list appear in the model with an interrogation mark
 * icon.
 */
class QuestFilesModel : public QSortFilterProxyModel {
  Q_OBJECT

public:

  static constexpr int FILE_COLUMN = 0;          /**< Column index of the file in the model. */
  static constexpr int DESCRIPTION_COLUMN = 1;   /**< Column index of the resource description in the model. */
  static constexpr int TYPE_COLUMN = 2;          /**< Column index of the type info in the model. */

  explicit QuestFilesModel(Quest& quest);

  Quest& get_quest();
  QModelIndex get_quest_root_index() const;
  QString get_file_path(const QModelIndex& index) const;
  QModelIndex get_file_index(const QString& path) const;

  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
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

  void resource_element_description_changed(
      ResourceType resource_type, const QString& element_id, const QString& description);

private:

  QString get_quest_file_icon_name(const QModelIndex& index) const;
  QString get_quest_file_tooltip(const QModelIndex& index) const;
  bool is_quest_data_index(const QModelIndex& source_index) const;

  Quest& quest;                        /**< The quest represented by this model. */
  QFileSystemModel* source_model;      /**< The underlying file model. */

};

#endif
