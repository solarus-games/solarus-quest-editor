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
#ifndef SOLARUSEDITOR_TEXT_EDITOR_H
#define SOLARUSEDITOR_TEXT_EDITOR_H

#include "widgets/editor.h"

namespace SolarusEditor {

class TextEditorWidget;

/**
 * \brief A plain text edition widget.
 */
class TextEditor : public Editor {
  Q_OBJECT

public:

  TextEditor(Quest& quest, const QString& file_path, QWidget* parent = nullptr);

  QString create_title() const;
  QIcon create_icon() const;

  void save() override;
  bool can_cut() const override;
  void cut() override;
  bool can_copy() const override;
  void copy() override;
  bool can_paste() const override;
  void paste() override;
  void select_all() override;
  void unselect_all() override;
  void find() override;
  void reload_settings() override;

private slots:

  void find_text_requested(const QString& text);
  void open_map_requested();

private:

  TextEditorWidget*
    text_widget;    /**< The text editing area contained. */
  QString map_id;   /**< The map id of this script (if it is a map script). */

};

}

#endif
