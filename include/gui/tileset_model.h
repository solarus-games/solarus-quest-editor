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
#ifndef SOLARUSEDITOR_TILESET_MODEL_H
#define SOLARUSEDITOR_TILESET_MODEL_H

#include "tile_pattern_animation.h"
#include "tile_pattern_separation.h"
#include <solarus/entities/TilesetData.h>
#include <QAbstractItemModel>
#include <QCollator>
#include <QImage>
#include <QItemSelectionModel>
#include <QList>
#include <QPixmap>
#include <map>

class Quest;

using Ground = Solarus::Ground;
using Layer = Solarus::Layer;

// TODO move to a separate file
class NaturalComparator {

public:

  NaturalComparator() {
    collator.setNumericMode(true);
  }

  bool operator() (const QString& lhs, const QString& rhs) const {
    return collator.compare(lhs, rhs) < 0;
  }

private:

  QCollator collator;

};

/**
 * @brief Model that wraps a tileset.
 *
 * It makes the link between the editor and the tileset data of the
 * Solarus library.
 * Each tile pattern is identified by both its string id and an integer index
 * for performance.
 * Signals are sent when something changes in the wrapped tileset.
 * This model can be used as a model for a list view of tile patterns.
 * It also stores the selection information.
 */
class TilesetModel : public QAbstractListModel {
  Q_OBJECT

public:

  // Creation.
  TilesetModel(
      Quest& quest, const QString& tileset_id, QObject* parent = nullptr);

  Quest& get_quest();
  QString get_tileset_id() const;

  // QAbstractListModel interface.
  virtual int rowCount(const QModelIndex& parent) const override;
  virtual QVariant data(const QModelIndex& index, int role) const override;

  // Tileset data.
  QColor get_background_color() const;
  void set_background_color(const QColor& background_color);
  int get_num_patterns() const;
  bool pattern_exists(int index) const;
  bool is_pattern_multi_frame(int index) const;
  int get_pattern_num_frames(int index) const;
  QRect get_pattern_frame(int index) const;
  QList<QRect> get_pattern_frames(int index) const;
  QRect get_pattern_frames_bounding_box(int index) const;
  Ground get_pattern_ground(int index) const;
  void set_pattern_ground(int index, Ground ground);
  Layer get_pattern_default_layer(int index) const;
  void set_pattern_default_layer(int index, Layer default_layer);
  TilePatternAnimation get_pattern_animation(int index) const;
  void set_pattern_animation(int index, TilePatternAnimation animation);
  TilePatternSeparation get_pattern_separation(int index) const;
  void set_pattern_separation(int index, TilePatternSeparation separation);
  QPixmap get_pattern_image(int index) const;
  QPixmap get_pattern_image_all_frames(int index) const;
  QPixmap get_pattern_icon(int index) const;
  QImage get_patterns_image() const;

  int id_to_index(const QString& pattern_id) const;
  QString index_to_id(int index) const;

  // Selected patterns.
  QItemSelectionModel& get_selection();
  int get_selected_index() const;

signals:

  void background_color_changed(const QColor& background_color);
  void pattern_ground_changed(int index, Ground ground);
  void pattern_default_layer_changed(int index, Layer default_layer);
  void pattern_animation_changed(int index, TilePatternAnimation animation);
  void pattern_separation_changed(int index, TilePatternSeparation separation);

public slots:

  void save() const;

private:

  /**
   * @brief Data of a specific tile pattern.
   */
  struct PatternModel {

  public:

    PatternModel(const QString& id) :
      id(id) {
    }

    QString id;                   /**< String id of the pattern. */
    mutable QPixmap image;        /**< Full-size image of the pattern,
                                   * only of the first frame for
                                   * multi-frame patterns. */
    mutable QPixmap
        image_all_frames;         /**< Full-size image of the pattern,
                                   * with all frames for multi-frame
                                   * patterns. */
    mutable QPixmap icon;         /**< 32x32 icon of the pattern. */
  };

  void build_index_map();

  Quest& quest;                   /**< The quest the tileset belongs to. */
  const QString tileset_id;       /**< Id of the tileset. */
  Solarus::TilesetData tileset;   /**< Tileset data wrapped by this model. */
  QImage patterns_image;          /**< PNG image of all tile patterns. */

  std::map<QString, int, NaturalComparator>
      ids_to_indexes;             /**< Index in the list of each pattern.
                                   * The order is determined here. */
  QList<PatternModel>
      patterns;                   /**< All patterns. */

  QItemSelectionModel selection;  /**< Patterns currently selected. */

};

#endif
