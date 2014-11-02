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
#include "obsolete_quest_exception.h"
#include <solarus/Common.h>
#include <QApplication>

/**
 * @brief Creates an obsolete quest exception.
 * @param quest_format Format of the quest.
 */
ObsoleteQuestException::ObsoleteQuestException(const QString& quest_format) :
  EditorException(
    QApplication::tr(
      "The format of this quest (%1) is obsolete.\nPlease upgrade your quest "
      " data files to Solarus %2.").arg(quest_format, SOLARUS_VERSION)
    ),
  quest_format(quest_format) {

}

/**
 * @brief Returns the format of the too old quest.
 * @return The quest format.
 */
QString ObsoleteQuestException::get_quest_format() const {
  return quest_format;
}
