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
#include "color.h"
#include "editor_exception.h"
#include "quest.h"
#include "tile_pattern_animation_traits.h"
#include "tileset_model.h"
#include <QIcon>

using TilePatternData = Solarus::TilePatternData;

/**
 * @brief Creates a tileset model.
 * @param quest The quest.
 * @param tileset_id Id of the tileset to manage.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
TilesetModel::TilesetModel(
    Quest& quest,
    const QString& tileset_id,
    QObject* parent) :
  QAbstractListModel(parent),
  quest(quest),
  tileset_id(tileset_id),
  selection(this) {

  // Load the tileset data file.
  QString path = quest.get_tileset_data_file_path(tileset_id);

  if (!tileset.import_from_file(path.toStdString())) {
    throw EditorException(tr("Cannot open tileset data file '%1'").arg(path));
  }

  build_index_map();
  for (const auto& kvp : ids_to_indexes) {
    const QString& pattern_id = kvp.first;
    patterns.append(PatternModel(pattern_id));
  }

  // Load the tileset image.
  patterns_image = QImage(quest.get_tileset_tiles_image_path(tileset_id));
}

/**
 * @brief Returns the quest.
 */
Quest& TilesetModel::get_quest() {
  return quest;
}

/**
 * @brief Returns the id of the tileset managed by this model.
 * @return The tileset id.
 */
QString TilesetModel::get_tileset_id() const {
  return tileset_id;
}

/**
 * @brief Saves the tileset to its data file.
 * @throws EditorException If the file could not be saved.
 */
void TilesetModel::save() const {

  QString path = quest.get_tileset_data_file_path(tileset_id);

  if (!tileset.export_to_file(path.toStdString())) {
    throw EditorException(tr("Cannot save tileset data file '%1'").arg(path));
  }
}

/**
 * @brief Returns the tileset's background color.
 * @return The background color.
 */
QColor TilesetModel::get_background_color() const {

  return Color::to_qcolor(tileset.get_background_color());
}

/**
 * @brief Sets the tileset's background color.
 *
 * Emits background_color_changed if there is a change.
 *
 * @param background_color The background color.
 */
void TilesetModel::set_background_color(const QColor& background_color) {

  const Solarus::Color solarus_color = Color::to_solarus_color(background_color);
  if (solarus_color == tileset.get_background_color()) {
    return;
  }
  tileset.set_background_color(solarus_color);
  emit background_color_changed(background_color);
}

/**
 * @brief Returns the number of items in the list.
 *
 * This is the number of patterns of the tileset.
 *
 * @return The number of patterns.
 */
int TilesetModel::rowCount(const QModelIndex& /* parent */) const {
  return tileset.get_num_patterns();
}

/**
 * @brief Returns the datat of an item for a given role.
 * @param index Index of the item to get.
 * @param role The wanted role.
 * @return The data.
 */
QVariant TilesetModel::data(const QModelIndex& index, int role) const {

  switch (role) {

  case Qt::DisplayRole:
    return QVariant();  // No text: only the icon.
    break;

  case Qt::DecorationRole:
    // Show an icon representing the pattern.
    return get_pattern_icon(index.row());
    break;

  default:
    break;
  }

  return QVariant();
}

/**
 * @brief Returns the number of patterns in the tileset.
 * @return The number of patterns.
 */
int TilesetModel::get_num_patterns() const {

  return tileset.get_num_patterns();
}

/**
 * @brief Returns whether there exists a pattern with the specified id.
 * @param index A pattern index.
 * @return @c true if such a pattern exists in the tileset.
 */
bool TilesetModel::pattern_exists(int index) const {

  return index >= 0 && index < patterns.size();
}

/**
 * @brief Returns the list index of the specified pattern.
 * @param pattern_id Id of a tile pattern
 * @return The corresponding index in the list.
 * Returns -1 there is no pattern with this id.
 */
int TilesetModel::id_to_index(const QString& pattern_id) const {

  auto it = ids_to_indexes.find(pattern_id);
  if (it == ids_to_indexes.end()) {
    return -1;
  }
  return it->second;
}

/**
 * @brief Returns the string id of the pattern at the specified list index.
 * @param index An index in the list of patterns.
 * @return The corresponding pattern id.
 * Returns an empty string if there is no pattern at this index.
 */
QString TilesetModel::index_to_id(int index) const {

  if (!pattern_exists(index)) {
    return "";
  }

  return patterns.at(index).id;
}

/**
 * @brief Builds or rebuilds the internal mapping that gives indexes from ids.
 *
 * Tile patterns are indexed by string ids, but the model also treats them
 * as a linear list, so we need an additional integer index.
 */
void TilesetModel::build_index_map() {

  // boost::multi_index_container could do that for us, but this feels a bit
  // overkill to add a boost dependency just for this use case.

  ids_to_indexes.clear();

  // This is a bit tricky because we change the order of
  // the map from the Solarus library to use natural order instead.

  const std::map<std::string, TilePatternData>& pattern_map =
      tileset.get_patterns();
  // First, put the string keys to have natural sort.
  for (const auto& kvp : pattern_map) {
    QString pattern_id = QString::fromStdString(kvp.first);
    ids_to_indexes.insert(std::make_pair(pattern_id, 0));
  }

  // Then, we can put the integer value.
  int index = 0;
  for (const auto& kvp : ids_to_indexes) {
    QString pattern_id = kvp.first;
    ids_to_indexes[pattern_id] = index;
    ++index;
  }
}

/**
 * @brief Creates a new pattern in this tileset with default properties.
 *
 * The index of multiple patterns in the pattern list may change, since
 * patterns are sorted alphabetically.
 * Emits rowsAboutToBeInserted(), adds the new pattern
 * and then emits rowsInserted(), as required by QAbstractItemModel.
 *
 * Then, emits pattern_created().
 *
 * The newly created pattern is not initially selected.
 * The existing selection is preserved, though the index of many patterns can
 * change.
 * The selection is cleared before the operations and restored after,
 * updated with the new indexes.
 *
 * @param pattern_id Id of the pattern to create.
 * @param frame Position of the pattern in the tileset image
 * (it will be a single-frame pattern).
 * @return Index of the created pattern.
 * @throws EditorException in case of error.
 */
int TilesetModel::create_pattern(const QString& pattern_id, const QRect& frame) {

  // Make some checks first.
  if (!is_valid_pattern_id(pattern_id)) {
      throw EditorException(tr("Invalid tile pattern id: '%1'").arg(pattern_id));
  }

  if (id_to_index(pattern_id) != -1) {
      throw EditorException(tr("Tile pattern '%1' already exists").arg(pattern_id));
  }

  // Save and clear the selection since a lot of indexes may change.
  QModelIndexList old_selected_indexes = selection.selection().indexes();
  QStringList old_selection_ids;
  for (const QModelIndex& old_selected_index : old_selected_indexes) {
    old_selection_ids << index_to_id(old_selected_index.row());
  }
  selection.clear();

  // Add the pattern to the tileset file.
  TilePatternData pattern(Solarus::Rectangle(frame.x(), frame.y(), frame.width(), frame.height()));
  tileset.add_pattern(pattern_id.toStdString(), pattern);

  // Rebuild indexes in the list model (indexes were shifted).
  build_index_map();
  int index = id_to_index(pattern_id);

  // Call beginInsertRows() as requested by QAbstractItemModel.
  beginInsertRows(QModelIndex(), index, index);

  // Update our pattern model list.
  patterns.insert(index, PatternModel(pattern_id));

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  endInsertRows();
  emit pattern_created(index, pattern_id);

  // Restore the selection.
  for (QString selected_pattern_id : old_selection_ids) {
    int new_index = id_to_index(selected_pattern_id);
    selection.select(this->index(new_index), QItemSelectionModel::Select);
  }

  return index;
}

/**
 * @brief Deletes a tile pattern.
 *
 * The index of multiple patterns in the pattern list may change, since
 * patterns are sorted alphabetically.
 * Emits rowsAboutToBeRemoved(), removes the pattern
 * and then emits rowsRemoved(), as required by QAbstractItemModel.
 *
 * Then, emits pattern_deleted().
 *
 * Except for the deleted pattern, the existing selection is preserved,
 * though the index of many patterns can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new indexes.
 *
 * @param index Index of the pattern to delete.
 * @throws EditorException in case of error.
 */
void TilesetModel::delete_pattern(int index) {

  QString pattern_id = index_to_id(index);

  // Make some checks first.
  if (pattern_id.isEmpty()) {
      throw EditorException(tr("Invalid tile pattern index: %1").arg(index));
  }

  // Save and clear the selection since a lot of indexes may change.
  QModelIndexList old_selected_indexes = selection.selection().indexes();
  QStringList old_selection_ids;
  for (const QModelIndex& old_selected_index : old_selected_indexes) {
    old_selection_ids << index_to_id(old_selected_index.row());
  }
  selection.clear();

  // Delete the pattern in the tileset file.
  tileset.remove_pattern(pattern_id.toStdString());

  // Rebuild indexes in the list model (indexes were shifted).
  build_index_map();

  // Call beginRemoveRows() as requested by QAbstractItemModel.
  beginRemoveRows(QModelIndex(), index, index);

  // Update our pattern model list.
  patterns.removeAt(index);

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  endRemoveRows();
  emit pattern_deleted(index, pattern_id);

  // Restore the selection.
  for (QString selected_pattern_id : old_selection_ids) {

    if (selected_pattern_id == pattern_id) {
      // Exclude the deleted one.
      continue;
    }

    int new_index = id_to_index(selected_pattern_id);
    selection.select(this->index(new_index), QItemSelectionModel::Select);
  }
}

/**
 * @brief Changes the string id of a pattern.
 *
 * The index of multiple patterns in the pattern list may change, since
 * patterns are sorted alphabetically.
 * In this case, emits rowsAboutToBeMoved(), changes the index
 * and then emits rowsMoved(), as required by QAbstractItemModel.
 *
 * Then, emits pattern_id_changed(), no matter if the index has also changed.
 *
 * The selection is preserved, though the index of many patterns can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new indexes.
 *
 * @param index Index of an existing pattern.
 * @param new_id The new id to set.
 * @return The new index of the pattern.
 * @throws EditorException in case of error.
 */
int TilesetModel::set_pattern_id(int index, const QString& new_id) {

  QString old_id = index_to_id(index);
  if (new_id == old_id) {
    // Nothing to do.
    return index;
  }

  // Make some checks first.
  if (old_id.isEmpty()) {
      throw EditorException(tr("Invalid tile pattern index: %1").arg(index));
  }

  if (!is_valid_pattern_id(new_id)) {
      throw EditorException(tr("Invalid tile pattern id: '%1'").arg(new_id));
  }

  if (id_to_index(new_id) != -1) {
      throw EditorException(tr("Tile pattern '%1' already exists").arg(new_id));
  }

  // Save and clear the selection since a lot of indexes may change.
  QModelIndexList old_selected_indexes = selection.selection().indexes();
  QStringList old_selection_ids;
  for (const QModelIndex& old_selected_index : old_selected_indexes) {
    old_selection_ids << index_to_id(old_selected_index.row());
  }
  selection.clear();

  // Change the id in the tileset file.
  tileset.set_pattern_id(old_id.toStdString(), new_id.toStdString());

  // Change the index in the list model (if the order has changed).
  build_index_map();
  int new_index = id_to_index(new_id);

  // Call beginMoveRows() if the index changes, as requested by
  // QAbstractItemModel.
  if (new_index != index) {
    int above_row = new_index;
    if (new_index > index) {
      ++above_row;
    }
    beginMoveRows(QModelIndex(), index, index,
                  QModelIndex(), above_row);

    // Update our pattern model list.
    patterns.move(index, new_index);
  }

  patterns[new_index].id = new_id;

  // Notify people before restoring the selection, so that they have a
  // chance to know new indexes before receiving selection signals.
  if (new_index != index) {
    endMoveRows();
  }
  emit pattern_id_changed(index, old_id, new_index, new_id);

  // Restore the selection.
  for (QString pattern_id : old_selection_ids) {
    if (pattern_id == old_id) {
      pattern_id = new_id;
    }
    int new_index = id_to_index(pattern_id);
    selection.select(this->index(new_index), QItemSelectionModel::Select);
  }

  return new_index;
}

/**
 * @brief Returns whether a string is a valid tile pattern id.
 * @param pattern_id The id to check.
 * @return @c true if this id is legal.
 */
bool TilesetModel::is_valid_pattern_id(const QString& pattern_id) {

  if (pattern_id.isEmpty()) {
      return false;
  }

  if (
      pattern_id.contains('\"') ||
      pattern_id.contains('\'') ||
      pattern_id.contains('\\') ||
      pattern_id.contains('\n') ||
      pattern_id.contains('\r')
      ) {
    return false;
  }

  return true;
}

/**
 * @brief Returns whether a pattern is a multi-frame pattern.
 * @param index A pattern index.
 * @return @c true if this is a multi-frame pattern.
 */
bool TilesetModel::is_pattern_multi_frame(int index) const {

  const std::string& pattern_id = index_to_id(index).toStdString();
  return tileset.get_pattern(pattern_id).is_multi_frame();
}

/**
 * @brief Returns the number of frames of a pattern.
 * @param index A pattern index.
 * @return The number of frames in the tileset (1, 3 or 4).
 */
int TilesetModel::get_pattern_num_frames(int index) const {

  const std::string& pattern_id = index_to_id(index).toStdString();
  return tileset.get_pattern(pattern_id).get_num_frames();
}

/**
 * @brief Returns the coordinates of a pattern's frame in the tileset image.
 * @param index A pattern index.
 * @return The pattern's frame.
 * If this is a multi-frame pattern, the first frame is returned.
 */
QRect TilesetModel::get_pattern_frame(int index) const {

  const std::string& pattern_id = index_to_id(index).toStdString();
  const Solarus::Rectangle& frame = tileset.get_pattern(pattern_id).get_frame();
  return QRect(frame.get_x(), frame.get_y(), frame.get_width(), frame.get_height());
}

/**
 * @brief Returns the coordinates of all frames of a pattern in the tileset
 * image.
 * @param index A pattern index.
 * @return The pattern's frames.
 */
QList<QRect> TilesetModel::get_pattern_frames(int index) const {

  const std::string& pattern_id = index_to_id(index).toStdString();
  const std::vector<Solarus::Rectangle>& frames =
      tileset.get_pattern(pattern_id).get_frames();

  QList<QRect> result;
  for (const Solarus::Rectangle& frame : frames) {
    result << QRect(frame.get_x(), frame.get_y(), frame.get_width(), frame.get_height());
  }
  return result;
}

/**
 * @brief Returns the coordinates of the rectangle containing all frames of a
 * pattern in the tileset image.
 *
 * For single-frame patterns, this gives the same result as get_pattern_frame().
 *
 * @param index A pattern index.
 * @return The bounding box of the pattern's frames.
 */
QRect TilesetModel::get_pattern_frames_bounding_box(int index) const {

  QRect bounding_box = get_pattern_frame(index);
  if (!is_pattern_multi_frame(index)) {
    return bounding_box;
  }

  if (get_pattern_separation(index) == TilePatternSeparation::HORIZONTAL) {
    bounding_box.setWidth(bounding_box.width() * 3);
  }
  else {
    bounding_box.setHeight(bounding_box.height() * 3);
  }
  return bounding_box;
}

/**
 * @brief Returns the ground of a pattern.
 * @param index A pattern index.
 * @return The pattern's ground.
 */
Ground TilesetModel::get_pattern_ground(int index) const {

  const std::string& pattern_id = index_to_id(index).toStdString();
  return tileset.get_pattern(pattern_id).get_ground();
}

/**
 * @brief Sets the ground of a tile pattern.
 *
 * Emits pattern_ground_changed() if there is a change.
 *
 * @param index A pattern index.
 * @param ground The ground to set.
 */
void TilesetModel::set_pattern_ground(int index, Ground ground) {

  Solarus::TilePatternData& pattern = tileset.get_pattern(index_to_id(index).toStdString());
  if (ground == pattern.get_ground()) {
    return;
  }
  pattern.set_ground(ground);
  emit pattern_ground_changed(index, ground);
}

/**
 * @brief Returns the default layer of a pattern.
 * @param index A pattern index.
 * @return The pattern's default layer.
 */
Layer TilesetModel::get_pattern_default_layer(int index) const {

  const std::string& pattern_id = index_to_id(index).toStdString();
  return tileset.get_pattern(pattern_id).get_default_layer();
}

/**
 * @brief Sets the default layer of a tile pattern.
 *
 * Emits pattern_default_layer_changed() if there is a change.
 *
 * @param index A pattern index.
 * @param default_layer The default layer to set.
 */
void TilesetModel::set_pattern_default_layer(int index, Layer default_layer) {

  Solarus::TilePatternData& pattern = tileset.get_pattern(index_to_id(index).toStdString());
  if (default_layer == pattern.get_default_layer()) {
    return;
  }
  pattern.set_default_layer(default_layer);
  emit pattern_default_layer_changed(index, default_layer);
}

/**
 * @brief Returns the animation property of a pattern.
 * @param index A pattern index.
 * @return The pattern's animation.
 */
TilePatternAnimation TilesetModel::get_pattern_animation(int index) const {

  const std::string& pattern_id = index_to_id(index).toStdString();
  const Solarus::TilePatternData& pattern = tileset.get_pattern(pattern_id);

  switch (pattern.get_scrolling()) {

  case Solarus::TileScrolling::NONE:
    if (!pattern.is_multi_frame()) {
      // No scrolling, single-frame.
      return TilePatternAnimation::NONE;
    }
    // No scrolling, multi-frame.
    if (pattern.get_num_frames() == 3) {
      return TilePatternAnimation::SEQUENCE_012;
    }
    return TilePatternAnimation::SEQUENCE_0121;

  case Solarus::TileScrolling::PARALLAX:
    // Parallax scrolling, single-frame.
    if (!pattern.is_multi_frame()) {
      return TilePatternAnimation::PARALLAX_SCROLLING;
    }
    // Parallax scrolling, multi-frame.
    if (pattern.get_num_frames() == 3) {
      return TilePatternAnimation::SEQUENCE_012_PARALLAX;
    }
    return TilePatternAnimation::SEQUENCE_0121_PARALLAX;

  case Solarus::TileScrolling::SELF:
    // Scrolling on itself (single-frame only).
    return TilePatternAnimation::SELF_SCROLLING;

  }

  return TilePatternAnimation();
}

/**
 * @brief Sets the animation property of a pattern.
 *
 * Emits pattern_animation_changed() if there is a change.
 *
 * If the new animation makes multi-frame a pattern that was single-frame,
 * then the existing frame is splitted in 3 parts.
 * The width or height must therefore be divisible by 3*8.
 * (In patterns that have 4 frames, frame 1 and frame 3 are identical
 * so multi-frame patterns actually always have 3 distinct frames.)
 *
 * If the new animation makes single-frame a pattern that was multi-frame,
 * then the existing 3 frames are merged into a single big frame.
 *
 * @param index A pattern index.
 * @return The pattern's animation.
 * @throws EditorException in case of error.
 */
void TilesetModel::set_pattern_animation(int index, TilePatternAnimation animation) {

  TilePatternAnimation old_animation = get_pattern_animation(index);
  if (animation == old_animation) {
    return;
  }

  const std::string& pattern_id = index_to_id(index).toStdString();
  Solarus::TilePatternData& pattern = tileset.get_pattern(pattern_id);

  // Set the scrolling.
  pattern.set_scrolling(TilePatternAnimationTraits::get_scrolling(animation));

  // Set the frames.
  const int old_num_frames = TilePatternAnimationTraits::get_num_frames(old_animation);
  const int num_frames = TilePatternAnimationTraits::get_num_frames(animation);

  if (old_num_frames > 1 &&
      num_frames == 1) {
    // Multi-frame to single-frame: merge the 3 frames into one.
    TilePatternSeparation separation = get_pattern_separation(index);
    Solarus::Rectangle frame = pattern.get_frame();  // Get the first frame.
    if (separation == TilePatternSeparation::HORIZONTAL) {
      frame.set_width(frame.get_width() * 3);
    }
    else {
      frame.set_height(frame.get_height() * 3);
    }
    pattern.set_frame(frame);
  }
  else if (old_num_frames == 1 &&
           num_frames > 1) {
    // Single-frame to multi-frame: split the pattern in 3 frames.
    const Solarus::Rectangle& initial_frame = pattern.get_frame();
    int width = initial_frame.get_width();
    int height = initial_frame.get_height();

    TilePatternSeparation separation = TilePatternSeparation::HORIZONTAL;
    if (width % 24 == 0) {
      if (height % 24 == 0) {
        // Divisible both horizontally or vertically.
        separation = width >= height ?
              TilePatternSeparation::HORIZONTAL :
              TilePatternSeparation::VERTICAL;
      }
      else {
        // Only divisible horizontally.
        separation = TilePatternSeparation::HORIZONTAL;
      }
    }
    else if (height % 24 == 0) {
      // Only divisible vertically.
      separation = TilePatternSeparation::VERTICAL;
    }
    else {
      // This pattern is not divisible.
      throw EditorException(tr("Cannot divide the pattern in 3 frames : "
                               "the size of each frame must be a multiple of 8 pixels"));
    }

    std::vector<Solarus::Rectangle> frames;
    if (separation == TilePatternSeparation::HORIZONTAL) {
      width = width / 3;
      for (int i = 0; i < 3; ++i) {
        frames.emplace_back(
              initial_frame.get_x() + i * width,
              initial_frame.get_y(),
              width,
              height
              );
      }
    }
    else {
      height = height / 3;
      for (int i = 0; i < 3; ++i) {
        frames.emplace_back(
              initial_frame.get_x(),
              initial_frame.get_y() + i * height,
              width,
              height
              );
      }
    }
    pattern.set_frames(frames);
  }

  // Set 3 or 4 frames for multi-frame patterns.
  if (num_frames > 1 && num_frames != old_num_frames) {
    std::vector<Solarus::Rectangle> frames = pattern.get_frames();
    if (num_frames == 4 && frames.size() == 3) {
      // Sequence 0-1-2-1: get back to frame 1 after frame 2.
      frames.emplace_back(frames[1]);
    }
    else if (num_frames == 3 && frames.size() == 4) {
      // Sequence 0-1-2: get back to frame 0 after frame 2.
      frames.resize(3);
    }
    pattern.set_frames(frames);
  }

  emit pattern_animation_changed(index, animation);
}

/**
 * @brief Returns the separation of the frames if the pattern is multi-frame.
 * @return The type of separation of the frames.
 * Returns TilePatternSeparation::HORIZONTAL if the pattern is single-frame.
 */
TilePatternSeparation TilesetModel::get_pattern_separation(int index) const {

  const std::string& pattern_id = index_to_id(index).toStdString();
  const Solarus::TilePatternData& pattern = tileset.get_pattern(pattern_id);

  const std::vector<Solarus::Rectangle>& frames = pattern.get_frames();
  if (frames.size() == 1) {
    return TilePatternSeparation::HORIZONTAL;
  }

  if (frames[0].get_y() == frames[1].get_y()) {
    return TilePatternSeparation::HORIZONTAL;
  }
  return TilePatternSeparation::VERTICAL;
}

/**
 * Sets the type of separation of the frames if the tile pattern is multi-frame.
 *
 * Emits pattern_separation_changed() if there is a change.
 *
 * Nothing is done if the tile pattern is single-frame.
 *
 * @param separation The type of separation of the frames.
 * @throws EditorException If the separation is not valid, i.e. if the size of
 * each frame after separation is not divisible by 8.
 */
void TilesetModel::set_pattern_separation(int index, TilePatternSeparation separation) {

  const std::string& pattern_id = index_to_id(index).toStdString();
  Solarus::TilePatternData& pattern = tileset.get_pattern(pattern_id);

  if (!pattern.is_multi_frame()) {
    // Nothing to do.
    return;
  }

  TilePatternSeparation old_separation = get_pattern_separation(index);
  if (separation == old_separation) {
    // No change.
    return;
  }

  Solarus::Rectangle first_frame = pattern.get_frame();
  int width = first_frame.get_width();
  int height = first_frame.get_height();
  std::vector<Solarus::Rectangle> frames;
  if (separation == TilePatternSeparation::HORIZONTAL) {
    // Vertical to horizontal separation.
    if (width % 24 != 0) {
      throw EditorException(tr("Cannot divide the pattern in 3 frames : "
                               "the size of each frame must be a multiple of 8 pixels"));
    }
    height *= 3;
    width /= 3;
    first_frame.set_width(width);
    first_frame.set_height(height);

    for (int i = 0; i < 3; ++i) {
      frames.emplace_back(
            first_frame.get_x() + i * width,
            first_frame.get_y(),
            width,
            height
            );
    }
  }
  else {
    // Horizontal to vertical separation.
    if (height % 24 != 0) {
      throw EditorException(tr("Cannot divide the pattern in 3 frames : "
                               "the size of each frame must be a multiple of 8 pixels"));
    }
    height /= 3;
    width *= 3;
    first_frame.set_width(width);
    first_frame.set_height(height);

    for (int i = 0; i < 3; ++i) {
      frames.emplace_back(
            first_frame.get_x(),
            first_frame.get_y() + i * height,
            width,
            height
            );
    }
  }
  const int num_frames = TilePatternAnimationTraits::get_num_frames(get_pattern_animation(index));
  if (num_frames == 4) {
    // Sequence 0-1-2-1: get back to frame 1 after frame 2.
    frames.emplace_back(frames[1]);
  }
  pattern.set_frames(frames);

  emit pattern_separation_changed(index, separation);
}

/**
 * @brief Returns an image representing the specified pattern.
 *
 * The image has the size of the pattern.
 * If the pattern is multi-frame, the image of the first frame is returned.
 *
 * @param index Index of a tile pattern.
 * @return The corresponding image.
 * Returns a null pixmap if the tileset image is not loaded.
 */
QPixmap TilesetModel::get_pattern_image(int index) const {

  if (!pattern_exists(index)) {
    // No such pattern.
    return QPixmap();
  }

  if (patterns_image.isNull()) {
    // No tileset image.
    return QPixmap();
  }

  const PatternModel& pattern = patterns.at(index);
  if (!pattern.image.isNull()) {
    // Image already created.
    return pattern.image;
  }

  // Lazily create the image.
  QRect frame = get_pattern_frame(index);
  QImage image = patterns_image.copy(frame);

  pattern.image = QPixmap::fromImage(image);
  return pattern.image;
}

/**
 * @brief Returns an image representing the specified pattern.
 *
 * If the pattern is multi-frame, the image returned contains all frames.
 *
 * @param index Index of a tile pattern.
 * @return The corresponding image with all frames.
 * Returns a null pixmap if the tileset image is not loaded.
 */
QPixmap TilesetModel::get_pattern_image_all_frames(int index) const {

  if (!pattern_exists(index)) {
    // No such pattern.
    return QPixmap();
  }

  if (!is_pattern_multi_frame(index)) {
    // Single frame pattern.
    return get_pattern_image(index);
  }

  if (patterns_image.isNull()) {
    // No tileset image.
    return QPixmap();
  }

  const PatternModel& pattern = patterns.at(index);
  if (!pattern.image_all_frames.isNull()) {
    // Image already created.
    return pattern.image_all_frames;
  }

  // Lazily create the image.
  QRect frame = get_pattern_frames_bounding_box(index);
  QImage image_all_frames = patterns_image.copy(frame);

  pattern.image_all_frames = QPixmap::fromImage(image_all_frames);
  return pattern.image_all_frames;
}

/**
 * @brief Returns a 32x32 icon representing the specified pattern.
 * @param index Index of a tile pattern.
 * @return The corresponding icon.
 * Returns a null pixmap if the tileset image is not loaded.
 */
QPixmap TilesetModel::get_pattern_icon(int index) const {

  QPixmap pixmap = get_pattern_image(index);

  if (pixmap.isNull()) {
    // No image available.
    return QPixmap();
  }

  const PatternModel& pattern = patterns.at(index);
  if (!pattern.icon.isNull()) {
    // Icon already created.
    return pattern.icon;
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

  pattern.icon = QPixmap::fromImage(image);
  return pattern.icon;
}

/**
 * @brief Returns the PNG image of all tile patterns.
 * @return The patterns image.
 * Returns a null image if the tileset image is not loaded.
 */
QImage TilesetModel::get_patterns_image() const {
  return patterns_image;
}

/**
 * @brief Returns the selection model of the tileset.
 * @return The selection info.
 */
QItemSelectionModel& TilesetModel::get_selection() {
  return selection;
}

/**
 * @brief Returns the index of the selected pattern.
 * @return The selected pattern index.
 * Returns -1 if no pattern is selected or if multiple patterns are selected.
 */
int TilesetModel::get_selected_index() const {

  QModelIndexList selected_indexes = selection.selectedIndexes();
  if (selected_indexes.size() != 1) {
    return -1;
  }
  return selected_indexes.first().row();
}

/**
 * @brief Returns all selected pattern indexes.
 * @return The selected pattern indexes.
 */
QList<int> TilesetModel::get_selected_indexes() const {

  QList<int> result;
  for (const QModelIndex& index : selection.selectedIndexes()) {
    result << index.row();
  }
  return result;
}

/**
 * @brief Returns whether a pattern is selected.
 * @param index A pattern index.
 * @return @c true if this pattern is selected.
 */
bool TilesetModel::is_selected(int index) const {

  return selection.isSelected(this->index(index));
}
