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
#include "widgets/gui_tools.h"
#include "widgets/tileset_scene.h"
#include "quest.h"
#include "tileset_model.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPalette>
#include <QStyleOptionGraphicsItem>
#include <memory>

namespace SolarusEditor {

/**
 * @brief Graphic item representing a tile pattern.
 */
class PatternItem : public QGraphicsPixmapItem {

public:

  PatternItem(TilesetModel& model, int index);

  // Enable the use of qgraphicsitem_cast with this item.
  enum {
    Type = UserType + 1
  };

  virtual int type() const override {
    return Type;
  }

  int get_index() const;
  void set_index(int index);

  void rebuild_pixmap();

protected:

  virtual void paint(QPainter* painter,
                     const QStyleOptionGraphicsItem* option,
                     QWidget* widget = nullptr) override;

private:

  TilesetModel& model;            /**< The tileset this pattern belongs to. */
  int index;                      /**< Index of the pattern in the tileset. */

};

/**
 * @brief Creates a tileset scene.
 * @param model The tileset data to represent in the scene.
 * @param parent The parent object or nullptr.
 */
TilesetScene::TilesetScene(TilesetModel& model, QObject* parent) :
  QGraphicsScene(parent),
  model(model) {

  build();

  // Synchronize the scene selection with the tileset selection model.
  connect(&model.get_selection_model(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_selection_to_scene(QItemSelection, QItemSelection)));
  connect(this, SIGNAL(selectionChanged()),
          this, SLOT(set_selection_from_scene()));

  // Watch pattern geometry changes.
  connect(&model, SIGNAL(pattern_position_changed(int, QPoint)),
          this, SLOT(update_pattern_position(int)));
  connect(&model, SIGNAL(pattern_animation_changed(int, PatternAnimation)),
          this, SLOT(update_pattern_animation(int)));
  connect(&model, SIGNAL(pattern_separation_changed(int, PatternSeparation)),
          this, SLOT(update_pattern_animation(int)));

  // Watch changes in the pattern list.
  connect(&model, SIGNAL(pattern_created(int, QString)),
          this, SLOT(pattern_created(int, QString)));
  connect(&model, SIGNAL(pattern_deleted(int, QString)),
          this, SLOT(pattern_deleted(int, QString)));
  connect(&model, SIGNAL(pattern_id_changed(int, QString, int, QString)),
          this, SLOT(pattern_id_changed(int, QString, int, QString)));
  connect(&model, SIGNAL(image_changed()),
          this, SLOT(image_changed()));
}

/**
 * @brief Returns the tileset represented in the scene.
 * @return The tileset.
 */
const TilesetModel& TilesetScene::get_model() const {
  return model;
}

/**
 * @brief Returns the quest the tileset belongs to.
 * @return The quest.
 */
const Quest& TilesetScene::get_quest() const {
  return model.get_quest();
}

/**
 * @brief Returns the index of the pattern represented by an item.
 * @param item A graphic item.
 * @return The pattern index, or -1 if this is not a pattern item of the scene.
 */
int TilesetScene::get_pattern_index(const QGraphicsItem* item) {

  const PatternItem* pattern_item = qgraphicsitem_cast<const PatternItem*>(item);
  if (pattern_item == nullptr) {
    return -1;
  }

  return pattern_item->get_index();
}

/**
 * @brief Draws the tileset image as background.
 * @param painter The painter.
 * @param rect The exposed rectangle in scene coordinates.
 * It may be larger than the scene.
 */
void TilesetScene::drawBackground(QPainter* painter, const QRectF& rect) {

  // Draw the background color.
  painter->fillRect(rect, backgroundBrush());

  // Draw the full PNG image of the tileset.
  const QImage& patterns_image = model.get_patterns_image();
  if (!patterns_image.isNull()) {
    painter->drawImage(0, 0, patterns_image);
  }
}

/**
 * @brief Create all patterns items in the scene.
 */
void TilesetScene::build() {

  clear();
  pattern_items.clear();

  if (model.get_patterns_image().isNull()) {
    // The tileset image does not exist yet.
    // Maybe this is a recently created tileset.
    QString path = get_quest().get_tileset_tiles_image_path(model.get_tileset_id());
    path = path.right(path.length() - get_quest().get_data_path().length() - 1);
    addText(tr("Missing tileset image '%1'").arg(path));
    return;
  }

  setSceneRect(QRectF(QPoint(0, 0), model.get_patterns_image().size()));
  for (int i = 0; i < model.get_num_patterns(); ++i) {

    // TODO make a function to create an item.
    PatternItem* pattern_item = new PatternItem(model, i);
    addItem(pattern_item);
    pattern_items.append(pattern_item);
  }

}

/**
 * @brief Slot called when the tileset selection has changed.
 *
 * The selection on the scene is updated.
 *
 * @param selected Items that have just been selected.
 * @param deselected Item that have just been deselected.
 */
void TilesetScene::update_selection_to_scene(
    const QItemSelection& selected, const QItemSelection& deselected) {

  const bool was_blocked = signalsBlocked();
  blockSignals(true);

  // Update the scene with the change.
  bool changed = false;
  Q_FOREACH (const QModelIndex& model_index, selected.indexes()) {
    int index = model_index.row();
    if (model.pattern_exists(index)) {
      if (!pattern_items[index]->isSelected()) {
        pattern_items[index]->setSelected(true);
        changed = true;
      }
    }
  }

  Q_FOREACH (const QModelIndex& model_index, deselected.indexes()) {
    int index = model_index.row();
    if (model.pattern_exists(index)) {
      if (pattern_items[index]->isSelected()) {
        pattern_items[index]->setSelected(false);
        changed = true;
      }
    }
  }
  blockSignals(was_blocked);

  if (changed) {
    emit selectionChanged();
  }
}

/**
 * @brief Slot called when the scene selection has changed.
 *
 * The new selection is forwarded to the tileset model.
 */
void TilesetScene::set_selection_from_scene() {

  // Forward the change to the tileset.
  QList<int> indexes;
  Q_FOREACH (QGraphicsItem* item, selectedItems()) {
    PatternItem* pattern_item = qgraphicsitem_cast<PatternItem*>(item);
    if (pattern_item != nullptr) {
      indexes << pattern_item->get_index();
    }
  }

  model.set_selected_indexes(indexes);
}

/**
 * @brief Selects all patterns of the tileset.
 */
void TilesetScene::select_all() {

  const bool was_blocked = signalsBlocked();
  blockSignals(true);
  Q_FOREACH (PatternItem* item, pattern_items) {
    if (item == nullptr) {
      continue;
    }
    item->setSelected(true);
  }
  blockSignals(was_blocked);

  // Emit the signal only once.
  emit selectionChanged();
}

/**
 * @brief Unselects all patterns of the tileset.
 */
void TilesetScene::unselect_all() {

  const bool was_blocked = signalsBlocked();
  blockSignals(true);
  Q_FOREACH (PatternItem* item, pattern_items) {
    if (item == nullptr) {
      continue;
    }
    item->setSelected(false);
  }
  blockSignals(was_blocked);

  // Emit the signal only once.
  emit selectionChanged();
}

/**
 * @brief Slot called when the position of a pattern changes.
 * @param index Index of the pattern changed.
 */
void TilesetScene::update_pattern_position(int index) {

  const QRect& box = model.get_pattern_frames_bounding_box(index);
  PatternItem* pattern_item = qgraphicsitem_cast<PatternItem*>(pattern_items[index]);
  if (pattern_item != nullptr) {
    pattern_item->setPos(box.topLeft());
    pattern_item->setPixmap(model.get_pattern_image_all_frames(index));
  }
}

/**
 * @brief Slot called when the animation of a pattern changes.
 * @param index Index of the pattern changed.
 */
void TilesetScene::update_pattern_animation(int index) {

  // Redraw the area containing the pattern: the selection marker may
  // have changed.
  const QRect& box = model.get_pattern_frames_bounding_box(index);
  update(box);
}

/**
 * @brief Slot called when a pattern is created.
 *
 * Elements are shifted in the items list.
 *
 * @param new_index Index of the newly created pattern.
 * @param new_id Id of the pattern.
 */
void TilesetScene::pattern_created(
    int new_index, const QString& /* new_id */) {

  // Keep the items list in sync with patterns in the model.
  PatternItem* pattern_item = new PatternItem(model, new_index);
  addItem(pattern_item);
  pattern_items.insert(new_index, pattern_item);

  // Each item stores its order, so we need to update them.
  for (int i = 0; i < pattern_items.size(); ++i) {
    pattern_items[i]->set_index(i);
  }
}

/**
 * @brief Slot called when a pattern is deleted.
 *
 * Elements are shifted in the items list.
 *
 * @param old_index Index of the pattern before it was deleted.
 * @param old_id Id of the deleted pattern.
 */
void TilesetScene::pattern_deleted(
    int old_index, const QString& /* old_id */) {

  // Keep the items list in sync with patterns in the model.
  std::unique_ptr<PatternItem> item = std::unique_ptr<PatternItem>(
        pattern_items.takeAt(old_index));

  // Each item stores its order, so we need to update them.
  for (int i = 0; i < pattern_items.size(); ++i) {
    pattern_items[i]->set_index(i);
  }

  removeItem(item.get());
}

/**
 * @brief Slot called when the id of a pattern changes.
 *
 * This changes its order in the items list.
 *
 * @param old_index Index of the pattern before the change.
 * @param old_id Id of the pattern before the change.
 * @param new_index Index of the pattern after the change.
 * @param new_id Id of the pattern after the change.
 */
void TilesetScene::pattern_id_changed(
    int old_index, const QString& /* old_id */,
    int new_index, const QString& /* new_id */) {

  // Keep the items list ordered as patterns in the model.
  pattern_items.move(old_index, new_index);

  // Each item stores its order, so we need to update them.
  for (int i = 0; i < pattern_items.size(); ++i) {
    pattern_items[i]->set_index(i);
  }
}

/**
 * @brief Slot called when the PNG image of the tileset has changed.
 */
void TilesetScene::image_changed() {

  for (int i = 0; i < pattern_items.size(); ++i) {
    pattern_items[i]->rebuild_pixmap();
  }
}

/**
 * @brief Creates a pattern item.
 * @param model The tileset.
 * @param index Index of the pattern in the tileset.
 */
PatternItem::PatternItem(TilesetModel& model, int index) :
  QGraphicsPixmapItem(),
  model(model),
  index(index) {

  QRect frame = model.get_pattern_frames_bounding_box(index);
  setPos(frame.topLeft());
  setFlags(ItemIsSelectable | ItemIsFocusable);
  rebuild_pixmap();
}

/**
 * @brief Returns the index of the pattern in the tileset.
 * @return The pattern index.
 */
int PatternItem::get_index() const {
  return index;
}

/**
 * @brief Sets the index of the pattern in the tileset.
 * @param index The new index.
 */
void PatternItem::set_index(int index) {
  this->index = index;
}

/**
 * @brief Creates or recreates the pixmap of this item.
 *
 * This function should be called when the PNG image of the tileset has changed.
 */
void PatternItem::rebuild_pixmap() {

  setPixmap(model.get_pattern_image_all_frames(index));
}

/**
 * @brief Paints the pattern item.
 *
 * Reimplemented to draw our own selection marker.
 *
 * @param painter The painter.
 * @param option Style option of the item.
 * @param widget The widget being painted or nullptr.
 */
void PatternItem::paint(QPainter* painter,
                        const QStyleOptionGraphicsItem* option,
                        QWidget* widget) {

  if (!model.pattern_exists(index)) {
    // Bug in the editor.
    qCritical() << TilesetScene::tr("No such pattern index: %1").arg(index);
    return;
  }

  QRect box = model.get_pattern_frames_bounding_box(index);
  QPoint top_left = box.topLeft();
  box.translate(-top_left);

  // Start with an opaque background, to erase anything below
  // if the pattern has transparency.
  if (scene() != nullptr) {
    painter->fillRect(box, scene()->backgroundBrush());
  } else {
    painter->fillRect(box, widget->palette().base());
  }

  const bool selected = option->state & QStyle::State_Selected;

  // First, paint the item like if there was no selection, to avoid
  // Qt's built-in selection marker.
  QStyleOptionGraphicsItem option_deselected = *option;
  option_deselected.state &= ~QStyle::State_Selected;
  QGraphicsPixmapItem::paint(painter, &option_deselected, widget);

  // Add our selection marker.
  if (selected) {
    QList<QRect> frames = model.get_pattern_frames(index);

    Q_FOREACH (QRect frame, frames) {
      frame.translate(-top_left);
      GuiTools::draw_rectangle_border(*painter, frame, Qt::blue, 1);
    }
  }
}

}
