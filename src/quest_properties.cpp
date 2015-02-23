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
#include "quest.h"
#include "quest_properties.h"
#include "size.h"

/**
 * @brief Creates quest properties for the specified quest.
 * @param quest The quest.
 */
QuestProperties::QuestProperties(Quest& quest) :
  quest(quest) {

  connect(&quest, SIGNAL(root_path_changed(const QString&)),
          this, SLOT(reload()));
  reload();
}

/**
 * @brief Reads quest.dat into this object.
 */
void QuestProperties::reload() {

  if (!quest.is_valid() || !quest.exists()) {
    return;
  }

  QString file_name = quest.get_properties_path();
  if (!properties.import_from_file(file_name.toStdString())) {
    throw EditorException(tr("Cannot open file '%1'").arg(file_name));
  }
}

/**
 * @brief Saves the properties to the quest.dat file of the quest.
 * @throws EditorException If the save operation failed.
 */
void QuestProperties::save() const {

  if (!quest.is_valid() || !quest.exists()) {
    throw EditorException(tr("No quest"));
  }

  QString file_name = quest.get_properties_path();
  if (!properties.export_to_file(file_name.toStdString())) {
    throw EditorException(tr("Cannot write file '%1'").arg(file_name));
  }
}

/**
 * @brief Returns the Solarus compatibility version of the quest.
 * @return The Solarus version.
 */
QString QuestProperties::get_solarus_version() const {

  return QString::fromStdString(properties.get_solarus_version());
}

/**
 * @brief Returns the Solarus compatibility version of the quest, without patch number.
 * @return The Solarus version without patch number.
 */
QString QuestProperties::get_solarus_version_without_patch() const {

  QString version = get_solarus_version();

  if (version.isEmpty()) {
    return version;
  }

  int dot_index_1 = version.indexOf('.');
  int dot_index_2 = version.indexOf('.', dot_index_1 + 1);
  if (dot_index_2 != -1) {
    // Remove the patch version (it does not break compatibility).
    version = version.section('.', 0, -2);
  }
  return version;
}

/**
 * @brief Returns the write directory of the quest.
 * @return The write directory.
 */
QString QuestProperties::get_write_dir() const {

  return properties.get_quest_write_dir().c_str();
}

/**
 * @brief Changes the write directory of the quest.
 * @param write_dir The new write directory.
 */
void QuestProperties::set_write_dir(const QString& write_dir) {

  if (write_dir.isEmpty()) {
    // Cannot be empty
    return;
  }

  QString old_write_dir = get_write_dir();
  if (write_dir == old_write_dir) {
    return;
  }

  properties.set_quest_write_dir(write_dir.toStdString());
  emit write_dir_changed(write_dir);
}

/**
 * @brief Returns the title bar of the quest.
 * @return The title bar.
 */
QString QuestProperties::get_title_bar() const {

  return properties.get_title_bar().c_str();
}

/**
 * @brief Changes the title bar of the quest.
 * @param title_bar The new title bar.
 */
void QuestProperties::set_title_bar(const QString& title_bar) {

  QString old_title_bar = get_title_bar();
  if (title_bar == old_title_bar) {
    return;
  }

  properties.set_title_bar(title_bar.toStdString());
  emit title_bar_changed(title_bar);
}

/**
 * @brief Returns the normal size of the quest.
 * @return The normal size.
 */
QSize QuestProperties::get_normal_quest_size() const {

  return Size::to_qsize(properties.get_normal_quest_size());
}

/**
 * @brief Changes the normal size of the quest.
 * @param size The new normal size.
 */
void QuestProperties::set_normal_quest_size(const QSize& size) {

  QSize old_size = get_normal_quest_size();
  if (size == old_size) {
    return;
  }

  properties.set_normal_quest_size(Size::to_solarus_size(size));
  emit normal_size_changed(size);
}

/**
 * @brief Returns the minimum quest size of the quest.
 * @return The minimum quest size.
 */
QSize QuestProperties::get_min_quest_size() const {

  return Size::to_qsize(properties.get_min_quest_size());
}

/**
 * @brief Changes the minimum size of the quest.
 * @param size The new minimum size.
 */
void QuestProperties::set_min_quest_size(const QSize& size) {

  QSize old_size = get_min_quest_size();
  if (size == old_size) {
    return;
  }

  properties.set_min_quest_size(Size::to_solarus_size(size));
  emit min_size_changed(size);
}

/**
 * @brief Returns the maximum quest size of the quest.
 * @return The maximum quest size.
 */
QSize QuestProperties::get_max_quest_size() const {

  return Size::to_qsize(properties.get_max_quest_size());
}

/**
 * @brief Changes the maximum size of the quest.
 * @param size The new maximum size.
 */
void QuestProperties::set_max_quest_size(const QSize& size) {

  QSize old_size = get_max_quest_size();
  if (size == old_size) {
    return;
  }

  properties.set_max_quest_size(Size::to_solarus_size(size));
  emit max_size_changed(size);
}
