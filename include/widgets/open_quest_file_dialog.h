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
#ifndef SOLARUSEDITOR_OPEN_QUEST_FILE_DIALOG_H
#define SOLARUSEDITOR_OPEN_QUEST_FILE_DIALOG_H

#include "ui_open_quest_file_dialog.h"
#include <QStyledItemDelegate>
#include <QWidget>

namespace SolarusEditor {

class Quest;

/**
 * @brief A dialog to open a quest file.
 */
class OpenQuestFileDialog : public QDialog {
  Q_OBJECT

public:

  OpenQuestFileDialog(QWidget* parent = nullptr);

public slots:

  void open_quest_file(Quest* quest, const QPoint& position);
  virtual void done(int) override;

protected:

  virtual void keyPressEvent(QKeyEvent* event) override;

private slots:

  void update_list();

signals:

  void open_file_requested(Quest& quest, const QString& file_path);

private:

  void append_item(const QString& full_path);

  void update_item(QListWidgetItem* item, int& top);
  void update_display(
    QListWidgetItem* item, const QRegularExpressionMatch& match);

  void select_visible_item(bool to_up = false);

private:

  static const int FULL_PATH_ROLE = Qt::UserRole;
  static const int SHORT_PATH_ROLE = Qt::UserRole + 1;

  static const QString style_sheet; /**< The dialog style sheet. */

  Ui::OpenQuestFileDialog ui;       /**< The widgets. */
  Quest* quest;                     /**< The quest. */
  QMap<QString, QVariant> history;  /**< History of text to path association. */

  /**
   * @brief The item delegate for the list widget.
   *
   * This class is used to allow rich text in the list view.
   */
  class  ItemDelegate: public QStyledItemDelegate {

  public:

    ItemDelegate(QObject* parent = nullptr);

    virtual void paint(
      QPainter* painter, const QStyleOptionViewItem& option,
      const QModelIndex& index) const override;
    virtual QSize sizeHint(
      const QStyleOptionViewItem& option,
      const QModelIndex& index) const override;

  };

};

}

#endif
