/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_ENTITY_MODEL_H
#define SOLARUSEDITOR_ENTITY_MODEL_H

#include "entity_traits.h"
#include "layer_traits.h"
#include "resize_mode.h"
#include "sprite_model.h"
#include <QPointer>

class MapModel;
class Quest;
class QuestResources;
class TilesetModel;

using SubtypeList = QList<QPair<QString, QString>>;

/**
 * @brief Model of a map entity.
 *
 * This class wraps an entity from the Solarus library and
 * adds all useful information about how to represent and modify it in the
 * editor.
 * An EntityModel can represent an entity on the map or an entity that is not
 * on the map yet.
 *
 * Each type of entity is a subclass of EntityModel.
 */
class EntityModel {

public:

  static EntityModelPtr create(
      MapModel& map, EntityType type);
  static EntityModelPtr create(
      MapModel& map, const QString& entity_string);
  static EntityModelPtr create(
      MapModel& map, const EntityIndex& index);
  static EntityModelPtr clone(
      MapModel& map, const EntityIndex& index);

  virtual ~EntityModel() = default;

  const MapModel& get_map() const;
  MapModel& get_map();
  QString get_tileset_id() const;
  const TilesetModel* get_tileset() const;
  const Quest& get_quest() const;
  const QuestResources& get_resources() const;

  // Index on the map.
  EntityIndex get_index() const;
  bool is_on_map() const;
  void added_to_map(const EntityIndex& index);
  void about_to_be_removed_from_map();
  void index_changed(const EntityIndex& index);

  EntityType get_type() const;
  QString get_type_name() const;
  bool is_dynamic() const;

  const Solarus::EntityData& get_entity() const;
  Solarus::EntityData& get_entity();

  // Access data.
  bool has_name() const;
  QString get_name() const;
  void set_name(const QString& name);
  void ensure_name_unique();
  Layer get_layer() const;
  void set_layer(Layer layer);
  QPoint get_xy() const;
  void set_xy(const QPoint& xy);
  QPoint get_top_left() const;
  void set_top_left(const QPoint& top_left);
  QPoint get_bottom_right() const;
  void set_bottom_right(const QPoint& bottom_right);
  QPoint get_center() const;
  void set_center(const QPoint& center);
  QPoint get_origin() const;
  void set_origin(const QPoint& origin);
  bool has_size_fields() const;
  int get_width() const;
  void set_width(int width);
  int get_height() const;
  void set_height(int height);
  QSize get_size() const;
  void set_size(const QSize& size);
  QRect get_bounding_box() const;
  bool get_has_preferred_layer() const;
  Layer get_preferred_layer() const;
  bool has_direction_field() const;
  bool is_no_direction_allowed() const;
  QString get_no_direction_text() const;
  int get_num_directions() const;
  int get_direction() const;
  void set_direction(int direction);
  bool has_subtype_field() const;
  SubtypeList get_existing_subtypes() const;
  QString get_subtype() const;
  void set_subtype(const QString& subtype);
  bool has_field(const QString& key) const;
  bool is_field_optional(const QString& key) const;
  bool is_field_unset(const QString& key) const;
  QVariant get_field(const QString& key) const;
  void set_field(const QString& key, const QVariant& value);
  QString to_string() const;

  // Resizing from the editor.
  bool is_resizable() const;
  ResizeMode get_resize_mode() const;
  QSize get_base_size() const;
  bool is_size_valid() const;
  virtual bool is_size_valid(const QSize& size) const;
  virtual QSize get_valid_size() const;

  // Displaying in the editor.
  virtual void draw(QPainter& painter) const;
  virtual void notify_tileset_changed(const QString& tileset_id);

protected:

  /**
   * @brief Describes how to draw an entity as a sprite.
   */
  struct DrawSpriteInfo {

    bool enabled = true;  // false means not drawn as a sprite.
    QString sprite_id;    // Only used if there is no "sprite" field.
    QString animation;    // Animation for sprite_id (empty means default).
    int frame = 0;        // Index of the frame to show. If negative,
                          // we count from the end (-1 is the last frame).
    bool tiled = false;   // Tiled or only once at origin point.
  };

  /**
   * @brief Describes how to draw an entity as a shape.
   *
   * The shape is filled with a background color or a pixmap,
   * or both if the pixmap is not tiled but centered,
   * and may have a border with two black lines and a color between them.
   * For now the shape is always rectangular.
   */
  struct DrawShapeInfo {

    bool enabled = false;  // false means not drawn as a shape.
    QColor background_color;
    QColor between_border_color;
    QPixmap pixmap;
    bool tiled_pixmap = false;  // Tiled or centered.
  };

  /**
   * @brief A rectangle region of an image file.
   */
  struct SubImage {
    QString file_name;
    QRect src_rect;  // An invalid rect means the whole image.
    mutable QPixmap pixmap;
  };

  /**
   * @brief Describes how to draw an entity as a fixed image.
   *
   * If nothing is specified, the image will be a generic icon for the entity
   * type.
   */
  struct DrawImageInfo {
    SubImage image_no_direction;
    QList<SubImage> images_by_direction;
    double scale = 1.0;  // If 2.0, the image will have a resolution twice better.
  };

  EntityModel(MapModel& map, const EntityIndex& index, EntityType type);

  void set_resizable(bool resizable);
  void set_resize_mode(ResizeMode resize_mode);
  void set_base_size(const QSize& base_size);

  void set_has_preferred_layer(bool has_preferred_layer);
  void set_preferred_layer(Layer preferred_layer);

  void set_num_directions(int num_directions);
  void set_no_direction_allowed(bool no_direction_allowed);
  void set_no_direction_text(const QString& no_direction_text);

  void set_existing_subtypes(const SubtypeList& subtypes);

  virtual void notify_field_changed(const QString& key, const QVariant& value);
  virtual void set_initial_values();

  const DrawSpriteInfo& get_draw_sprite_info() const;
  void set_draw_sprite_info(const DrawSpriteInfo& draw_sprite_info);
  const DrawShapeInfo& get_draw_shape_info() const;
  void set_draw_shape_info(const DrawShapeInfo& draw_shape_info);
  const DrawImageInfo& get_draw_image_info() const;
  void set_draw_image_info(const DrawImageInfo& draw_shape_info);

  bool draw_as_sprite(QPainter& painter) const;
  bool draw_as_sprite(QPainter& painter,
                      const QString& sprite_id,
                      const QString& animation,
                      int frame) const;
  bool draw_as_shape(QPainter& painter) const;
  bool draw_as_image(QPainter& painter) const;
  bool draw_as_image(QPainter& painter, const SubImage& image) const;
  bool draw_as_icon(QPainter& painter) const;

private:

  static EntityModelPtr create(
      MapModel& map, const EntityIndex& index, EntityType type);
  void set_entity(const Solarus::EntityData& entity);

  QPointer<MapModel> map;         /**< The map this entity belongs to
                                   * (could be a reference but we want operator=). */
  EntityIndex index;              /**< Index of this entity in the map.
                                   * When invalid, the entity is not added to the map yet. */
  Solarus::EntityData stub;       /**< Stub of entity, used before it gets added to the map. */
  QString name;                   /**< Name of the entity. */
  QPoint origin;                  /**< Origin point of the entity relative to its top-left corner. */
  QSize size;                     /**< Size of the entity for the editor. */
  QSize base_size;                /**< Reference size when resizing. */
  ResizeMode resize_mode;         /**< How the entity can be resized. */
  bool has_preferred_layer;       /**< Whether the entity has a preferred layer when added to the map. */
  Layer preferred_layer;          /**< The preferred layer if has_preferred_layer is true. */
  int num_directions;             /**< Number of possible directions (except the possible special one -1). */
  bool no_direction_allowed;      /**< Whether the special no-value -1 is an allowed direction. */
  QString no_direction_text;      /**< The text to show in a GUI for the special no-value -1 (if allowed). */
  SubtypeList subtypes;           /**< Existing subtypes of this entity type. */

  // Displaying.
  DrawSpriteInfo
      draw_sprite_info;           /**< How to draw the entity
                                   * when it is drawn as a sprite. */
  mutable std::unique_ptr<SpriteModel>
      sprite_model;               /**< Sprite to show when the entity is drawn
                                   * as a sprite. */
  mutable QPixmap sprite_image;   /**< Fixed image from the sprite. */
  DrawShapeInfo draw_shape_info;  /**< Shape to use when the entity is drawn as
                                   * a shape. */
  DrawImageInfo draw_image_info;  /**< Subimage to use when the entity is
                                   * drawn as a fixed image from a file. */
  mutable QPixmap icon;           /**< Icon to use when the entity is drawn as
                                   * an icon. */
};

#endif
