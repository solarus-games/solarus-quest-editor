/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_DESTINATION_H
#define SOLARUSEDITOR_DESTINATION_H

#include "entity_model.h"

namespace SolarusEditor {

/**
 * @brief An editable destination.
 */
class Destination : public EntityModel {

public:

  Destination(MapModel& map, const EntityIndex& index);

  bool get_update_teletransporters() const;
  void set_update_teletransporters(bool update_teletransporters);

protected:

  void set_initial_values() override;

private:

  bool update_teletransporters;    /**< Whether teletransporters should be updated
                                     * when this destination is renamed. */
};

}

#endif
