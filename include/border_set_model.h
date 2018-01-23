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
#ifndef SOLARUSEDITOR_BORDER_SET_MODEL_H
#define SOLARUSEDITOR_BORDER_SET_MODEL_H

#include "border_kind_traits.h"
#include <QAbstractItemModel>
#include <memory>

namespace SolarusEditor {

class TilesetModel;

/**
 * @brief Item model used by the border set tree view.
 */
class BorderSetModel : public QAbstractItemModel {
  Q_OBJECT

public:

  explicit BorderSetModel(TilesetModel& tileset, QObject* parent = nullptr);

  // QAbstractItemModel interface.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList& indexes) const override;
  Qt::DropActions supportedDropActions() const override;
  bool canDropMimeData(
      const QMimeData* data,
      Qt::DropAction action,
      int row,
      int column,
      const QModelIndex& parent
  ) const override;
  bool dropMimeData(
      const QMimeData* data,
      Qt::DropAction action,
      int row,
      int column,
      const QModelIndex& parent
  ) override;

  bool is_border_set_index(const QModelIndex& index) const;
  bool is_pattern_index(const QModelIndex& index) const;
  QString get_border_set_id(const QModelIndex& index) const;
  QString get_pattern_id(const QModelIndex& index) const;
  BorderKind get_border_kind(const QModelIndex& index) const;
  QPair<QString, QString> get_pattern_info(const QModelIndex& index) const;
  QModelIndex get_border_set_index(const QString& border_set_id) const;
  QModelIndex get_pattern_index(const QString& border_set_id, BorderKind border_kind) const;

signals:

  void change_border_set_patterns_requested(
      const QString& border_set_id,
      const QStringList& pattern_ids
  );

public slots:

  void border_set_created(const QString& border_set_id);
  void border_set_deleted(const QString& border_set_id);
  void border_set_id_changed(const QString& old_id, const QString& new_id);
  void border_set_pattern_changed(
      const QString& border_set_id,
      BorderKind border_kind,
      const QString& pattern_id
  );
  void pattern_id_changed(int old_index, const QString& old_id,
                          int new_index, const QString& new_id);

private:

  /**
   * @brief Data of a specific border set.
   */
  struct BorderSetIndex {

    /**
     * @brief Creates a border set id.
     * @param id Name of the border set.
     */
    explicit BorderSetIndex(const QString& border_set_id) :
      border_set_id(new QString(border_set_id)) {
    }

    std::shared_ptr<QString> border_set_id;
  };

  TilesetModel& tileset;                     /**< The tileset represented by this model. */
  QList<BorderSetIndex> border_set_indexes;  /**< Ids of the border sets in the model. */
};

}

#endif
