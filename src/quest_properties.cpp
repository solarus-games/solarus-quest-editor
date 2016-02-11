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
#include "quest.h"
#include "quest_properties.h"
#include "size.h"

namespace SolarusEditor {

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

  return QString::fromStdString(properties.get_quest_write_dir());
}

/**
 * @brief Changes the write directory of the quest.
 * @param write_dir The new write directory.
 */
void QuestProperties::set_write_dir(const QString& write_dir) {

  QString old_write_dir = get_write_dir();
  if (write_dir == old_write_dir) {
    return;
  }

  properties.set_quest_write_dir(write_dir.toStdString());
  emit write_dir_changed(write_dir);
}

/**
 * @brief Returns the title of the quest.
 * @return The title.
 */
QString QuestProperties::get_title() const {

  return QString::fromStdString(properties.get_title());
}

/**
 * @brief Changes the title of the quest.
 * @param title The new title.
 */
void QuestProperties::set_title(const QString& title) {

  QString old_title = get_title();
  if (title == old_title) {
    return;
  }

  properties.set_title(title.toStdString());
  emit title_changed(title);
}

/**
 * @brief Returns the one-line description of the quest.
 * @return The one-line description.
 */
QString QuestProperties::get_short_description() const {

  return QString::fromStdString(properties.get_short_description());
}

/**
 * @brief Changes the one-line description of the quest.
 * @param short_description The one-line description.
 */
void QuestProperties::set_short_description(const QString& short_description) {

  QString old_short_description = get_short_description();
  if (short_description == old_short_description) {
    return;
  }

  properties.set_short_description(short_description.toStdString());
  emit short_description_changed(short_description);
}

/**
 * @brief Returns the one-line description of the quest.
 * @return The one-line description.
 */
QString QuestProperties::get_long_description() const {

  return QString::fromStdString(properties.get_long_description());
}

/**
 * @brief Changes the one-line description of the quest.
 * @param long_description The one-line description.
 */
void QuestProperties::set_long_description(const QString& long_description) {

  QString old_long_description = get_long_description();
  if (long_description == old_long_description) {
    return;
  }

  properties.set_long_description(long_description.toStdString());
  emit long_description_changed(long_description);
}

/**
 * @brief Returns the author of the quest.
 * @return The author.
 */
QString QuestProperties::get_author() const {

  return QString::fromStdString(properties.get_author());
}

/**
 * @brief Changes the author of the quest.
 * @param author The author.
 */
void QuestProperties::set_author(const QString& author) {

  QString old_author = get_author();
  if (author == old_author) {
    return;
  }

  properties.set_author(author.toStdString());
  emit author_changed(author);
}

/**
 * @brief Returns the version of the quest.
 * @return The version.
 */
QString QuestProperties::get_quest_version() const {

  return QString::fromStdString(properties.get_quest_version());
}

/**
 * @brief Changes the version of the quest.
 * @param quest_version The version.
 */
void QuestProperties::set_quest_version(const QString& quest_version) {

  QString old_quest_version = get_quest_version();
  if (quest_version == old_quest_version) {
    return;
  }

  properties.set_quest_version(quest_version.toStdString());
  emit quest_version_changed(quest_version);
}

/**
 * @brief Returns the website of the quest.
 * @return The website.
 */
QString QuestProperties::get_website() const {

  return QString::fromStdString(properties.get_website());
}

/**
 * @brief Changes the website of the quest.
 * @param website The website.
 */
void QuestProperties::set_website(const QString& website) {

  QString old_website = get_website();
  if (website == old_website) {
    return;
  }

  properties.set_website(website.toStdString());
  emit website_changed(website);
}

/**
 * @brief Returns the quest release date.
 * @return The release date or an invalid date.
 */
QDate QuestProperties::get_release_date() const {

  QString date_string = QString::fromStdString(properties.get_release_date());
  return QDate::fromString(date_string, "yyyyMMdd");
}

/**
 * @brief Changes the quest release date.
 * @param release_date The release date or an invalid date.
 */
void QuestProperties::set_release_date(const QDate& release_date) {

  QDate old_release_date = get_release_date();
  if (release_date == old_release_date) {
    return;
  }

  QString date_string;
  if (release_date.isValid()) {
    date_string = release_date.toString("yyyyMMdd");
  }
  properties.set_release_date(date_string.toStdString());
  emit release_date_changed(release_date);
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

}
