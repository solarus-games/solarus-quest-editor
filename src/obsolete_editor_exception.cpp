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
#include "obsolete_editor_exception.h"
#include <solarus/Common.h>
#include <QApplication>

/**
 * @brief Creates an obsolete editor exception.
 * @param quest_format Format of the quest.
 */
ObsoleteEditorException::ObsoleteEditorException(const QString& quest_format) :
  EditorException(
    QApplication::tr(
      "The format of this quest (%1) is not supported by this version of the "
      "quest editor (%2).\nPlease download the latest version of the editor "
      "on www.solarus-games.org."
    ).arg(quest_format, SOLARUS_VERSION_WITHOUT_PATCH)),
  quest_format(quest_format) {

}

/**
 * @brief Returns the format of the too recent quest.
 * @return The quest format.
 */
QString ObsoleteEditorException::get_quest_format() const {
  return quest_format;
}
