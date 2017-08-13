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
#ifndef SOLARUSEDITOR_BORDER_SET_H
#define SOLARUSEDITOR_BORDER_SET_H

#include "which_border.h"
#include <QStringList>

namespace SolarusEditor {

/**
 * @brief A set of borders for the autotiler.
 */
class BorderSet {

public:

  BorderSet();
  explicit BorderSet(const QStringList& patterns);

  QStringList get_patterns() const;
  void set_patterns(const QStringList& patterns);

  QString get_pattern(WhichBorder which_border) const;
  void set_pattern(WhichBorder which_border, const QString& pattern_id);

  bool is_inner() const;
  void set_inner(bool inner);

private:

  QStringList patterns;     /**< Ids of the 12 tile patterns defining the border. */
  bool inner;               /**< Whether the borders should be placed inside the countours
                             * or outside them. */

};

}

#endif
