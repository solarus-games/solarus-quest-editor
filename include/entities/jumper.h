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
#ifndef SOLARUSEDITOR_JUMPER_H
#define SOLARUSEDITOR_JUMPER_H

#include "entity_model.h"

/**
 * @brief An editable jumper.
 */
class Jumper : public EntityModel {

public:

  Jumper(MapModel& map, const EntityIndex& index);

  bool is_jump_horizontal() const;
  bool is_jump_vertical() const;
  bool is_jump_diagonal() const;

  bool is_size_valid() const override;
  QSize get_valid_size() const override;


  void draw(QPainter& painter) const override;

protected:

  void notify_field_changed(const QString& key, const QVariant& value) override;

private:

  void update_resize_mode();

};

#endif
