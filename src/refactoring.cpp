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
#include "refactoring.h"

namespace SolarusEditor {

/**
 * @brief Creates a refactoring object.
 * @param function The function to wrap.
 * It should perform the refactoring and return the list of files that
 * were modified.
 */
Refactoring::Refactoring(const RefactoringFunction& function):
  function(function),
  files_unsaved_allowed() {
}

/**
 * @brief Returns the file paths that don't need to be saved
 * before performing this refactoring.
 * @return The files that don't need to be saved.
 */
QSet<QString> Refactoring::get_files_unsaved_allowed() const {
  return files_unsaved_allowed;
}

/**
 * @brief Returns whether a file needs to be saved before this refactoring.
 * @param file_name Path of a file of this quest.
 * @return @c true if this file needs to be saved for this refactoring.
 */
bool Refactoring::get_file_unsaved_allowed(const QString& file_name) const {

  return files_unsaved_allowed.contains(file_name);
}

/**
 * @brief Sets whether a file needs to be saved before this refactoring.
 * @param file_name Path of a file of this quest.
 * @param @c true to require this file to be saved for this refactoring.
 * By default, all files need to be saved.
 */
void Refactoring::set_file_unsaved_allowed(const QString& file_name, bool allow_unsaved) {

  if (allow_unsaved) {
    files_unsaved_allowed.insert(file_name);
  }
  else {
    files_unsaved_allowed.remove(file_name);
  }
}

/**
 * @brief Performs the refactoring operation.
 * @return The list of files that were modified by the refactoring.
 * @throws EditorException in case of error.
 */
QStringList Refactoring::execute() const {
  return function();
}

}
