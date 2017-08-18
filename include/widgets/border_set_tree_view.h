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
#ifndef SOLARUSEDITOR_BORDER_SET_TREE_VIEW_H
#define SOLARUSEDITOR_BORDER_SET_TREE_VIEW_H

#include <QTreeView>

namespace SolarusEditor {

class BorderSetModel;
class TilesetModel;

/**
 * @brief Tree view of a tileset's border sets.
 */
class BorderSetTreeView : public QTreeView {
  Q_OBJECT

public:

  explicit BorderSetTreeView(QWidget* parent = nullptr);

  void set_tileset(TilesetModel& tileset);

  QStringList get_selection() const;

signals:

  void delete_border_sets_requested(const QStringList& border_set_ids);
  void change_border_set_patterns_requested(
      const QString& border_set_id,
      const QStringList& pattern_ids
  );

public slots:

  void delete_border_set_selection_requested();

private:

  TilesetModel* tileset;              /**< The tileset. */
  BorderSetModel* model;              /**< The item model of this view. */

};

}

#endif
