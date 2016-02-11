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
#ifndef SOLARUSEDITOR_QUEST_PROPERTIES_H
#define SOLARUSEDITOR_QUEST_PROPERTIES_H

#include <solarus/QuestProperties.h>
#include <QDate>
#include <QObject>

namespace SolarusEditor {

class Quest;

/**
 * @brief Stores the properties of a quest and sends signals when they change.
 */
class QuestProperties : public QObject {
  Q_OBJECT

public:

  explicit QuestProperties(Quest& quest);

  void save() const;

  QString get_solarus_version() const;
  QString get_solarus_version_without_patch() const;

  QString get_write_dir() const;
  void set_write_dir(const QString& write_dir);
  QString get_title() const;
  void set_title(const QString& title);
  QString get_short_description() const;
  void set_short_description(const QString& short_description);
  QString get_long_description() const;
  void set_long_description(const QString& long_description);
  QString get_author() const;
  void set_author(const QString& author);
  QString get_quest_version() const;
  void set_quest_version(const QString& quest_version);
  QDate get_release_date() const;
  void set_release_date(const QDate& release_date);
  QString get_website() const;
  void set_website(const QString& website);
  QSize get_normal_quest_size() const;
  void set_normal_quest_size(const QSize& size);
  QSize get_min_quest_size() const;
  void set_min_quest_size(const QSize& size);
  QSize get_max_quest_size() const;
  void set_max_quest_size(const QSize& size);

signals:

  void write_dir_changed(const QString& write_dir);
  void title_changed(const QString& title);
  void short_description_changed(const QString& short_description);
  void long_description_changed(const QString& long_description);
  void author_changed(const QString& author);
  void quest_version_changed(const QString& quest_version);
  void release_date_changed(const QDate& release_date);
  void website_changed(const QString& website);
  void normal_size_changed(const QSize& size);
  void min_size_changed(const QSize& size);
  void max_size_changed(const QSize& size);

private slots:

  void reload();

private:

  Quest& quest;                                  /**< The quest. */
  Solarus::QuestProperties properties;           /**< The wrapped data. */

};

}

#endif
