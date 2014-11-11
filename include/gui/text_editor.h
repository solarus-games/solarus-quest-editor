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
#ifndef SOLARUSEDITOR_TEXT_EDITOR_H
#define SOLARUSEDITOR_TEXT_EDITOR_H

#include "gui/editor.h"

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

  virtual void save() override;
  virtual bool can_cut() const override;
  virtual void cut() override;
  virtual bool can_copy() const override;
  virtual void copy() override;
  virtual bool can_paste() const override;
  virtual void paste() override;

private:

  TextEditorWidget* text_widget;    /**< The text editing area contained. */

};

#endif
