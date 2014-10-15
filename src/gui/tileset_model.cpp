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
#include "gui/tileset_model.h"
#include "color.h"
#include "editor_exception.h"
#include "quest.h"
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
  tileset() {

  // Load the tileset data file.
  QString path = quest.get_tileset_data_file_path(tileset_id);

  if (!tileset.import_from_file(path.toStdString())) {
    throw EditorException(tr("Cannot open tileset data file '%1'").arg(path));
  }
  build_index_map();

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
 * @return
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
    return get_pattern_icon(get_pattern_id(index.row()));
    break;

  default:
    break;
  }

  return QVariant();
}

/**
 * @brief Creates internal mappings that gives fast access to patterns ids
 * and indexes.
 *
 * The tile patterns are indexed by string ids, but the model also treats them
 * as a linear list, so we need an additional integer index.
 * boost::multi_index_container could do that for use, but this feels a bit
 * overkill to add a boost dependency just for this use case.
 */
void TilesetModel::build_index_map() {

  indexes_to_ids.clear();
  ids_to_indexes.clear();

  // This is a bit tricky because we change the order of
  // by the map from the Solarus library to use natural order instead.

  const std::map<std::string, TilePatternData>& patterns =
      tileset.get_patterns();
  // First, put the string keys to have natural sort.
  for (const auto& kvp : patterns) {
    QString pattern_id = QString::fromStdString(kvp.first);
    ids_to_indexes.insert(std::make_pair(pattern_id, 0));
  }

  // Then, put the integer value and fill the second structure.
  int index = 0;
  for (const auto& kvp : ids_to_indexes) {
    QString pattern_id = kvp.first;
    indexes_to_ids.append(pattern_id);
    ids_to_indexes[pattern_id] = index;
    ++index;
  }
}

/**
 * @brief Returns an image representing the specified pattern.
 *
 * The image has the size of the pattern.
 *
 * @param pattern_id Id of a tile pattern.
 * @return The corresponding image.
 * Returns a null pixmap if the pattern does not exists or if the
 * tileset image is not loaded.
 */
QPixmap TilesetModel::get_pattern_image(const QString& pattern_id) const {

  if (!pattern_exists(pattern_id)) {
    // No such pattern.
    return QPixmap();
  }

  if (patterns_image.isNull()) {
    // No tileset image.
    return QPixmap();
  }

  QPixmap pixmap = patterns_images.value(pattern_id, QPixmap());
  if (!pixmap.isNull()) {
    // Image already created.
    return pixmap;
  }

  // Lazily create the image.
  QRect frame = get_pattern_frame(pattern_id);
  QImage image = patterns_image.copy(frame);

  pixmap = QPixmap::fromImage(image);
  patterns_images.insert(pattern_id, pixmap);
  return pixmap;
}

/**
 * @brief Returns a 32x32 icon representing the specified pattern.
 * @param pattern_id Id of a tile pattern.
 * @return The corresponding icon.
 * Returns a null pixmap if the pattern does not exists or if the
 * tileset image is not loaded.
 */
QPixmap TilesetModel::get_pattern_icon(const QString& pattern_id) const {

  QPixmap pixmap = get_pattern_image(pattern_id);

  if (pixmap.isNull()) {
    // No image available.
    return QPixmap();
  }

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

  QPixmap icon = QPixmap::fromImage(image);
  patterns_icons.insert(pattern_id, icon);
  return icon;
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
 * @brief Returns the number of patterns in the tileset.
 * @return The number of patterns.
 */
int TilesetModel::get_num_patterns() const {

  return tileset.get_num_patterns();
}

/**
 * @brief Returns whether there exists a pattern with the specified id.
 * @param pattern_id A pattern id.
 * @return @c true if such a pattern exists in the tileset.
 */
bool TilesetModel::pattern_exists(const QString& pattern_id) const {

  return tileset.exists(pattern_id.toStdString());
}

/**
 * @brief Returns the coordinates of a pattern's frame in the tileset image.
 * @param pattern_id A pattern id.
 * @return The pattern's frame.
 * If this is a multi-frame pattern, the first frame is returned.
 * Returns a null rectangle if the pattern does not exist.
 */
QRect TilesetModel::get_pattern_frame(const QString& pattern_id) const {

  if (!pattern_exists(pattern_id)) {
    return QRect();
  }

  const Solarus::Rectangle& frame = tileset.get_pattern(pattern_id.toStdString()).get_frame();
  return QRect(frame.get_x(), frame.get_y(), frame.get_width(), frame.get_height());
}

/**
 * @brief Returns all patterns' frame coordinates in the tileset image.
 * @return The frame of each pattern, indexed by the pattern id.
 * For multi-frame patterns, only the first frame is returned.
 */
QMap<QString, QRect> TilesetModel::get_patterns_frame() const {

  QMap<QString, QRect> patterns_frame;

  for (const auto& kvp : tileset.get_patterns()) {
    const QString& pattern_id = QString::fromStdString(kvp.first);
    const Solarus::Rectangle& frame = kvp.second.get_frame();
    patterns_frame.insert(pattern_id, QRect(frame.get_x(), frame.get_y(), frame.get_width(), frame.get_height()));
  }

  return patterns_frame;
}

/**
 * @brief Returns the list index of the specified pattern.
 * @param id Id of a tile pattern
 * @return The corresponding index in the list.
 * Returns -1 there is no pattern with this id.
 */
int TilesetModel::get_pattern_index(const QString& id) const {

  auto it = ids_to_indexes.find(id);
  if (it == ids_to_indexes.end()) {
    return -1;
  }
  return it->second;
}

/**
 * @brief Returns the id of the pattern at the specified place.
 * @param index An index in the list of patterns.
 * @return The corresponding pattern id.
 * Returns an empty string if there is no pattern at this index.
 */
QString TilesetModel::get_pattern_id(int pattern_index) const {

  if (pattern_index < 0 || pattern_index >= indexes_to_ids.size()) {
    return "";
  }

  return indexes_to_ids.at(pattern_index);
}
