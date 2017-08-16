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
#ifndef SOLARUSEDITOR_BORDER_SET_MODEL_H
#define SOLARUSEDITOR_BORDER_SET_MODEL_H

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
  QModelIndex parent(const QModelIndex& model_index) const override;
  QVariant data(const QModelIndex& model_index, int role) const override;

  QString get_border_set_id(int row) const;

private:

  /**
   * @brief Data of a specific border set.
   */
  struct BorderSetId {

    /**
     * @brief Creates a border set id.
     * @param id Name of the border set.
     */
    explicit BorderSetId(const QString& id) :
      id(new QString(id)) {
    }

    std::shared_ptr<QString> id;
  };

  TilesetModel& tileset;             /**< The tileset represented by this model. */
  QList<BorderSetId> border_set_ids; /**< Ids of the border sets in the model. */
};

}

#endif
