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
#include "widgets/sprite_previewer.h"
#include <QMenu>

/**
 * @brief Create a sprite previewer.
 * @param parent The parent object or nullptr.
 */
SpritePreviewer::SpritePreviewer(QWidget *parent) :
  QWidget(parent),
  model(nullptr),
  zoom(1.0) {

  ui.setupUi(this);

  // Create frame and origin items.
  item = new QGraphicsPixmapItem();
  origin_h = new QGraphicsLineItem();
  origin_v = new QGraphicsLineItem();

  origin_h->setPen(QPen(Qt::blue));
  origin_v->setPen(QPen(Qt::blue));

  // Create the scene.
  ui.frame_view->setScene(new QGraphicsScene());
  ui.frame_view->scene()->addItem(item);
  ui.frame_view->scene()->addItem(origin_h);
  ui.frame_view->scene()->addItem(origin_v);
  ui.frame_view->scene()->setBackgroundBrush(
        ui.frame_view->scene()->palette().base());

  // Zoom.
  ui.zoom_button->setMenu(create_zoom_menu());
  ui.zoom_button->setPopupMode(QToolButton::InstantPopup);
  set_zoom(2.0);
  update_zoom();

  connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));

  connect(ui.start_button, SIGNAL(clicked()), this, SLOT(start()));
  connect(ui.stop_button, SIGNAL(clicked()), this, SLOT(stop()));
  connect(ui.first_button, SIGNAL(clicked()), this, SLOT(first()));
  connect(ui.previous_button, SIGNAL(clicked()), this, SLOT(previous()));
  connect(ui.last_button, SIGNAL(clicked()), this, SLOT(last()));
  connect(ui.next_button, SIGNAL(clicked()), this, SLOT(next()));

  connect(ui.origin_check_box, SIGNAL(clicked()), this, SLOT(update_origin()));
}

/**
 * @brief Sets the sprite model.
 * @param model The sprite model, or nullptr to remove any model.
 * This class does not take ownership on the model.
 * The model can be deleted safely.
 */
void SpritePreviewer::set_model(SpriteModel* model) {

  if (this->model != nullptr) {
    this->model->disconnect(this);
    this->model = nullptr;
  }

  this->model = model;

  if (model != nullptr) {
    connect(&model->get_selection_model(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(update_selection()));
    update_selection();

    connect(model, SIGNAL(animation_frame_delay_changed(Index,uint32_t)),
            this, SLOT(update_frame_delay()));
    connect(model, SIGNAL(animation_image_changed(Index,QString)),
            this, SLOT(update_frames()));

    connect(model, SIGNAL(direction_position_changed(Index,QPoint)),
            this, SLOT(update_frames()));
    connect(model, SIGNAL(direction_size_changed(Index,QSize)),
            this, SLOT(update_frames()));
    connect(model, SIGNAL(direction_num_frames_changed(Index,int)),
            this, SLOT(update_frames()));
    connect(model, SIGNAL(direction_num_columns_changed(Index,int)),
            this, SLOT(update_frames()));

    connect(model, SIGNAL(direction_origin_changed(Index,QPoint)),
            this, SLOT(update_origin()));
  }
}

/**
 * @brief Changes the background color.
 * @param color The color to set.
 */
void SpritePreviewer::set_background_color(const QColor& color) {
  ui.frame_view->scene()->setBackgroundBrush(QBrush(color));
}

/**
 * @brief Changes whether the origin is displayed.
 * @param show_origin true to display the origin, false otherwise.
 */
void SpritePreviewer::set_show_origin(bool show_origin) {

  ui.origin_check_box->setChecked(show_origin);
  update_origin();
}

/**
 * @brief Changes the origin color.
 * @param color The color to set.
 */
void SpritePreviewer::set_origin_color(const QColor& color) {

  origin_h->setPen(QPen(color));
  origin_v->setPen(QPen(color));
}

/**
 * @brief Update the selection.
 */
void SpritePreviewer::update_selection() {

  index = model->get_selected_index();
  update_frames();
  update_frame_delay();
  update_buttons();
  update_origin();

  bool is_direction = index.is_direction_index();
  ui.origin_check_box->setEnabled(is_direction);
  ui.zoom_button->setEnabled(is_direction);

  if (!is_direction) {
    timer.stop();
  }
}

/**
 * @brief Update the frame delay.
 */
void SpritePreviewer::update_frame_delay() {

  if (timer.isActive()) {
    timer.setInterval(model->get_animation_frame_delay(index));
  }
}

/**
 * @brief Update buttons.
 */
void SpritePreviewer::update_buttons() {

  bool start_enabled  = index.is_direction_index();
  bool active = timer.isActive();
  bool first_enabled = start_enabled && !active && current_frame > 0;
  bool last_enabled =
      start_enabled && !active && current_frame < frames.size() - 1;

  ui.start_button->setEnabled(start_enabled);
  ui.stop_button->setEnabled(start_enabled && active);

  ui.first_button->setEnabled(first_enabled);
  ui.previous_button->setEnabled(first_enabled);

  ui.last_button->setEnabled(last_enabled);
  ui.next_button->setEnabled(last_enabled);

  if (active) {
    ui.start_button->setIcon(QIcon(":/images/icon_pause.png"));
    ui.start_button->setToolTip(tr("Pause"));
  } else {
    ui.start_button->setIcon(QIcon(":/images/icon_start.png"));
    ui.start_button->setToolTip(tr("Start"));
  }
}

/**
 * @brief Update the frames of the selected direction.
 */
void SpritePreviewer::update_frames() {

  frames = model->get_direction_all_frames(index);
  current_frame = 0;
  update_frame();
}

/**
 * @brief Update the displayed frame.
 */
void SpritePreviewer::update_frame() {

  QPixmap pixmap;
  int nb_frames = frames.size();

  if (current_frame < nb_frames) {
    pixmap = frames[current_frame];
  }
  item->setPixmap(pixmap);

  QString size_str = QString::number(nb_frames > 0 ? nb_frames - 1 : 0);
  ui.frame_label->setText(QString::number(current_frame) + " / " + size_str);
}

/**
 * @brief Update the displayed origin.
 */
void SpritePreviewer::update_origin() {

  bool show_origin = ui.origin_check_box->isChecked() && frames.size() > 0;

  origin_h->setVisible(show_origin);
  origin_v->setVisible(show_origin);

  if (!show_origin) {
    return;
  }

  QRect rect = frames[0].rect();
  QPoint origin = model->get_direction_origin(index);

  origin_h->setLine(0, origin.y(), rect.width(), origin.y());
  origin_v->setLine(origin.x(), 0, origin.x(), rect.height());
}

/**
 * @brief Slot called when the timer timeout signal is emit.
 */
void SpritePreviewer::timeout() {

  int next_frame = current_frame + 1;

  if (next_frame >= frames.size()) {
    int loop_on_frame = model->get_animation_loop_on_frame(index);
    if (loop_on_frame >= 0) {
      next_frame = loop_on_frame;
    } else {
      next_frame = current_frame;
    }
  }

  if (next_frame == current_frame) {
    timer.stop();
  } else {
    current_frame = next_frame;
  }

  update_frame();
  update_buttons();
}

/**
 * @brief Slot called when the user click on the start button.
 */
void SpritePreviewer::start() {

  if (timer.isActive()) {
    timer.stop();
    update_frame();
    update_buttons();
  } else {
    int frame_delay = model->get_animation_frame_delay(index);
    timer.start(frame_delay);
  }
}

/**
 * @brief Slot called when the user click on the stop button.
 */
void SpritePreviewer::stop() {

  timer.stop();
  current_frame = 0;
  update_frame();
  update_buttons();
}

/**
 * @brief Slot called when the user click on the first button.
 */
void SpritePreviewer::first() {

  current_frame = 0;
  update_frame();
  update_buttons();
}

/**
 * @brief Slot called when the user click on the previous button.
 */
void SpritePreviewer::previous() {

  if (current_frame <= 0) {
    return;
  }

  current_frame--;
  update_frame();
  update_buttons();
}

/**
 * @brief Slot called when the user click on the last button.
 */
void SpritePreviewer::last() {

  int nb_frames = frames.size();
  current_frame = nb_frames > 0 ? nb_frames - 1 : 0;
  update_frame();
  update_buttons();
}

/**
 * @brief Slot called when the user click on the next button.
 */
void SpritePreviewer::next() {

  if (current_frame + 1 >= frames.size()) {
    return;
  }

  current_frame++;
  update_frame();
  update_buttons();
}

/**
 * @brief Updates the zoom menu.
 */
void SpritePreviewer::update_zoom() {

  if (zoom_actions.contains(this->zoom)) {
    zoom_actions[this->zoom]->setChecked(true);
  }
}

/**
 * @brief Changes the current zoom.
 * @param zoom The new zoom value.
 */
void SpritePreviewer::set_zoom(double zoom) {

  zoom = qMin(4.0, qMax(0.25, zoom));

  if (zoom == this->zoom) {
    return;
  }

  double scale_factor = zoom / this->zoom;
  ui.frame_view->scale(scale_factor, scale_factor);
  this->zoom = zoom;
}

/**
 * @brief Creates a menu with zoom actions.
 * @return The created menu. It has no parent initially.
 */
QMenu* SpritePreviewer::create_zoom_menu() {

  QMenu* zoom_menu = new QMenu(tr("Zoom"));
  std::vector<std::pair<QString, double>> zooms = {
    { tr("50 %"), 0.5 },
    { tr("100 %"), 1.0 },
    { tr("200 %"), 2.0 },
    { tr("400 %"), 4.0 }
  };
  QActionGroup* action_group = new QActionGroup(this);
  for (const std::pair<QString, double>& zoom : zooms) {
    QAction* action = new QAction(zoom.first, action_group);
    zoom_actions[zoom.second] = action;
    action->setCheckable(true);
    connect(action, &QAction::triggered, [=]() {
      set_zoom(zoom.second);
    });
    zoom_menu->addAction(action);
  }

  return zoom_menu;
}
