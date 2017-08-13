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
#include "border_set.h"

namespace SolarusEditor {

/**
 * @brief Creates an empty border set.
 */
BorderSet::BorderSet() :
  patterns(),
  inner(false) {

  patterns.reserve(12);
  for (int i = 0; i < 12; ++i) {
    patterns.append(QString());
  }
}

/**
 * @brief Creates a border set with the given properties.
 * @param patterns The 12 pattern ids to set.
 * @param inner Whether the tiles should be generated inside or outside the countour.
 */
BorderSet::BorderSet(const QStringList& patterns, bool inner):
  patterns(patterns),
  inner(inner) {

  Q_ASSERT(patterns.size() == 12);
}

/**
 * @brief Returns the 12 patterns ids of this border set.
 * @return The pattern ids.
 */
QStringList BorderSet::get_patterns() const {
  return patterns;
}

/**
 * @brief Sets the 12 patterns ids of this border set.
 * @param patterns The pattern ids. It must be a list of 12 elements.
 */
void BorderSet::set_patterns(const QStringList& patterns) {

  Q_ASSERT(patterns.size() == 12);
  this->patterns = patterns;
}

/**
 * @brief Returns the pattern id of the given border.
 * @param which_border Which border to get.
 * @return The corresponding pattern id, or an empty string to mean no pattern.
 */
QString BorderSet::get_pattern(WhichBorder which_border) const {

  Q_ASSERT(patterns.size() == 12);
  return patterns[static_cast<int>(which_border)];
}

/**
 * @brief Sets the pattern id of the given border.
 * @param which_border Which border to set.
 * @param pattern_id The corresponding pattern id,
 * or an empty string to mean no pattern.
 */
void BorderSet::set_pattern(WhichBorder which_border, const QString& pattern_id) {

  Q_ASSERT(patterns.size() == 12);
  patterns[static_cast<int>(which_border)] = pattern_id;
}

/**
 * @brief Returns whether the borders should be generated inside or outside
 * the countour.
 * @return @c true if these borders should be generated outside the countour.
 */
bool BorderSet::is_inner() const {
  return inner;
}

/**
 * @brief Sets whether the borders should be generated inside or outside
 * the countour.
 * @param inner @c true if these borders should be generated outside the
 * countour.
 */
void BorderSet::set_inner(bool inner) {
  this->inner = inner;
}

}
