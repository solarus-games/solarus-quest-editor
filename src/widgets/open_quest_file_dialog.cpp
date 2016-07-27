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
#include "widgets/open_quest_file_dialog.h"
#include "editor_settings.h"
#include "quest.h"
#include <QDirIterator>
#include <QKeyEvent>
#include <QPainter>
#include <QStaticText>

namespace SolarusEditor {

const QString OpenQuestFileDialog::style_sheet =
  "SolarusEditor--OpenQuestFileDialog {\n"
  "  border: 1px solid %1;"
  "  border-top: none;"
  "}";

/**
 * @brief Creates a open quest file dialog.
 * @param parent Parent object or nullptr.
 */
OpenQuestFileDialog::OpenQuestFileDialog(QWidget* parent) :
  QDialog(parent),
  quest(nullptr) {

  ui.setupUi(this);

  // Prepare the gui.
  QPalette palette;
  setStyleSheet(style_sheet.arg(palette.dark().color().name()));
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  setModal(false);
  setFocusProxy(ui.search_field);

  ui.list_widget->setFocusPolicy(Qt::NoFocus);
  ui.list_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui.list_widget->setAlternatingRowColors(true);
  ui.list_widget->setItemDelegate(new ItemDelegate);

  // Load history.
  EditorSettings settings;
  history = settings.get_value_map(EditorSettings::open_quest_file_history);

  // Make connections.
  connect(ui.search_field, SIGNAL(textChanged(QString)),
          this, SLOT(update_list()));
  connect(ui.search_field, SIGNAL(focus_out(QFocusEvent*)),
          this, SLOT(reject()));
  connect(ui.list_widget, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(accept()));
}

/**
 * @brief Shows this dialog to open a quest file.
 * @param quest The quest.
 * @param position The position where to show this dialog.
 */
void OpenQuestFileDialog::open_quest_file(
  Quest* quest, const QPoint &position) {

  this->quest = quest;
  if (quest == nullptr) {
    return;
  }

  QDirIterator it(
    quest->get_data_path(),
    QStringList() << "*.lua" << "*.dat",
    QDir::Files,
    QDirIterator::Subdirectories);

  ui.list_widget->clear();
  while (it.hasNext()) {
    append_item(it.next());
  }

  update_list();

  QRect rect = geometry();
  QPoint pos = QPoint(position.x() - rect.width() / 2, position.y());
  setGeometry(QRect(pos, rect.size()));
  show();
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void OpenQuestFileDialog::done(int result) {

  if (quest != nullptr && result == QDialog::Accepted) {
    QString full_path =
      ui.list_widget->currentIndex().data(FULL_PATH_ROLE).toString();
    QString short_path =
      ui.list_widget->currentIndex().data(SHORT_PATH_ROLE).toString();

    EditorSettings settings;
    history[ui.search_field->text()] = short_path;
    settings.set_value(EditorSettings::open_quest_file_history, history);

    emit open_file_requested(*quest, full_path);
  }

  QDialog::done(result);
}

/**
 * @brief Called when the user press a key.
 * @param event The event.
 */
void OpenQuestFileDialog::keyPressEvent(QKeyEvent* event) {

  int key = event->key();

  if (key == Qt::Key_Up || key == Qt::Key_Down) {

    int row = ui.list_widget->currentRow() + (key == Qt::Key_Down ? 1 : -1);
    if (row < 0) {
      row = ui.list_widget->count() - 1;
    }

    ui.list_widget->setCurrentRow(row);
    select_visible_item(key == Qt::Key_Up);
    event->accept();
  }
  else if (key == Qt::Key_PageUp) {
    ui.list_widget->setCurrentRow(0);
    select_visible_item();
    event->accept();
  }
  else if (key == Qt::Key_PageDown) {
    ui.list_widget->setCurrentRow(ui.list_widget->count() - 1);
    select_visible_item(true);
    event->accept();
  }
  else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
    accept();
    event->accept();
  }

  QDialog::keyPressEvent(event);
}

/**
 * @brief Updates the list widget.
 */
void OpenQuestFileDialog::update_list() {

  ui.list_widget->sortItems();
  ui.list_widget->setCurrentRow(-1);

  int top = 0;
  for (int i = 0; i < ui.list_widget->count(); ++i) {
    update_item(ui.list_widget->item(i), top);
  }

  if (ui.list_widget->currentRow() == -1) {
    select_visible_item();
  }
}

/**
 * @brief Appends an item in the list widget.
 * @param full_path The file path of the item to append.
 */
void OpenQuestFileDialog::append_item(const QString& full_path) {

  QString data_path = quest->get_data_path();
  QString short_path = QString(full_path).replace(data_path + "/", "");
  short_path.replace(QRegularExpression("\\.dat$"), "");
  QIcon icon = quest->get_file_icon(full_path);

  QListWidgetItem* item = new QListWidgetItem(icon, short_path);
  item->setData(FULL_PATH_ROLE, full_path);
  item->setData(SHORT_PATH_ROLE, short_path);

  ui.list_widget->addItem(item);
}

/**
 * @brief Updates an item.
 * @param item The item to update.
 * @param top The top position where to move an exact matched item.
 */
void OpenQuestFileDialog::update_item(QListWidgetItem* item, int& top) {

  QString text = ui.search_field->text();
  QString short_path = item->data(SHORT_PATH_ROLE).toString();
  QString pattern;
  for (const QChar& c: text) {
    pattern += QString(".*(%1)").arg(QRegularExpression::escape(QString(c)));
  }

  QRegularExpression general_regex(QString("^%1.*$").arg(pattern));
  QRegularExpressionMatch general_match = general_regex.match(short_path);
  if (general_match.hasMatch()) {

    QRegularExpression exact_regex(
      QString("^.*(%1).*$").arg(QRegularExpression::escape(text)));
    QRegularExpressionMatch exact_match = exact_regex.match(short_path);
    if (exact_match.hasMatch()) {
      ui.list_widget->takeItem(ui.list_widget->row(item));
      ui.list_widget->insertItem(top++, item);
      update_display(item, exact_match);
    } else {
      update_display(item, general_match);
    }

    // Selected if is corresponding to the history.
    if (history.contains(text)) {
      if (history[text].toString() == short_path) {
        ui.list_widget->setCurrentItem(item);
      }
    }

    item->setHidden(false);
  } else {
    item->setHidden(true);
  }
}

/**
 * @brief Update the displayed text of an item.
 * @param item The item to update.
 * @param match The match result.
 */
void OpenQuestFileDialog::update_display(
  QListWidgetItem* item, const QRegularExpressionMatch& match) {

  QString short_path = match.captured(0);
  QString display;
  int p = 0;
  for (int i = 1; i <= match.lastCapturedIndex(); ++i) {
    int start = match.capturedStart(i);
    if (p < start) {
      display += short_path.mid(p, start - p);
    }
    display += QString("<b>%1</b>").arg(match.captured(i));
    p = match.capturedEnd(i);
  }
  display += short_path.mid(p);

  item->setData(Qt::DisplayRole, display);
}

/**
 * @brief Selects the first visible item from the current one (included).
 * @param to_up Set to @c true to select by up.
 */
void OpenQuestFileDialog::select_visible_item(bool to_up) {

  int count = ui.list_widget->count();
  if (count == 0) {
    return;
  }

  int row = qBound(0, ui.list_widget->currentRow(), count);
  int row_start = row;
  QListWidgetItem* item;

  do {
    item = ui.list_widget->item(row);

    if (to_up) {
      row--;
      if (row < 0) {
        row = count - 1;
      }
    } else {
      row++;
      if (row >= count) {
        row = 0;
      }
    }

    if (row == row_start) {
      return; // no visible item.
    }
  } while(item->isHidden());

  ui.list_widget->setCurrentItem(item);
}

/**
 * @brief Create a item delegate.
 * @param parent The parent.
 */
OpenQuestFileDialog::ItemDelegate::ItemDelegate(QObject* parent) :
  QStyledItemDelegate(parent) {
}

/**
 * @brief Paints an item.
 * @param painter The painter.
 * @param option The option.
 * @param index The index of the item.
 */
void OpenQuestFileDialog::ItemDelegate::paint(
  QPainter* painter, const QStyleOptionViewItem& option,
  const QModelIndex& index) const {

  QString display = index.data(Qt::DisplayRole).toString();
  QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
  QPoint pos = option.rect.topLeft() + QPoint(2, 2);

  painter->save();

  if (option.state & QStyle::State_Selected) {
    painter->fillRect(option.rect, option.palette.highlight());
    painter->setPen(option.palette.highlightedText().color());
  }

  painter->drawPixmap(pos, icon.pixmap(16, 16));

  QStaticText text(display);
  text.setTextWidth(option.rect.width());
  painter->drawStaticText(pos.x() + 20, pos.y(), text);

  painter->restore();
}

/**
 * @brief Returns the size hint.
 * @param option The option.
 * @param index The index.
 * @return The size hint.
 */
QSize OpenQuestFileDialog::ItemDelegate::sizeHint(
  const QStyleOptionViewItem& /*option*/,
  const QModelIndex &index) const {

  QString display = index.data(SHORT_PATH_ROLE).toString();
  QFontMetrics metrics = QFontMetrics(QApplication::font());
  QSize size = metrics.boundingRect(display).size() + QSize(24, 4);
  return size;
}

}
