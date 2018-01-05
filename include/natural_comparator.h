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
#ifndef SOLARUSEDITOR_NATURAL_COMPARATOR_H
#define SOLARUSEDITOR_NATURAL_COMPARATOR_H

#include <QCollator>

namespace SolarusEditor {

/**
 * @brief A string comparator that sorts number parts intuitively.
 *
 * For example, "enemy_2" is before "enemy_10".
 */
class NaturalComparator {

public:

  NaturalComparator() {
    collator.setNumericMode(true);
  }

  bool operator() (const QString& lhs, const QString& rhs) const {
    return collator.compare(lhs, rhs) < 0;
  }

private:

  QCollator collator;

};

}

#endif
