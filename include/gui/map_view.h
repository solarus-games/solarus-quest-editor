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
#ifndef SOLARUSEDITOR_MAP_VIEW_H
#define SOLARUSEDITOR_MAP_VIEW_H

#include "entities/entity_traits.h"
#include "layer_traits.h"
#include <QGraphicsView>
#include <QPointer>
#include <memory>
#include <solarus/MapData.h>

class EntityModel;
class MapModel;
class MapScene;
class ViewSettings;

using EntityModels = std::vector<std::unique_ptr<EntityModel>>;

/**
 * @brief Graphical view of the map image, allowing to manage entities.
 */
class MapView : public QGraphicsView {
  Q_OBJECT

public:

  /**
   * @brief Indicates what the user is currently doing on the map view.
   */
  class State {

  public:

    State(MapView& view);
    virtual ~State();

    MapView& get_view();
    MapScene& get_scene();
    MapModel& get_map();

    virtual void start();
    virtual void stop();

    virtual void mouse_pressed(const QMouseEvent& event);
    virtual void mouse_released(const QMouseEvent& event);
    virtual void mouse_moved(const QMouseEvent& event);
    virtual void context_menu_requested(const QPoint& where);
    virtual void tileset_selection_changed();

  private:

    MapView& view;

  };

  MapView(QWidget* parent = nullptr);

  MapModel* get_model();
  MapScene* get_scene();
  void set_model(MapModel* map);
  void set_view_settings(ViewSettings& view_settings);

  // State of the view.
  void start_state_doing_nothing();
  void start_state_drawing_rectangle(const QPoint& initial_point);
  void start_state_moving_entities(const QPoint& initial_point);
  void start_state_adding_entities(EntityModels&& entities);
  void start_adding_entities_from_tileset_selection();

  // Selection.
  int get_num_selected_entities() const;
  QList<EntityIndex> get_selected_entities() const;
  void set_selected_entities(const QList<EntityIndex>& indexes);

signals:

  void mouse_map_coordinates_changed(const QPoint& xy);
  void mouse_left();
  void undo_requested();
  void move_entities_requested(
      const QList<EntityIndex>& indexes,
      const QPoint& translation,
      bool allow_merge_to_previous);
  void add_entities_requested(
      EntityModels& entities);
  void remove_entities_requested(const QList<EntityIndex>& indexes);

public slots:

  void update_zoom();
  void zoom_in();
  void zoom_out();
  void mouse_coordinates_changed(const QPoint& xy);
  void update_grid_visibility();
  void update_layer_visibility(Layer layer);
  void update_entity_type_visibility(EntityType type);
  void tileset_selection_changed();

  void move_selected_entities(const QPoint& translation, bool allow_merge_to_previous);
  void remove_selected_entities();

protected:

  void drawForeground(QPainter* painter, const QRectF& rectangle) override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

private:

  void set_state(std::unique_ptr<State> state);

  QPointer<MapModel> model;        /**< The map model. */
  MapScene* scene;                 /**< The scene viewed. */
  QPointer<ViewSettings>
      view_settings;               /**< What is displayed in the view. */
  double zoom;                     /**< Zoom factor currently applied. */
  std::unique_ptr<State> state;    /**< Current state of the view. */

};

#endif
