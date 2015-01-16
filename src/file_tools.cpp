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
#include "editor_exception.h"
#include "file_tools.h"
#include <QDir>
#include <QFileInfo>
#include <QApplication>

namespace FileTools {

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
      throw EditorException(QApplication::tr("Cannot create folder '%1'").arg(dst));
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
      throw EditorException(QApplication::tr("Cannot copy file '%1' to '%2'").arg(src, dst));
    }

    if (src.startsWith(":/")) {
      // Files from Qt resources are read-only. Set usual permissions now.
      QFile::setPermissions(dst,
                            QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                            QFile::ReadGroup| QFile::ExeGroup |
                            QFile::ReadOther| QFile::ExeOther);
    }
  }
}

/**
 * @brief Utility function to delete a file or a directory with its content.
 *
 * Does nothing if the file or directory does not exist.
 *
 * @param path The file or directory to delete.
 * @throws EditorException if the deletion failed.
 * In this case, it the path to delete was a directory, this function still
 * tries to delete as much files as possible in the directory.
 */
void delete_recursive(const QString& path) {

  QFileInfo info(path);
  if (!info.exists()) {
    return;
  }

  if (!info.isDir()) {
    // Not a directory.
    if (!QFile::remove(path)) {
      throw EditorException(QApplication::tr("Failed to delete file '%1'").arg(path));
    }
  }
  else {
    // Directory.
    if (!QDir(path).removeRecursively()) {
      throw EditorException(QApplication::tr("Failed to delete folder '%1'").arg(path));
    }
  }
}

}
