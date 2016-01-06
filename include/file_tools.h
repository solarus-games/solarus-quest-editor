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
#ifndef SOLARUSEDITOR_FILE_TOOLS_H
#define SOLARUSEDITOR_FILE_TOOLS_H

class QString;

/**
 * \brief Utility functions on files and directories.
 *
 * These are general helper functions, not specific to Solarus.
 * The Quest class contains file functions specialized to Solarus quests.
 */
namespace FileTools {

void copy_recursive(const QString& src, const QString& dst);
void delete_recursive(const QString& path);

}

#endif
