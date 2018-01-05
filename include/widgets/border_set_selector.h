/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_BORDER_SET_SELECTOR_H
#define SOLARUSEDITOR_BORDER_SET_SELECTOR_H

#include <QComboBox>
#include <QPointer>

namespace SolarusEditor {

class Quest;

/**
 * @brief A combo box that allows to choose a border set from a tileset.
 */
class BorderSetSelector : public QComboBox {
  Q_OBJECT

public:

  BorderSetSelector(QWidget* parent);

  const QString& get_tileset_id() const;
  void set_tileset_id(Quest& quest, const QString& tileset_id);

  QString get_selected_border_set_id() const;
  void set_selected_border_set_id(const QString& border_set_id);

public slots:

  void build();

private:

  QPointer<Quest> quest;     /**< The quest or nullptr if it is not set yet. */
  QString tileset_id;        /**< Id of the tileset from where to show border sets, or an empty string. */

};

}

#endif
