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
#ifndef SOLARUSEDITOR_SPRITE_MODEL_H
#define SOLARUSEDITOR_SPRITE_MODEL_H

#include "natural_comparator.h"

#include <solarus/SpriteData.h>
#include <QAbstractItemModel>
#include <QImage>
#include <QItemSelectionModel>
#include <QPixmap>
#include <map>
#include <memory>

namespace SolarusEditor {

class Quest;

/**
 * @brief Model that wraps a sprite.
 *
 * It makes the link between the editor and the sprite data of the
 * Solarus library.
 * Signals are sent when something changes in the wrapped sprite.
 * This model can be used as a model for a tree view of animations
 * and directions.
 * It also stores the selection information.
 */
class SpriteModel : public QAbstractItemModel {
  Q_OBJECT

public:

  /**
   * @brief Index identifying an animation and direction of a sprite.
   */
  struct Index {

  public:

    /**
     * @brief Creates a sprite direction index.
     * @param animation_name Animation name of this index.
     * @param direction_nb Direction number of this index.
     */
    Index(const QString& animation_name = "", int direction_nb = -1) :
      animation_name(animation_name),
      direction_nb(direction_nb) {
    }

    /**
     * @brief Returns whether this index is valid.
     * @return \c true if this index is valid.
     */
    bool is_valid() const {
      return !animation_name.isEmpty();
    }

    /**
     * @brief Returns whether this index represents an animation.
     * @return \c true if this index represents an animation.
     */
    bool is_animation_index() const {
      return !animation_name.isEmpty() && direction_nb < 0;
    }

    /**
     * @brief Returns whether this index represents a direction.
     * @return \c true if this index represents a direction.
     */
    bool is_direction_index() const {
      return !animation_name.isEmpty() && direction_nb >= 0;
    }

    QString animation_name; /**< Animation name of this index. */
    int direction_nb;       /**< Direction number of this index. */
  };

  // Creation.
  SpriteModel(const Quest& quest, const QString& sprite_id, QObject* parent = nullptr);

  const Quest& get_quest() const;
  QString get_sprite_id() const;
  QString get_default_animation_name() const;
  void set_default_animation_name(const QString& default_animation_name);
  bool exists(const Index& index) const;
  QModelIndex get_model_index(const Index& index) const;
  QString get_tileset_id() const;
  void set_tileset_id(const QString& tileset_id);

  // QAbstractItemModel interface.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& model_index) const override;
  bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& model_index, int role) const override;

  // Animation.
  int get_animation_nb(const Index& index) const;
  Index get_animation_index(int animation_nb) const;

  bool animation_exists(const Index& index) const;
  void create_animation(const QString& animation_name);
  void insert_animation(
      const Index& index, const Solarus::SpriteAnimationData& data);
  void delete_animation(const Index& index);
  void set_animation_name(const Index& index, const QString& new_name);

  Solarus::SpriteAnimationData get_animation_data(const Index& index) const;
  QString get_animation_source_image(const Index& index) const;
  bool is_animation_image_is_tileset(const Index& index) const;
  void set_animation_source_image(const Index& index, const QString &src_image);
  uint32_t get_animation_frame_delay(const Index& index) const;
  void set_animation_frame_delay(const Index& index, uint32_t frame_delay);
  int get_animation_loop_on_frame(const Index& index) const;
  void set_animation_loop_on_frame(const Index& index, int loop_on_frame);
  int get_animation_num_directions(const Index& index) const;
  int get_animation_max_num_frames(const Index& index) const;

  // Direction.
  bool direction_exists(const Index& index) const;
  int add_direction(
    const Index& index, const QRect &frame,
    int num_frames = 1, int num_columns = 1);
  int insert_direction(
      const Index& index, const Solarus::SpriteAnimationDirectionData &data);
  void delete_direction(const Index& index);
  void move_direction(const Index& index, int new_direction_nb);

  Solarus::SpriteAnimationDirectionData
    get_direction_data(const Index& index) const;
  QRect get_direction_first_frame_rect(const Index& index) const;
  QRect get_direction_all_frames_rect(const Index& index) const;
  QList<QRect> get_direction_frames(const Index& index) const;
  QPoint get_direction_position(const Index& index) const;
  void set_direction_position(const Index& index, const QPoint& position);
  QSize get_direction_size(const Index& index) const;
  void set_direction_size(const Index& index, const QSize& size);
  QPoint get_direction_origin(const Index& index) const;
  void set_direction_origin(const Index& index, const QPoint& origin);
  bool is_direction_multi_frame(const Index& index) const;
  int get_direction_num_frames(const Index& index) const;
  void set_direction_num_frames(const Index& index, int num_frames);
  int get_direction_num_columns(const Index& index) const;
  void set_direction_num_columns(const Index& index, int num_columns);

  // Images.
  QImage get_animation_image(const Index& index) const;
  QList<QPixmap> get_direction_all_frames(const Index& index) const;
  QPixmap get_direction_first_frame(const Index& index) const;
  QPixmap get_direction_frame(const Index& index, int frame) const;
  QPixmap get_direction_icon(const Index& index) const;
  QPixmap get_icon() const;

  // Selection.
  QItemSelectionModel& get_selection_model();
  bool is_selection_empty() const;
  Index get_selected_index() const;
  void set_selected_index(const Index& index);
  void set_selected_animation(const Index& index);
  void clear_selection();

signals:

  void default_animation_changed(
      const QString& old_default_animation, const QString& new_default_animation);
  void animation_created(const Index& index);
  void animation_deleted(const Index& index);
  void animation_name_changed(const Index& old_index, const Index& new_index);
  void animation_image_changed(const Index& index, const QString& src_image);
  void animation_frame_delay_changed(const Index& index, uint32_t frame_delay);
  void animation_loop_on_frame_changed(const Index& index, int loop_on_frame);

  void direction_added(const Index& index);
  void direction_deleted(const Index& index);
  void direction_moved(const Index& index, int new_direction_nb);
  void direction_position_changed(const Index& index, const QPoint& position);
  void direction_size_changed(const Index& index, const QSize& size);
  void direction_origin_changed(const Index& index, const QPoint& origin);
  void direction_num_frames_changed(const Index& index, int num_frames);
  void direction_num_columns_changed(const Index& index, int num_columns);

public slots:

  void save() const;

private:

  /**
   * @brief Data of a specific sprite animation direction.
   */
  struct DirectionModel {

  public:

    /**
     * @brief Creates a sprite animation direction model.
     */
    DirectionModel(const QString& animation_name, int direction_nb) :
      index(new Index(animation_name, direction_nb)) {
    }

    /**
     * @brief Clears the image cache of this direction.
     */
    void set_image_dirty() const {
      frames.clear();
      icon = QPixmap();
    }

    std::shared_ptr<Index> index;   /**< Index of the direction. */
    mutable QList<QPixmap> frames;  /**< All frames of the direction. */
    mutable QPixmap icon;           /**< 32x32 icon of the direction. */
  };

  /**
   * @brief Data of a specific sprite animation.
   */
  struct AnimationModel {

  public:

    /**
     * @brief Creates a sprite animation model.
     * @param name Name of the animation to represent.
     */
    AnimationModel(const QString& animation_name) :
      index(new Index(animation_name)) {
    }

    /**
     * @brief Changes the name of this animation.
     */
    void set_animation_name(const QString& animation_name) {
      index->animation_name = animation_name;
      for (auto& direction: directions) {
        direction.index->animation_name = animation_name;
      }
    }

    /**
     * @brief Clears the image cache of this animation.
     */
    void set_image_dirty() const {
      image = QImage();
      for (const auto& direction: directions) {
        direction.set_image_dirty();
      }
    }

    std::shared_ptr<Index> index;     /**< Index of the animation. */
    QList<DirectionModel> directions; /**< Directions of the animation. */
    mutable QImage image;             /**< Image of the animation. */
  };

  void build_index_map();

  void set_animation_image_dirty(const Index& index);
  void set_direction_image_dirty(const Index& index);

  const Solarus::SpriteAnimationData& get_animation(const Index& index) const;
  Solarus::SpriteAnimationData& get_animation(const Index& index);

  const Solarus::SpriteAnimationDirectionData& get_direction(const Index& index) const;
  Solarus::SpriteAnimationDirectionData& get_direction(const Index& index);

  const Quest& quest;             /**< The quest the sprite belongs to. */
  const QString sprite_id;        /**< Id of the sprite. */
  Solarus::SpriteData sprite;     /**< Sprite data wrapped by this model. */
  QString tileset_id;             /**< Tileset id used for animations images. */

  std::map<QString, int, NaturalComparator>
      names_to_indexes;           /**< Index in the list of each animation.
                                   * The order is determined here. */

  QList<AnimationModel>
      animations;                 /**< All animations. */

  QItemSelectionModel
      selection_model;            /**< Animations currently selected. */
};

}

#endif
