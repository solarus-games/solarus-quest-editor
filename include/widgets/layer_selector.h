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
#ifndef SOLARUSEDITOR_LAYER_SELECTOR_H
#define SOLARUSEDITOR_LAYER_SELECTOR_H

#include <QComboBox>

namespace SolarusEditor {

/**
 * @brief A combo box that allows to choose a layer.
 */
class LayerSelector : public QComboBox {
  Q_OBJECT

public:

  explicit LayerSelector(QWidget* parent = nullptr);

  int get_selected_layer() const;
  void set_selected_layer(int layer);

};

}

#endif
