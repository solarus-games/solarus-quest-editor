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
#include "gui/gui_tools.h"
#include "gui/tileset_editor.h"
#include "color.h"
#include "editor_exception.h"
#include "quest.h"
#include "quest_resources.h"
#include <QColorDialog>
#include <QUndoStack>

namespace {

/**
 * @brief Parent class of all undoable commands of the tileset editor.
 */
class TilesetEditorCommand : public QUndoCommand {

public:

  TilesetEditorCommand(TilesetEditor& editor) :
    QUndoCommand(TilesetEditor::tr("Background color")),
    editor(editor) {
  }

  TilesetEditor& get_editor() const {
    return editor;
  }

  TilesetData& get_tileset() const {
    return editor.get_tileset();
  }

private:

  TilesetEditor& editor;

};

/**
 * @brief Changing the background color of the tileset.
 */
class SetBackgroundCommand : public TilesetEditorCommand {

public:

  SetBackgroundCommand(TilesetEditor& editor, const QColor& color) :
    TilesetEditorCommand(editor),
    color_before(Color::to_qcolor(get_tileset().get_background_color())),
    color_after(color) {
  }

  virtual void undo() override {
    get_tileset().set_background_color(Color::to_solarus_color(color_before));
    get_editor().update_background_color();
  }

  virtual void redo() override {
    get_tileset().set_background_color(Color::to_solarus_color(color_after));
    get_editor().update_background_color();
  }

private:

  QColor color_before;
  QColor color_after;

};

}

/**
 * @brief Creates a tileset editor.
 * @param quest The quest containing the file.
 * @param path Path of the tileset data file to open.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
TilesetEditor::TilesetEditor(Quest& quest, const QString& path, QWidget* parent) :
  Editor(quest, path, parent),
  tileset() {

  // Get the tileset.
  ResourceType resource_type;
  QString tileset_id;
  quest.check_exists(path);
  if (!quest.is_resource_element(path, resource_type, tileset_id) ||
      resource_type != ResourceType::TILESET) {
    throw EditorException("File '%1' is not a tileset");
  }
  this->tileset_id = tileset_id;

  // Editor properties.
  set_title("Tileset " + get_file_name_without_extension());
  set_icon(QIcon(":/images/icon_resource_tileset.png"));
  set_close_confirm_message(
        tr("Tileset '%1' has been modified. Save changes?").arg(tileset_id));

  // Prepare the gui.
  ui.setupUi(this);
  layout()->addWidget(ui.splitter);
  const int side_width = 300;
  ui.splitter->setSizes(QList<int>() << side_width << width() - side_width);

  // Open the file.
  load();
  get_undo_stack().setClean();

  // Make connections.
  connect(&get_resources(), SIGNAL(element_description_changed(ResourceType, const QString&, const QString&)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));
  connect(ui.background_button, SIGNAL(clicked()),
          this, SLOT(background_button_clicked()));
}

/**
 * @brief Returns the tileset data being edited.
 * @return The tileset.
 */
TilesetData& TilesetEditor::get_tileset() {
  return tileset;
}

/**
 * @brief Loads the tileset file into the gui.
 */
void TilesetEditor::load() {

  QString path = get_file_path();

  if (!tileset.import_from_file(path.toStdString())) {
    throw EditorException(tr("Cannot open tileset data file '%1'").arg(path));
  }
  update();  // Fill the gui.
}

/**
 * @copydoc Editor::save
 */
void TilesetEditor::save() {

  QString path = get_file_path();

  if (!tileset.export_to_file(path.toStdString())) {
    throw EditorException(tr("Cannot save tileset data file '%1'").arg(path));
  }
}

/**
 * @copydoc Editor::cut
 */
void TilesetEditor::cut() {

  // TOOD
}

/**
 * @copydoc Editor::copy
 */
void TilesetEditor::copy() {

  // TOOD
}

/**
 * @copydoc Editor::paste
 */
void TilesetEditor::paste() {

  // TOOD
}

/**
 * @brief Updates everything in the gui.
 */
void TilesetEditor::update() {

  ui.tileset_id_field->setText(tileset_id);
  update_description_to_gui();
  update_background_color();
  ui.num_tiles_field->setText(QString::number(tileset.get_num_patterns()));
}

/**
 * @brief Updates the background color button from the one of the tileset.
 */
void TilesetEditor::update_background_color() {

  QString style_sheet =
      "QPushButton {\n"
      "    background-color: %1;\n"
      "    border-style: inset;\n"
      "    border-width: 2px;\n"
      "    border-color: gray;\n"
      "    min-width: 1em;\n"
      "    padding: 1px;\n"
      "}";
  QColor background_color = Color::to_qcolor(tileset.get_background_color());
  ui.background_button->setStyleSheet(
        style_sheet.arg(background_color.name()));
}

/**
 * @brief Updates the content of the tileset description text edit.
 */
void TilesetEditor::update_description_to_gui() {

  QString description = get_resources().get_description(ResourceType::TILESET, tileset_id);
  if (ui.description_field->text() != description) {
    ui.description_field->setText(description);
  }
}

/**
 * @brief Slot called when the user clicks the background color button.
 */
void TilesetEditor::background_button_clicked() {

  QColor color = QColorDialog::getColor(
        Color::to_qcolor(tileset.get_background_color()), this, "Select tileset background color");
  if (!color.isValid()) {
    return;
  }

  get_undo_stack().push(new SetBackgroundCommand(*this, color));
}

/**
 * @brief Modifies the tileset description in the quest resource list with
 * the new text entered by the user.
 *
 * If the new description is invalid, an error dialog is shown.
 */
void TilesetEditor::set_description_from_gui() {

  QString description = ui.description_field->text();
  if (description == get_resources().get_description(ResourceType::TILESET, tileset_id)) {
    return;
  }

  if (description.isEmpty()) {
    GuiTools::errorDialog("Invalid description");
    update_description_to_gui();
    return;
  }

  blockSignals(true);  // TODO scoped blocker
  try {
    get_resources().set_description(ResourceType::TILESET, tileset_id, description);
    get_resources().save();
  }
  catch (const EditorException& ex) {
    ex.print_message();
  }
  blockSignals(false);
}
