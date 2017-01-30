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
#include "editor_exception.h"
#include "file_tools.h"
#include <solarus/Common.h>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <QDebug>

namespace SolarusEditor {

namespace FileTools {

namespace {
  bool assets_path_initialized = false;
  QString assets_path;
}

/**
 * @brief Determines the path to the Solarus Quest Editor assets directory.
 *
 * The directory "assets" is searched in the following paths in this order:
 * - The directory containing the executable.
 * - The source path (macro SOLARUSEDITOR_SOURCE_PATH)
 *   (useful for developer builds).
 * - The install path (macro SOLARUSEDITOR_INSTALL_PATH)
 *   followed by "share/solarus-quest-editor/".
 */
void initialize_assets() {

  const QString& executable_path = QCoreApplication::applicationDirPath();
  QStringList potential_paths;

  // Try the current directory first.
  potential_paths << executable_path + "/assets";

  // Try the source path if we are not running the installed executable.
  bool running_installed_executable = (executable_path == SOLARUSEDITOR_INSTALL_PATH "/bin");
#ifdef SOLARUSEDITOR_SOURCE_PATH
  if (!running_installed_executable) {
    potential_paths << SOLARUSEDITOR_SOURCE_PATH "/assets";
  }
#endif

  // Try the install path if we are running the installed executable.
#ifdef SOLARUSEDITOR_INSTALL_PATH
  if (running_installed_executable) {
    potential_paths << SOLARUSEDITOR_INSTALL_PATH "/share/solarus-quest-editor/assets";
  }
#endif

  assets_path_initialized = true;
  for (const QString& potential_path : potential_paths) {
    if (QFile(potential_path).exists()) {
      assets_path = potential_path;
      return;
    }
  }
}

/**
 * @brief Returns the path to the Solarus Quest Editor assets directrory.
 * @return The assets path or an empty string if assets could not be found.
 */
QString get_assets_path() {

  if (!assets_path_initialized) {
    initialize_assets();
  }
  return assets_path;
}

/**
 * @brief Utility function to copy a file or directory with its content.
 * @param src The file or directory to copy.
 * @param dst The destination file path. It should be the name of the file
 * or directory to create.
 * @throws EditorException if the copy failed. In this case, files already
 * successfully copied are left.
 */
void copy_recursive(const QString& src, const QString& dst) {

  if (src == dst) {
    throw EditorException(QApplication::tr("Source and destination are the same: '%1'").arg(src));
  }

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

    QString src_canonical_path = src_info.canonicalFilePath();
    QString dst_parent_canonical_path = dst_dir.canonicalPath();

    if (dst_parent_canonical_path.startsWith(src_canonical_path)) {
      throw EditorException(QApplication::tr("Cannot copy directory '%1' to one of its own subdirectories: '%2'").arg(src, dst));
    }

    if (!dst_dir.mkdir(dst_info.fileName())) {
      throw EditorException(QApplication::tr("Cannot create folder '%1'").arg(dst));
    }

    QDir src_dir(src);
    QStringList file_names = src_dir.entryList(
          QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    Q_FOREACH (const QString& file_name, file_names) {
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

/**
 * @brief Replaces all occurences of the given pattern in a file.
 * @param path Path of the file to modify.
 * @param regexp The pattern to replace.
 * @param replacement The string to put instead of the pattern.
 * @return @c true if there was a change.
 * @throws EditorException In case of error.
 */
bool replace_in_file(
    const QString& path,
    const QRegularExpression& regex,
    const QString& replacement
) {
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    throw EditorException(QApplication::tr("Cannot open file '%1'").arg(file.fileName()));
  }
  QTextStream in(&file);
  in.setCodec("UTF-8");
  QString content = in.readAll();
  file.close();

  QString old_content = content;
  content.replace(regex, replacement);

  if (content == old_content) {
    // No change.
    return false;
  }

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    throw EditorException(QApplication::tr("Cannot open file '%1' for writing").arg(file.fileName()));
  }
  QTextStream out(&file);
  out.setCodec("UTF-8");
  out << content;
  file.close();
  return true;
}

}  // namespace FileTools

}  // namespace SolarusEditor
