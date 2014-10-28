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
#include "editor_exception.h"
#include "new_quest_builder.h"
#include "quest.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace NewQuestBuilder {

namespace {

/**
 * @brief Utility function to copy a file or directory with its content.
 * @param src The file or directory to copy.
 * @param dst The destination file path. It should be the name of the file
 * or directory to create.
 * @throws EditorException if the copy failed. In this case, files already
 * successfully copied are left.
 */
void copy_recursive(const QString& src, const QString& dst) {

  QFileInfo src_info(src);
  QFileInfo dst_info(dst);

  if (!src_info.exists()) {
    throw EditorException(QApplication::tr("No such file or directory: '%1'").arg(src));
  }

  if (!src_info.isReadable()) {
    throw EditorException(QApplication::tr("Source file cannot be read: '%1'").arg(src));
  }

  if (dst_info.exists()) {
    throw EditorException(QApplication::tr("Destination already exists: '%1'").arg(dst));
  }

  if (src_info.isDir()) {
    QDir dst_dir(dst);

    dst_dir.cdUp();

    if (!dst_dir.exists()) {
      throw EditorException(QApplication::tr("No such directory: '%1'").arg(dst_dir.path()));
    }

    if (!dst_dir.mkdir(dst_info.fileName())) {
      throw EditorException(QApplication::tr("Cannot create directory '%1'").arg(dst));
    }

    QDir src_dir(src);
    QStringList file_names = src_dir.entryList(
          QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    for (const QString& file_name : file_names) {
      QString next_src = src + '/' + file_name;
      QString next_dst = dst + '/' + file_name;
      copy_recursive(next_src, next_dst);
    }
  }
  else {
    if (!QFile::copy(src, dst)) {
      throw EditorException(QApplication::tr("Cannot create file '%1' to '%2'").arg(src, dst));
    }
    // Files from resources are read-only. Set usual permissions now.
    QFile::setPermissions(dst,
          QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
          QFile::ReadGroup| QFile::ExeGroup |
          QFile::ReadOther| QFile::ExeOther);
  }
}

}

/**
 * @brief Creates initial files of a new quest.
 * @param quest_path Root path of the quest to create.
 * The data directory will be created there.
 * @throws EditorException if the files creation failed.
 */
void create_initial_quest_files(const QString& quest_path) {

  // Create files from resources.
  copy_recursive(":/initial_quest/data", quest_path + "/data");

  // Make sure all resource directories exist.
  Quest quest(quest_path);
  for (const auto& kvp : Solarus::QuestResources::get_resource_type_names()) {
    ResourceType resource_type = kvp.first;
    quest.create_dir_if_not_exists(quest.get_resource_path(resource_type));
  }
}

}
