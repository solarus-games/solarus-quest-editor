/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
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

#include "natural_comparator.h"
#include "pattern_animation.h"
#include "pattern_separation.h"
#include <solarus/entities/TilesetData.h>
#include <QAbstractItemModel>
#include <QImage>
#include <QItemSelectionModel>
#include <QList>
#include <QPixmap>
#include <map>

namespace SolarusEditor {

class Quest;

using Ground = Solarus::Ground;
using TilePatternRepeatMode = Solarus::TilePatternRepeatMode;

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

  // Tileset data.
  QColor get_background_color() const;
  void set_background_color(const QColor& background_color);

  // QAbstractListModel interface.
  virtual int rowCount(const QModelIndex& parent) const override;
  virtual QVariant data(const QModelIndex& index, int role) const override;

  int get_num_patterns() const;
  bool pattern_exists(int index) const;
  bool pattern_exists(const QString& pattern_id) const;
  int id_to_index(const QString& pattern_id) const;
  QString index_to_id(int index) const;
  int create_pattern(const QString& pattern_id, const QRect& frame);
  void delete_pattern(int index);
  void delete_patterns(const QList<int>& indexes);
  int set_pattern_id(int index, const QString& new_id);
  static bool is_valid_pattern_id(const QString& pattern_id);

  bool is_pattern_multi_frame(int index) const;
  int get_pattern_num_frames(int index) const;
  QRect get_pattern_frame(int index) const;
  QList<QRect> get_pattern_frames(int index) const;
  QRect get_pattern_frames_bounding_box(int index) const;
  void set_pattern_position(int index, const QPoint& position);
  Ground get_pattern_ground(int index) const;
  bool is_common_pattern_ground(const QList<int>& indexes, Ground& ground) const;
  void set_pattern_ground(int index, Ground ground);
  int get_pattern_default_layer(int index) const;
  bool is_common_pattern_default_layer(const QList<int>& indexes, int& default_layer) const;
  void set_pattern_default_layer(int index, int default_layer);
  TilePatternRepeatMode get_pattern_repeat_mode(int index) const;
  bool is_common_pattern_repeat_mode(const QList<int>& indexes, TilePatternRepeatMode& repeat_mode) const;
  void set_pattern_repeat_mode(int index, TilePatternRepeatMode repeat_mode);
  PatternAnimation get_pattern_animation(int index) const;
  bool is_common_pattern_animation(const QList<int>& indexes, PatternAnimation& animation) const;
  void set_pattern_animation(int index, PatternAnimation animation);
  PatternSeparation get_pattern_separation(int index) const;
  bool is_common_pattern_separation(const QList<int>& indexes, PatternSeparation& separation) const;
  void set_pattern_separation(int index, PatternSeparation separation);

  QPixmap get_pattern_image(int index) const;
  QPixmap get_pattern_image_all_frames(int index) const;
  QPixmap get_pattern_icon(int index) const;
  QImage get_patterns_image() const;
  void reload_patterns_image();

  // Selected patterns.
  QItemSelectionModel& get_selection_model();
  bool is_selection_empty() const;
  int get_selection_count() const;
  int get_selected_index() const;
  QList<int> get_selected_indexes() const;
  void set_selected_index(int index);
  void set_selected_indexes(const QList<int>& indexes);
  void add_to_selected(int index);
  void add_to_selected(const QList<int>& index);
  bool is_selected(int index) const;
  void toggle_selected(int index);
  void select_all();
  void clear_selection();

signals:

  void background_color_changed(const QColor& background_color);
  void image_changed();

  void pattern_created(int new_index, const QString& new_id);
  void pattern_deleted(int old_index, const QString& old_id);
  void pattern_id_changed(int old_index, const QString& old_id,
                          int new_index, const QString& new_id);
  void pattern_position_changed(int index, const QPoint& position);
  void pattern_ground_changed(int index, Ground ground);
  void pattern_default_layer_changed(int index, int default_layer);
  void pattern_repeat_mode_changed(int index, TilePatternRepeatMode repeat_mode);
  void pattern_animation_changed(int index, PatternAnimation animation);
  void pattern_separation_changed(int index, PatternSeparation separation);

public slots:

  void save() const;

private:

  /**
   * @brief Data of a specific tile pattern.
   */
  struct PatternModel {

  public:

    /**
     * @brief Creates a tile pattern model.
     * @param id Id of the tile pattern to represent.
     */
    PatternModel(const QString& id) :
      id(id) {
    }

    /**
     * @brief Clears the image cache of this pattern.
     */
    void set_image_dirty() const {
      image = QPixmap();
      image_all_frames = QPixmap();
      icon = QPixmap();
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

  QItemSelectionModel
      selection_model;            /**< Patterns currently selected. */

};

}

#endif
