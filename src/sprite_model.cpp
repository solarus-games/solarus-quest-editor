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
#include "editor_exception.h"
#include "quest.h"
#include "sprite_model.h"
#include "size.h"
#include "point.h"
#include "rectangle.h"
#include <QIcon>
#include <QFont>

using SpriteAnimationData = Solarus::SpriteAnimationData;
using SpriteAnimationDirectionData = Solarus::SpriteAnimationDirectionData;

/**
 * @brief Creates a sprite model.
 * @param quest The quest.
 * @param sprite_id Id of the sprite to manage.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
SpriteModel::SpriteModel(
    Quest& quest,
    const QString& sprite_id,
    QObject* parent) :
  QAbstractItemModel(parent),
  quest(quest),
  sprite_id(sprite_id),
  selection_model(this) {

  // Load the sprite data file.
  QString path = quest.get_sprite_path(sprite_id);

  if (!sprite.import_from_file(path.toStdString())) {
    throw EditorException(tr("Cannot open sprite '%1'").arg(path));
  }

  // Build the index map of animations.
  build_index_map();

  // Create animations and directions models.
  for (const auto& kvp : names_to_indexes) {
    const QString& animation_name = kvp.first;
    AnimationModel animation(animation_name);
    int num_dir = get_animation(animation_name).get_num_directions();
    for (int nb = 0; nb < num_dir; nb++) {
      animation.directions.append(DirectionModel(animation_name, nb));
    }
    animations.append(animation);
  }

  // use the first tileset of the quest
  QStringList tilesets =
      quest.get_resources().get_elements(ResourceType::TILESET);
  if (tilesets.size() > 0) {
    tileset_id = tilesets[0];
  }
}

/**
 * @brief Returns the quest.
 */
Quest& SpriteModel::get_quest() {
  return quest;
}

/**
 * @brief Returns the id of the sprite managed by this model.
 * @return The sprite id.
 */
QString SpriteModel::get_sprite_id() const {
  return sprite_id;
}

/**
 * @brief Returns the default animation name of the sprite.
 * @return The default animation name.
 */
QString SpriteModel::get_default_animation_name() const {
  return sprite.get_default_animation_name().c_str();
}

/**
 * @brief Changes the default animation name of the sprite.
 * @param default_animation_name The new default animation name.
 */
void SpriteModel::set_default_animation_name(
    const QString& default_animation_name) {

  QString old_default_animation_name = get_default_animation_name();

  if (default_animation_name == old_default_animation_name) {
    return;
  }

  sprite.set_default_animation_name(default_animation_name.toStdString());

  emit default_animation_changed(
        old_default_animation_name, default_animation_name);

  // Notify data change
  Index old_index(old_default_animation_name);
  Index new_index(default_animation_name);

  if (animation_exists(old_index)) {
    QModelIndex model_index = get_model_index(old_index);
    emit dataChanged(model_index, model_index);
  }

  if (animation_exists(new_index)) {
    QModelIndex model_index = get_model_index(new_index);
    emit dataChanged(model_index, model_index);
  }
}

/**
 * @brief Returns whether there exists an animation or a direction.
 * @param index Index of an animation or a direction.
 * @return \c true if there exists an animation or a direction with this index.
 */
bool SpriteModel::exists(const Index& index) const {

  return (index.is_direction_index() && direction_exists(index)) ||
         (index.is_animation_index() && animation_exists(index));
}

/**
 * @brief Returns the tileset id used for animations images.
 * @return The tileset id.
 */
QString SpriteModel::get_tileset_id() const {
  return tileset_id;
}

/**
 * @brief Changes the tileset id used for animations images.
 * @param tileset_id The new tileset id.
 */
void SpriteModel::set_tileset_id(const QString& tileset_id) {

  if (tileset_id == this->tileset_id) {
    return;
  }

  this->tileset_id = tileset_id;

  for (const auto& kvp: sprite.get_animations()) {
    if (kvp.second.src_image_is_tileset()) {
      set_animation_image_dirty(QString(kvp.first.c_str()));
    }
  }
}

/**
 * @brief Saves the sprite to its data file.
 * @throws EditorException If the file could not be saved.
 */
void SpriteModel::save() const {

  QString path = quest.get_sprite_path(sprite_id);

  if (!sprite.export_to_file(path.toStdString())) {
    throw EditorException(tr("Cannot save sprite '%1'").arg(path));
  }
}

/**
 * @brief Returns the number of columns in the model.
 * @param parent Parent index.
 * @return The number of columns
 */
int SpriteModel::columnCount(const QModelIndex& /* parent */) const {
  return 1;
}

/**
 * @brief Returns the number of animations or directions.
 * @param parent Parent index.
 * @return The number of animations or number of directions from parent.
 */
int SpriteModel::rowCount(const QModelIndex& parent) const {

  if (!parent.isValid()) {
    // in the root
    return animations.size();
  }

  Index* index = static_cast<Index*>(parent.internalPointer());
  if (index->is_animation_index()) {
    // in an animation
    return get_animation_num_directions(*index);
  }
  return 0;
}

/**
 * @brief Returns the index of an item.
 *
 * Reimplemented from QAbstractItemModel to create custom indexes for items.
 * Such items can represent an animation or a direction of an animation.
 *
 * @param row Row of the item.
 * @param column Column of the item.
 * @param parent Parent of the item.
 * @return The corresponding index. Returns an invalid index if there is no
 * such item.
 */
QModelIndex SpriteModel::index(int row, int column, const QModelIndex& parent) const {

  if (row < 0 || column != 0) {
    return QModelIndex();
  }

  if (!parent.isValid()) {
    // in the root
    if (row < animations.size()) {
      // return animation model index
      return createIndex(row, 0, animations[row].index.get());
    }
    return QModelIndex();
  }

  Index* index = static_cast<Index*>(parent.internalPointer());
  if (index->is_animation_index()) {
    // in an animation
    const AnimationModel& animation = animations[get_animation_nb(*index)];
    if (row < animation.directions.size()) {
      // return direction model index
      return createIndex(row, 0, animation.directions[row].index.get());
    }
  }

  return QModelIndex();
}

/**
 * @brief Returns the parent of the model item with the given index.
 * @param index Index to get the parent of.
 * @return The parent index, or an invalid index if the item has no parent.
 */
QModelIndex SpriteModel::parent(const QModelIndex& model_index) const {

  if (!model_index.isValid()) {
    return QModelIndex();
  }

  Index* index = static_cast<Index*>(model_index.internalPointer());
  if (index->is_direction_index()) {
    return this->index(get_animation_nb(*index), 0);
  }

  return QModelIndex();
}

/**
 * @brief Returns whether an item has any children.
 * @param parent The item to test.
 * @return @c true if this item has children.
 */
bool SpriteModel::hasChildren(const QModelIndex& parent) const {

  return rowCount(parent) > 0;
}

/**
 * @brief Returns the data of an item for a given role.
 * @param index Index of the item to get.
 * @param role The wanted role.
 * @return The data.
 */
QVariant SpriteModel::data(const QModelIndex& model_index, int role) const {

  if (!model_index.isValid()) {
    return QVariant();
  }

  Index* index = static_cast<Index*>(model_index.internalPointer());
  // animation
  if (index->is_animation_index()) {
      switch (role) {
        case Qt::DisplayRole: {
          QString name = index->animation_name;
          if (name == get_default_animation_name()) {
            name += tr(" (default)");
          }
          return name;
        } break;

        case Qt::DecorationRole:
          return QIcon(":/images/icon_folder_open.png");

        case Qt::FontRole: {
          if (index->animation_name == get_default_animation_name()) {
            QFont font;
            font.setBold(true);
            return font;
          }
        } break;
      }
  }
  // direction
  else switch (role) {
    case Qt::DisplayRole: {
      QString direction_nb = std::to_string(index->direction_nb).c_str();
      QString direction_name = "";
      if (animations[model_index.parent().row()].directions.size() == 4) {
        switch (index->direction_nb) {
        case 0: direction_name = tr("(right)"); break;
        case 1: direction_name = tr("(up)"); break;
        case 2: direction_name = tr("(left)"); break;
        case 3: direction_name = tr("(down)"); break;
        }
      }
      return tr("Direction %1 %2").arg(direction_nb, direction_name);
    } break;

    case Qt::DecorationRole:
      return get_direction_icon(*index);
  }

  return QVariant();
}

/**
 * @brief Returns the number of an animation with the specified index.
 * @param index An animation index.
 * @return The corresponding number.
 * Returns -1 there is no animation with this index.
 */
int SpriteModel::get_animation_nb(const Index& index) const {

  auto it = names_to_indexes.find(index.animation_name);
  if (it == names_to_indexes.end()) {
    return -1;
  }
  return it->second;
}

/**
 * @brief Returns the index of the animation at the specified number.
 * @param animation_nb An animation number.
 * @return The corresponding animation index.
 * Returns an empty index if there is no animation at this number.
 */
SpriteModel::Index SpriteModel::get_animation_index(int animation_nb) const {

  if (animation_nb < animations.size()) {
    return Index();
  }

  return *animations[animation_nb].index;
}

/**
 * @brief Returns whether there exists an animation at the specified index.
 * @param index An animation index.
 * @return @c true if such the specified animation exists in the sprite.
 */
bool SpriteModel::animation_exists(const Index& index) const {

  return get_animation_nb(index) != -1;
}

/**
 * @brief Creates a new empty animation in this sprite.
 *
 * The index of the selection may change, since animations are sorted
 * alphabetically.
 * Emits rowsAboutToBeInserted(), adds the new animation
 * and then emits rowsInserted(), as required by QAbstractItemModel.
 *
 * Then, emits animation_created().
 *
 * The newly created animation is not initially selected.
 * The existing selection is preserved, though the index of many
 * animations can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new index.
 *
 * @param animation_name Name of the animation to create.
 * @throws EditorException in case of error.
 */
void SpriteModel::create_animation(const QString& animation_name) {

  // Make some checks first.
  if (animation_name.length() <= 0) {
      throw EditorException(tr("Animation name cannot be empty"));
  }

  if (animation_exists(animation_name)) {
      throw EditorException(
            tr("Animation '%1' already exists").arg(animation_name));
  }

  // Save and clear the selection since a lot of indexes may change.
  Index selection = get_selected_index();
  clear_selection();

  // Add the animation to the sprite file.
  sprite.add_animation(animation_name.toStdString(), SpriteAnimationData());

  // Rebuild indexes in the list model (indexes were shifted).
  build_index_map();

  // Call beginInsertRows() as requested by QAbstractItemModel.
  int animation_nb = get_animation_nb(animation_name);
  beginInsertRows(QModelIndex(), animation_nb, animation_nb);

  // Update our animation model list.
  animations.insert(animation_nb, AnimationModel(animation_name));

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  endInsertRows();
  emit animation_created(Index(animation_name));

  // Restore the selection.
  set_selected_index(selection);
}

/**
 * @brief Deletes an animation.
 *
 * The index of the selection may change, since animations are sorted
 * alphabetically.
 * Emits rowsAboutToBeRemoved(), removes the animation
 * and then emits rowsRemoved(), as required by QAbstractItemModel.
 *
 * Then, emits animation_deleted().
 *
 * Except for the deleted animation, the existing selection is preserved,
 * though the index of many animations can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new index.
 *
 * @param index Index of the animation to delete.
 * @throws EditorException in case of error.
 */
void SpriteModel::delete_animation(const Index &index) {

  // Make some checks first.
  if (!animation_exists(index)) {
      throw EditorException(
            tr("Animation '%1' don't exists").arg(index.animation_name));
  }

  // Save and clear the selection since a lot of indexes may change.
  Index selection = get_selected_index();
  clear_selection();

  int animation_nb = get_animation_nb(index);

  // Delete the animation in the sprite file.
  sprite.remove_animation(index.animation_name.toStdString());

  // Rebuild indexes in the list model (indexes were shifted).
  build_index_map();

  // Call beginRemoveRows() as requested by QAbstractItemModel.
  beginRemoveRows(QModelIndex(), animation_nb, animation_nb);

  // Update our animation model list.
  animations.removeAt(animation_nb);

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  endRemoveRows();
  emit animation_deleted(index);

  // Restore the selection.
  set_selected_index(selection);
}

/**
 * @brief Changes the name of an animation.
 *
 * The index of the selection may change, since animations are sorted
 * alphabetically.
 * In this case, emits rowsAboutToBeMoved(), changes the index
 * and then emits rowsMoved(), as required by QAbstractItemModel.
 *
 * Then, emits animation_name_changed(), no matter if the index has also changed.
 *
 * The selection is preserved, though the index of many animations can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new index.
 *
 * @param index Index of an existing animation.
 * @param new_name The new name to set.
 * @throws EditorException in case of error.
 */
void SpriteModel::set_animation_name(const Index& index, const QString& new_name) {

  if (new_name == index.animation_name) {
    // Nothing to do.
    return;
  }

  // Make some checks first.
  if (!animation_exists(index)) {
    throw EditorException(
            tr("Animation '%1' don't exists").arg(index.animation_name));
  }

  if (new_name.length() <= 0) {
      throw EditorException(tr("Animation name cannot be empty"));
  }

  if (animation_exists(new_name)) {
      throw EditorException(tr("animation '%1' already exists").arg(new_name));
  }

  // Save and clear the selection since a lot of indexes may change.
  Index selection = get_selected_index();
  clear_selection();

  int animation_nb = get_animation_nb(index);

  // Change the name in the sprite file.
  sprite.set_animation_name(
        index.animation_name.toStdString(), new_name.toStdString());

  // Change the index in the list model (if the order has changed).
  build_index_map();
  int new_animation_nb = get_animation_nb(new_name);

  // Call beginMoveRows() if the index changes, as requested by
  // QAbstractItemModel.
  if (new_animation_nb != animation_nb) {
    int above_row = new_animation_nb;
    if (new_animation_nb > animation_nb) {
      ++above_row;
    }
    beginMoveRows(QModelIndex(), animation_nb, animation_nb,
                  QModelIndex(), above_row);

    // Update our animation model list.
    animations.move(animation_nb, new_animation_nb);
  }

  animations[new_animation_nb].set_animation_name(new_name);

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  if (new_animation_nb != animation_nb) {
    endMoveRows();
  }
  emit animation_name_changed(index, Index(new_name));

  // Restore the selection.
  if (selection.animation_name == index.animation_name) {
    selection.animation_name = new_name;
  }

  set_selected_index(selection);
}

/**
 * @brief Returns the source image of an animation.
 * @param index An animation index.
 * @return The animation's source image.
 */
QString SpriteModel::get_animation_source_image(const Index& index) const {

  if (!animation_exists(index)) {
    return "";
  }
  return get_animation(index).get_src_image().c_str();
}

/**
 * @brief Returns whether is the source image of an animation is a tileset.
 * @param index An animation index.
 * @return \c true if the animation's source image is a tileset.
 */
bool SpriteModel::is_animation_image_is_tileset(const Index& index) const {

  if (!animation_exists(index)) {
    return false;
  }
  return get_animation(index).src_image_is_tileset();
}

/**
 * @brief Sets the source image of an animation.
 *
 * Emits animation_image_changed() if there is a change.
 *
 * @param index An animation index.
 * @param src_image The source image to set.
 */
void SpriteModel::set_animation_source_image(
    const Index& index, const QString& src_image) {

  if (!animation_exists(index)) {
    return;
  }

  SpriteAnimationData& animation = get_animation(index);
  if (animation.get_src_image() == src_image.toStdString()) {
    return;
  }

  animation.set_src_image(src_image.toStdString());
  set_animation_image_dirty(index);

  emit animation_image_changed(index, src_image);
}

/**
 * @brief Returns the frame delay of an animation.
 * @param index An animation index.
 * @return The animation's frame delay.
 */
uint32_t SpriteModel::get_animation_frame_delay(const Index& index) const {

  if (!animation_exists(index)) {
    return 0;
  }
  return get_animation(index).get_frame_delay();
}

/**
 * @brief Sets the frame delay of an animation.
 *
 * Emits animation_frame_delay_changed() if there is a change.
 *
 * @param index An animation index.
 * @param frame_delay The frame delay to set.
 */
void SpriteModel::set_animation_frame_delay(
    const Index& index, uint32_t frame_delay) {

  if (!animation_exists(index)) {
    return;
  }

  SpriteAnimationData& animation = get_animation(index);
  if (animation.get_frame_delay() == frame_delay) {
    return;
  }

  animation.set_frame_delay(frame_delay);
  emit animation_frame_delay_changed(index, frame_delay);
}

/**
 * @brief Returns the loop on frame of an animation.
 * @param index An animation index.
 * @return The animation's loop on frame.
 */
int SpriteModel::get_animation_loop_on_frame(const Index& index) const {

  if (!animation_exists(index)) {
    return -1;
  }
  return get_animation(index).get_loop_on_frame();
}

/**
 * @brief Sets the loop on frame of an animation.
 *
 * Emits animation_loop_on_frame_changed() if there is a change.
 *
 * @param index An animation index.
 * @param loop_on_frame The loop on frame to set.
 */
void SpriteModel::set_animation_loop_on_frame(
    const Index& index, int loop_on_frame) {

  if (!animation_exists(index)) {
    return;
  }

  SpriteAnimationData& animation = get_animation(index);
  if (animation.get_loop_on_frame() == loop_on_frame) {
    return;
  }

  animation.set_loop_on_frame(loop_on_frame);
  emit animation_loop_on_frame_changed(index, loop_on_frame);
}

/**
 * @brief Returns the number of directions in an animation.
 * @param index An animation index.
 * @return The animation's number of directions.
 */
int SpriteModel::get_animation_num_directions(const Index& index) const {

  if (!animation_exists(index)) {
    return 0;
  }
  return get_animation(index).get_num_directions();
}

/**
 * @brief Returns whether there exists a direction at the specified index.
 * @param index A direction index.
 * @return @c true if such the specified direction exists in the sprite.
 */
bool SpriteModel::direction_exists(const Index& index) const {

  if (!index.is_direction_index()) {
    return false;
  }
  return index.direction_nb < get_animation_num_directions(index);
}

/**
 * @brief Adds a direction in an animation of this sprite.
 * @param index Index of the animation to add the direction.
 * @param frame The first frame of the direction to create.
 * @return Index of the created direction.
 * @throws EditorException in case of error.
 */
int SpriteModel::add_direction(const Index& index, const QRect& frame) {

  // Make some checks first.
  if (!animation_exists(index)) {
    throw EditorException(
            tr("Animation '%1' don't exists").arg(index.animation_name));
  }

  // Add the direction to the sprite file.
  SpriteAnimationData& animation_data = get_animation(index);
  SpriteAnimationDirectionData direction(
        Point::to_solarus_point(frame.topLeft()),
        Size::to_solarus_size(frame.size()));

  animation_data.add_direction(direction);

  // Rebuild indexes in the list model (indexes were shifted).
  int direction_nb = animation_data.get_num_directions() - 1;

  // Call beginInsertRows() as requested by QAbstractItemModel.
  QModelIndex model_index = get_model_index(Index(index.animation_name));
  beginInsertRows(model_index, direction_nb, direction_nb);

  // Update our animation model list.
  int animation_nb = get_animation_nb(index);
  animations[animation_nb].directions.append(
        DirectionModel(index.animation_name, direction_nb));

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  endInsertRows();
  emit direction_added(Index(index.animation_name, direction_nb));

  return direction_nb;
}

/**
 * @brief Inserts a direction in an animation of this sprite.
 * @param index Index of the direction to insert.
 * @param data The direction data to insert.
 * @return Index of the inserted direction.
 * @throws EditorException in case of error.
 */
int SpriteModel::insert_direction(
    const Index& index, const SpriteAnimationDirectionData& data) {

  // Make some checks first.
  if (!animation_exists(index)) {
    throw EditorException(
            tr("Animation '%1' don't exists").arg(index.animation_name));
  }

  // Save and clear the selection.
  Index selection = get_selected_index();
  clear_selection();

  // Insert the direction to the sprite file.
  SpriteAnimationData& animation_data = get_animation(index);

  animation_data.add_direction(data);

  int last_dir = animation_data.get_num_directions() - 1;
  animation_data.move_direction(last_dir, index.direction_nb);

  int direction_nb = std::min(index.direction_nb, last_dir);

  // Call beginInsertRows() as requested by QAbstractItemModel.
  QModelIndex model_index = get_model_index(Index(index.animation_name));
  beginInsertRows(model_index, direction_nb, direction_nb);

  // Update our animation model list.
  int animation_nb = get_animation_nb(index);
  auto& directions = animations[animation_nb].directions;
  directions.insert(
        direction_nb, DirectionModel(index.animation_name, direction_nb));

  // Update direction model indexes.
  for (int nb = direction_nb + 1; nb < directions.size(); nb++) {
    directions[nb].index->direction_nb = nb;
  }

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  endInsertRows();
  emit direction_added(Index(index.animation_name, direction_nb));

  // Restore the selection.
  set_selected_index(selection);

  return direction_nb;
}

/**
 * @brief Deletes a direction.
 * @param index Index of the direction to delete.
 * @throws EditorException in case of error.
 */
void SpriteModel::delete_direction(const Index &index) {

  // Make some checks first.
  if (!direction_exists(index)) {
      QString nb = std::to_string(index.direction_nb).c_str();
      throw EditorException(
            tr("Direction %1 don't exists in animation '%2'").arg(
              nb, index.animation_name));
  }

  // Save and clear the selection.
  Index selection = get_selected_index();
  if (selection.animation_name == index.animation_name &&
      selection.direction_nb == index.direction_nb) {
    selection.direction_nb = -1;
  }
  clear_selection();

  // Delete the direction in the sprite file.
  get_animation(index).remove_direction(index.direction_nb);

  // Call beginRemoveRows() as requested by QAbstractItemModel.
  QModelIndex model_index = get_model_index(Index(index.animation_name));
  beginRemoveRows(model_index, index.direction_nb, index.direction_nb);

  // Update our direction model list.
  int animation_nb = get_animation_nb(index);
  auto& directions = animations[animation_nb].directions;
  directions.removeAt(index.direction_nb);

  // Update direction model indexes.
  for (int nb = index.direction_nb; nb < directions.size(); nb++) {
    directions[nb].index->direction_nb = nb;
  }

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  endRemoveRows();
  emit direction_deleted(index);

  // Restore the selection.
  set_selected_index(selection);
}

/**
 * @brief Moves a direction.
 * @param index Index of the direction to move.
 * @param new_direction_nb The new number of the direction.
 * @throws EditorException in case of error.
 */
void SpriteModel::move_direction(const Index& index, int new_direction_nb) {

  if (new_direction_nb == index.direction_nb) {
    // Nothing to do.
    return;
  }

  // Make some checks first.
  if (!direction_exists(index)) {
      QString nb = std::to_string(index.direction_nb).c_str();
      throw EditorException(
            tr("Direction %1 don't exists in animation '%2'").arg(
              nb, index.animation_name));
  }

  // Save and clear the selection.
  Index selection = get_selected_index();
  clear_selection();

  // Move the direction in the sprite file.
  get_animation(index).move_direction(index.direction_nb, new_direction_nb);

  // Call beginMoveRows() as requested by QAbstractItemModel.
  int above_row = new_direction_nb;
  if (new_direction_nb > index.direction_nb) {
    ++above_row;
  }
  QModelIndex model_index = get_model_index(Index(index.animation_name));
  beginMoveRows(model_index, index.direction_nb, index.direction_nb,
                model_index, above_row);

  // Update our animation model list.
  int animation_nb = get_animation_nb(index);
  animations[animation_nb].directions.move(index.direction_nb, new_direction_nb);

  // Update direction model indexes.
  int num_dir = animations[animation_nb].directions.size();
  for (int nb = 0; nb < num_dir; nb++) {
    animations[animation_nb].directions[nb].index->direction_nb = nb;
  }

  endMoveRows();

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  emit direction_deleted(index);

  // Restore the selection.
  if (selection.direction_nb == index.direction_nb) {
    selection.direction_nb = new_direction_nb;
  }

  set_selected_index(selection);
}

/**
 * @brief Returns a direction data.
 * @param index A direction index.
 * @return The direction data.
 */
SpriteAnimationDirectionData SpriteModel::get_direction_data(
    const Index& index) const {

  if (!direction_exists(index)) {
    return SpriteAnimationDirectionData();
  }
  return get_direction(index);
}

/**
 * @brief Returns the first frame of a direction.
 * @param index A direction index.
 * @return The direction's first frame.
 */
QRect SpriteModel::get_direction_frame_rect(const Index& index) const {

  if (!direction_exists(index)) {
    return QRect();
  }
  return Rectangle::to_qrect(get_direction(index).get_frame());
}

/**
 * @brief Returns all frames of a direction.
 * @param index A direction index.
 * @return The direction's frames.
 */
QList<QRect> SpriteModel::get_direction_frames(const Index& index) const {

  QList<QRect> list;

  if (!direction_exists(index)) {
    return list;
  }

  for (const auto& rect: get_direction(index).get_all_frames()) {
    list.append(Rectangle::to_qrect(rect));
  }
  return list;
}

/**
 * @brief Returns the position of a direction.
 * @param index A direction index.
 * @return The direction's position.
 */
QPoint SpriteModel::get_direction_position(const Index& index) const {

  if (!direction_exists(index)) {
    return QPoint();
  }
  return Point::to_qpoint(get_direction(index).get_xy());
}

/**
 * @brief Sets the position of a direction.
 *
 * Emits direction_position_changed() if there is a change.
 *
 * @param index A direction index.
 * @param position The position to set.
 */
void SpriteModel::set_direction_position(
    const Index& index, const QPoint& position) {

  if (!direction_exists(index)) {
    return;
  }

  SpriteAnimationDirectionData& direction = get_direction(index);

  if (Point::to_qpoint(direction.get_xy()) == position) {
    return;
  }

  direction.set_xy(Point::to_solarus_point(position));
  set_direction_image_dirty(index);
  emit direction_position_changed(index, position);
}

/**
 * @brief Returns the size of a direction.
 * @param index A direction index.
 * @return The direction's size.
 */
QSize SpriteModel::get_direction_size(const Index& index) const {

  if (!direction_exists(index)) {
    return QSize();
  }
  return Size::to_qsize(get_direction(index).get_size());
}

/**
 * @brief Sets the size of a direction.
 *
 * Emits direction_size_changed() if there is a change.
 *
 * @param index A direction index.
 * @param size The size to set.
 */
void SpriteModel::set_direction_size(const Index& index, const QSize& size) {

  if (!direction_exists(index)) {
    return;
  }

  SpriteAnimationDirectionData& direction = get_direction(index);

  if (Size::to_qsize(direction.get_size()) == size) {
    return;
  }

  direction.set_size(Size::to_solarus_size(size));
  set_direction_image_dirty(index);
  emit direction_size_changed(index, size);
}

/**
 * @brief Returns the origin point of frames of a direction.
 * @param index A direction index.
 * @return The frames direction's origin point.
 */
QPoint SpriteModel::get_direction_origin(const Index& index) const {

  if (!direction_exists(index)) {
    return QPoint();
  }
  return Point::to_qpoint(get_direction(index).get_origin());
}

/**
 * @brief Sets the origin point of frames of a direction.
 *
 * Emits direction_origin_changed() if there is a change.
 *
 * @param index A direction index.
 * @param origin The origin point to set.
 */
void SpriteModel::set_direction_origin(
    const Index& index, const QPoint& origin) {

  if (!direction_exists(index)) {
    return;
  }

  SpriteAnimationDirectionData& direction = get_direction(index);

  if (Point::to_qpoint(direction.get_origin()) == origin) {
    return;
  }

  direction.set_origin(Point::to_solarus_point(origin));
  emit direction_origin_changed(index, origin);
}

/**
 * @brief Returns whether a direction is multi frames.
 * @param index A direction index.
 * @return \c true if The direction is multi frames.
 */
bool SpriteModel::is_direction_multi_frame(const Index& index) const {
  return get_direction_num_frames(index) > 0;
}

/**
 * @brief Returns the number frames of a direction.
 * @param index A direction index.
 * @return The direction's number frames.
 */
int SpriteModel::get_direction_num_frames(const Index& index) const {

  if (!direction_exists(index)) {
    return 1;
  }
  return get_direction(index).get_num_frames();
}

/**
 * @brief Sets the number frames of a direction.
 *
 * Emits direction_num_frames_changed() if there is a change.
 *
 * @param index A direction index.
 * @param num_frames The number frames to set.
 */
void SpriteModel::set_direction_num_frames(const Index& index, int num_frames) {

  if (!direction_exists(index)) {
    return;
  }

  SpriteAnimationDirectionData& direction = get_direction(index);

  if (direction.get_num_frames() == num_frames) {
    return;
  }

  direction.set_num_frames(num_frames);
  set_direction_image_dirty(index);
  emit direction_num_frames_changed(index, num_frames);
}

/**
 * @brief Returns the number columns of a direction.
 * @param index A direction index.
 * @return The direction's number columns.
 */
int SpriteModel::get_direction_num_columns(const Index& index) const {

  if (!direction_exists(index)) {
    return 1;
  }
  return get_direction(index).get_num_columns();
}

/**
 * @brief Sets the number of columns of a direction.
 *
 * Emits direction_num_columns_changed() if there is a change.
 *
 * @param index A direction index.
 * @param num_frames The number of columns to set.
 */
void SpriteModel::set_direction_num_columns(
    const Index& index, int num_columns) {

  if (!direction_exists(index)) {
    return;
  }

  SpriteAnimationDirectionData& direction = get_direction(index);

  if (direction.get_num_columns() == num_columns) {
    return;
  }

  direction.set_num_columns(num_columns);
  set_direction_image_dirty(index);
  emit direction_num_columns_changed(index, num_columns);
}

/**
 * @brief Returns the image of an animation.
 * @param index An animation index.
 * @return The corresponding image.
 * Returns a null pixmap if the image is not loaded.
 */
QImage SpriteModel::get_animation_image(const Index& index) const {

  if (!animation_exists(index)) {
    // No such animation.
    return QImage();
  }

  const AnimationModel& animation = animations[get_animation_nb(index)];

  if (animation.image.isNull()) {
    // Lazily load image.
    if (is_animation_image_is_tileset(index)) {
      animation.image = QImage(quest.get_tileset_entities_image_path(tileset_id));
    } else {
      QString src_image = get_animation_source_image(index);
      animation.image = QImage(quest.get_sprite_image_path(src_image));
    }
  }

  return animation.image;
}

/**
 * @brief Returns alls images representing frames of a specified direction.
 * @param index A direction index.
 * @return The corresponding images.
 * Returns an empty list if the animation image is not loaded.
 */
QList<QPixmap> SpriteModel::get_direction_all_frames(const Index& index) const {

  if (!direction_exists(index)) {
    // No such direction.
    return QList<QPixmap>();
  }

  const AnimationModel& animation = animations[get_animation_nb(index)];
  const DirectionModel& direction = animation.directions[index.direction_nb];

  if (direction.frames.size() > 0) {
    // Frames already created.
    return direction.frames;
  }

  QImage image = get_animation_image(index);
  if (image.isNull()) {
    // No image.
    return QList<QPixmap>();
  }

  // Lazily create frames.
  std::string name = index.animation_name.toStdString();
  const auto& data = sprite.get_animation(name).get_direction(index.direction_nb);

  for (const auto& rect: data.get_all_frames()) {
    QRect r(rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height());
    direction.frames.append(QPixmap::fromImage(image.copy(r)));
  }
  return direction.frames;
}

/**
 * @brief Returns an image representing the first frame of a specified direction.
 * @param index A direction index.
 * @return The corresponding images.
 * Returns a null pixmap if the animation image is not loaded.
 */
QPixmap SpriteModel::get_direction_frame(const Index& index) const {

  QList<QPixmap> frames = get_direction_all_frames(index);

  if (frames.size() > 0) {
    return frames[0];
  }

  return QPixmap();
}

/**
 * @brief Returns a 32x32 icon representing the specified direction.
 * @param index A direction index.
 * @return The corresponding icon.
 * Returns a null pixmap if the animation image is not loaded.
 */
QPixmap SpriteModel::get_direction_icon(const Index& index) const {

  QPixmap pixmap = get_direction_frame(index);

  if (pixmap.isNull()) {
    // No image available.
    return QPixmap();
  }

  int animation_nb = get_animation_nb(index);
  const AnimationModel& animation = animations[animation_nb];
  const DirectionModel& direction = animation.directions[index.direction_nb];

  if (!direction.icon.isNull()) {
    // Icon already created.
    return direction.icon;
  }

  // Lazily create the icon.
  QImage image = pixmap.toImage();
  // Make sure we have an alpha channel.
  image = image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);

  if (image.height() <= 16) {
    image = image.scaledToHeight(image.height() * 2);
  }
  else if (image.height() > 32) {
    image = image.scaledToHeight(32);
  }

  // Center the pattern in a 32x32 pixmap.
  int dx = (32 - image.width()) / 2;
  int dy = (32 - image.height()) / 2;
  image = image.copy(-dx, -dy, 32, 32);

  direction.icon = QPixmap::fromImage(image);
  return direction.icon;
}

/**
 * @brief Returns the selection model of the sprite.
 * @return The selection info.
 */
QItemSelectionModel& SpriteModel::get_selection_model() {
  return selection_model;
}

/**
 * @brief Returns whether no animation are selected.
 * @return @c true if the selection is empty.
 */
bool SpriteModel::is_selection_empty() const {

  return selection_model.selection().isEmpty();
}

/**
 * @brief Returns the index of the selection.
 * @return The selected index.
 */
SpriteModel::Index SpriteModel::get_selected_index() const {

  QModelIndexList selected_indexes = selection_model.selectedIndexes();
  if (selected_indexes.size() != 1) {
    return Index();
  }

  QModelIndex model_index = selected_indexes.first();
  Index* index = static_cast<Index*>(model_index.internalPointer());
  return *index;
}

/**
 * @brief Selects a specified index.
 * @param index The index to select.
 */
void SpriteModel::set_selected_index(const Index& index) {

  if (!index.is_valid() || !exists(index)) {
    selection_model.clear();
    return;
  }

  QModelIndex model_index = get_model_index(index);
  selection_model.select(model_index, QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Selects an animation at the specified index.
 *
 * If the selection is a direction of the specified animation,
 * the selection remains unchanged.
 *
 * @param index The index of animation to select.
 */
void SpriteModel::set_selected_animation(const Index& index) {

  if (!index.is_valid()) {
    selection_model.clear();
    return;
  }

  QString selected_name = get_selected_index().animation_name;
  if (index.is_direction_index() && index.animation_name == selected_name) {
    return;
  }

  // select the animation
  set_selected_index(index.animation_name);
}

/**
 * @brief Deselects all selected items.
 */
void SpriteModel::clear_selection() {

  selection_model.clear();
}

/**
 * @brief Builds or rebuilds the internal mapping that gives indexes from names.
 *
 * Sprite animations are indexed by string names, but the model also treats them
 * as a linear list, so we need an additional integer index.
 */
void SpriteModel::build_index_map() {

  names_to_indexes.clear();

  // This is a bit tricky because we change the order of
  // the map from the Solarus library to use natural order instead.

  const std::map<std::string, SpriteAnimationData>& animation_map =
      sprite.get_animations();
  // First, put the string keys to have natural sort.
  for (const auto& kvp : animation_map) {
    QString animation_name = QString::fromStdString(kvp.first);
    names_to_indexes.insert(std::make_pair(animation_name, 0));
  }

  // Then, we can put the integer value.
  int index = 0;
  for (const auto& kvp : names_to_indexes) {
    QString animation_name = kvp.first;
    names_to_indexes[animation_name] = index;
    ++index;
  }
}

/**
 * @brief Returns a model index corresponding to a specified index.
 * @param index Index of an animation or a direction.
 * @return The corresponding model index or invalid model index if index no exists.
 */
QModelIndex SpriteModel::get_model_index(const Index& index) const {

  if (!exists(index)) {
    return QModelIndex();
  }

  // get model index of the animation
  int animation_nb = get_animation_nb(index);
  QModelIndex model_index = this->index(animation_nb, 0);

  if (index.is_direction_index()) {
    // return the direction model index
    return this->index(index.direction_nb, 0, model_index);
  }

  return model_index;
}

/**
 * @brief Clears the image cache of an animation and notify views.
 *
 * The animation must be exists (use animation_exists() method to check it).
 *
 * @param index Index of an animation.
 */
void SpriteModel::set_animation_image_dirty(const Index& index) {

  animations[get_animation_nb(index)].set_image_dirty();

  int num_dir = get_animation_num_directions(index);
  QString name = index.animation_name;

  QModelIndex first_model_index = get_model_index(Index(name, 0));
  QModelIndex last_model_index = get_model_index(Index(name, num_dir));
  emit dataChanged(first_model_index, last_model_index);
}

/**
 * @brief Clears the image cache of a direction and notify views.
 *
 * The direction must be exists (use direction_exists() method to check it).
 *
 * @param index Index of a direction.
 */
void SpriteModel::set_direction_image_dirty(const Index& index) {

  int animation_nb = get_animation_nb(index);
  animations[animation_nb].directions[index.direction_nb].set_image_dirty();

  QModelIndex model_index = get_model_index(index);
  emit dataChanged(model_index, model_index);
}

/**
 * @brief Returns the reference of an animation of the sprite.
 *
 * The animation must be exists (use animation_exists() method to check it).
 *
 * @param index An animation index.
 * @return Reference of the animation.
 */
const Solarus::SpriteAnimationData &SpriteModel::get_animation(
    const Index& index) const {

  return sprite.get_animation(index.animation_name.toStdString());
}

/**
 * @brief Returns the reference of an animation of the sprite.
 *
 * Non-const version.
 *
 * The animation must be exists (use animation_exists() method to check it).
 *
 * @param index An animation index.
 * @return Reference of the animation.
 */
Solarus::SpriteAnimationData &SpriteModel::get_animation(const Index& index) {

  return sprite.get_animation(index.animation_name.toStdString());
}

/**
 * @brief Returns the reference of a direction of the sprite.
 *
 * The direction must be exists (use direction_exists() method to check it).
 *
 * @param index A direction index.
 * @return Reference of the direction.
 */
const Solarus::SpriteAnimationDirectionData &SpriteModel::get_direction(
    const Index& index) const {

  return get_animation(index).get_direction(index.direction_nb);
}

/**
 * @brief Returns the reference of a direction of the sprite.
 *
 * Non-const version.
 *
 * The direction must be exists (use direction_exists() method to check it).
 *
 * @param index A direction index.
 * @return Reference of the direction.
 */
Solarus::SpriteAnimationDirectionData &SpriteModel::get_direction(
    const Index& index) {

  return get_animation(index).get_direction(index.direction_nb);
}
