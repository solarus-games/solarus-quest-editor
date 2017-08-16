/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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
#include "entities/entity_traits.h"
#include "widgets/editor.h"
#include "editor_exception.h"
#include "quest.h"
#include <solarus/SolarusFatal.h>
#include <QApplication>
#include <QMessageBox>
#include <QUndoStack>
#include <QVBoxLayout>
#include <iostream>

namespace SolarusEditor {

namespace {

/**
 * @brief Undo command that wraps another one and does nothing the first time.
 *
 * This allows to use a command that was already executed once before being
 * pushed on a QUndoStack.
 *
 * Once a command is in a QUndoStack, redo() and undo() should never throw
 * exceptions, because it would let the history in an undefined state
 * (with a command partially done).
 * However, if it happens anyway, this class catches the exception and prints
 * an error message.
 */
class UndoCommandSkipFirst : public QUndoCommand {

public:

  /**
   * @brief Constructor.
   * @param undo_stack The undo stack.
   * @param wrapped_command The undo command to wrap.
   */
  explicit UndoCommandSkipFirst(std::unique_ptr<QUndoCommand> wrapped_command):
    QUndoCommand(wrapped_command->text()),
    wrapped_command(std::move(wrapped_command)),
    first_time(true) {

  }

  /**
   * @brief Calls undo() on the wrapped command.
   *
   * The wrapped undo() should not throw exceptions: if it does, this function
   * catches it because it is better than a crash, but the undo stack is then
   * unrecoverable.
   */
  void undo() override {

    try {
      wrapped_command->undo();
    }
    catch (const std::exception& ex) {
      // This is a bug in the editor.
      std::cerr << "Error in undo(): " << ex.what() << std::endl;
    }
  }

  /**
   * @brief Calls redo() on the wrapped command.
   *
   * Does nothing the first time, because the command was already done once.
   *
   * The wrapped redo() should not throw exceptions if it did not the first
   * time. If it does anyway, this function catches it because it is better
   * than a crash, but the undo stack is then unrecoverable.
   */
  void redo() override {

    if (first_time) {
      // Not a real redo.
      first_time = false;
      return;
    }

    try {
      wrapped_command->redo();
    }
    catch (const std::exception& ex) {
      // This is a bug in the editor.
      std::cerr << "Error in redo(): " << ex.what() << std::endl;
    }
  }

  /**
   * @brief Returns a value identifying this type of command.
   * @return 0.
   */
  int id() const override {
    return 0;
  }

  /**
   * @brief Attempts to merge this command with another.
   *
   * If the other command is also an UndoCommandSkipFirst, this function
   * attempts to merge both wrapped comands.
   *
   * @param other Another command.
   * @return @c true if they could be merged.
   */
  bool mergeWith(const QUndoCommand* other) override {

    if (other->id() != id()) {
      return false;
    }
    const UndoCommandSkipFirst& other_skip = *static_cast<const UndoCommandSkipFirst*>(other);

    return wrapped_command->mergeWith(other_skip.wrapped_command.get());
  }

private:

  std::unique_ptr<QUndoCommand>
      wrapped_command;     /**< The text editor widget to
                            * forward undo/redo commands to. */
  bool first_time;         /**< \c true if redo has not been called yet. */
};

}

/**
 * @brief Creates an editor.
 * @param quest The quest the file to edit belongs to.
 * @param file_path Path of the file to edit.
 * @param parent The parent object or nullptr.
 */
Editor::Editor(Quest& quest, const QString& file_path, QWidget* parent) :
  QWidget(parent),
  quest(quest),
  file_path(file_path),
  title(get_file_name()),
  undo_stack(new QUndoStack(this)),
  common_actions(),
  select_all_supported(false),
  find_supported(false),
  zoom_supported(false),
  grid_supported(false),
  min_layer_supported(0),
  max_layer_supported(-1),
  traversables_visibility_supported(false),
  obstacles_visibility_supported(false),
  entity_type_visibility_supported(false),
  view_settings() {

  setFocusPolicy(Qt::StrongFocus);

  // Default close confirmation message.
  set_close_confirm_message(
        tr("File '%1' has been modified. Save changes?").arg(get_file_name()));

  connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)),
          this, SLOT(application_state_changed(Qt::ApplicationState)));
}

/**
 * @brief Returns the quest the edited file belongs to.
 * @return The quest.
 */
const Quest& Editor::get_quest() const {
  return quest;
}

/**
 * @brief Returns the quest the edited file belongs to.
 *
 * Non-const version.
 *
 * @return The quest.
 */
Quest& Editor::get_quest() {
  return quest;
}

/**
 * @brief Returns the list of resources of the quest.
 * @return The quest resources.
 */
const QuestResources& Editor::get_resources() const {
  return quest.get_resources();
}

/**
 * @brief Returns the list of resources of the quest.
 *
 * Non-const version.
 *
 * @return The quest resources.
 */
QuestResources& Editor::get_resources() {
  return quest.get_resources();
}

/**
 * @brief Returns the path of the file being edited.
 * @return The file path.
 */
QString Editor::get_file_path() const {
  return file_path;
}

/**
 * @brief Returns the name of the file being edited.
 *
 * The file name is the last component of the path.
 *
 * @return The file name.
 */
QString Editor::get_file_name() const {
  return file_path.section('/', -1, -1, QString::SectionSkipEmpty);
}

/**
 * @brief Returns the name of the file being edited, without extension.
 *
 * The file name is the last component of the path.
 *
 * @return The file name without extension.
 */
QString Editor::get_file_name_without_extension() const {
  return get_file_name().section('.', 0, -2);
}

/**
 * @fn Editor::get_title
 * @brief Returns a user-friendly title for this editor.
 *
 * It is used in the tab label.
 * An asterisk is automatically added in the tab title if the file is modified.
 *
 * The default title is the file name (without its directories).
 *
 * @return A title describing the file edited.
 */
QString Editor::get_title() const {
  return title;
}

/**
 * @brief Sets the title of this editor.
 * @param title The title to set.
 */
void Editor::set_title(const QString& title) {
  this->title = title;
}

/**
 * @brief Returns an icon representing this editor.
 *
 * It is used in the tab bar.
 *
 * @return An icon representing the file edited.
 */
QIcon Editor::get_icon() const {
  return icon;
}

/**
 * @brief Sets the icon representing this editor.
 *
 * It is used in the tab bar.
 *
 * @param icon An icon representing the file edited.
 */
void Editor::set_icon(const QIcon& icon) {
  this->icon = icon;
}

/**
 * @brief Returns the message proposing to save changes when closing.
 * @return The closing confirmation message.
 */
QString Editor::get_close_confirm_message() const {
  return close_confirm_message;
}

/**
 * @brief Returns the message proposing to save changes when closing.
 * @param message The closing confirmation message.
 */
void Editor::set_close_confirm_message(const QString& message) {
  this->close_confirm_message = message;
}

/**
 * @brief Returns the undo/redo history of editing this file.
 * @return The undo/redo history.
 */
const QUndoStack& Editor::get_undo_stack() const {
  return *undo_stack;
}

/**
 * @overload
 *
 * Non-const version.
 */
QUndoStack& Editor::get_undo_stack() {
  return *undo_stack;
}

/**
 * @brief Returns the actions available to all editors.
 * @return The common actions.
 */
const QMap<QString, QAction*>& Editor::get_common_actions() const {
  return common_actions;
}

/**
 * @brief Sets the actions available to all editors.
 *
 * This function should be called at initialization time to make these actions
 * available.
 *
 * @param common_actions The common actions.
 */
void Editor::set_common_actions(const QMap<QString, QAction*>& common_actions) {
  this->common_actions = common_actions;
}

/**
 * @brief Attempts to execute a command.
 *
 * Only adds it to the undo/redo history if everything is okay.
 * Otherwise, shows an error dialog if an exception occurs.
 *
 * @param command The command to do.
 * This function takes ownership of the pointer.
 * TODO unique_ptr
 * @return @c true in case of success, @c false if an exception occurred.
 */
bool Editor::try_command(QUndoCommand* command) {

  std::unique_ptr<QUndoCommand> command_ptr(command);
  try {
    command_ptr->redo();  // Exceptions are allowed here.
    // Now we know that the command succeeds.
    // Unfortunately, we cannot directly add it to the undo stack because
    // the undo stack would execute it again.
    // So let's wrap it in a special command.
    get_undo_stack().push(new UndoCommandSkipFirst(std::move(command_ptr)));
    return true;
  }
  catch (const EditorException& ex) {
    // Error from the user.
    ex.show_dialog();
  }
  catch (const Solarus::SolarusFatal& ex) {
    // Internal error of the Solarus library.
    std::cerr << ex.what() << std::endl;
  }
  catch (const std::exception& ex) {
    // Other error.
    std::cerr << ex.what() << std::endl;
  }

  return false;
}

/**
 * @brief Undoes the last command from the undo/redo history.
 */
void Editor::undo() {

  get_undo_stack().undo();
}


/**
 * @fn Editor::save
 * @brief Saves the file.
 *
 * You don't have to call QUndoStack::setClean() from your save() function:
 * this is automatically done if the save operation is successful.
 *
 * @throws EditorException In case of failure.
 */

/**
 * @brief Returns whether the user made changes that are not saved yet.
 * @return @c true if there are unsaved changes.
 */
bool Editor::has_unsaved_changes() const {

  return !get_undo_stack().isClean();
}

/**
 * @brief Function called when the user wants to close the editor.
 *
 * If the file is not saved, a dialog proposes to save it.
 *
 * @return @c false to cancel the closing operation.
 */
bool Editor::confirm_before_closing() {

  if (!has_unsaved_changes()) {
    // The file is saved.
    return true;
  }

  QMessageBox::StandardButton answer = QMessageBox::question(
        nullptr,
        tr("Save changes"),
        get_close_confirm_message(),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
        );

  switch (answer) {

  case QMessageBox::Save:
    // Save and close.
    try {
      save();
      get_undo_stack().setClean();
      return true;
    }
    catch (const EditorException& ex) {
      ex.show_dialog();
      return false;
    }

  case QMessageBox::Discard:
    // Close without saving.
    return true;

  case QMessageBox::Cancel:
  case QMessageBox::Escape:
    // Don't close.
    return false;

  default:
    return false;
  }

}

/**
 * @brief Returns whether a cut action is currently possible.
 *
 * The default implementation returns can_copy().
 */
bool Editor::can_cut() const {
  return can_copy();
}

/**
 * @brief Copies the selection to the clipboard and removes it.
 *
 * The default implementation does nothing, meaning that clipboard operations
 * are not supported.
 */
void Editor::cut() {
}

/**
 * @brief Returns whether a copy action is currently possible.
 *
 * The default implementation returns false, meaning that clipboard operations
 * are not supported.
 */
bool Editor::can_copy() const {
  return false;
}

/**
 * @brief Copies the selection to the clipboard.
 *
 * The default implementation does nothing, meaning that clipboard operations
 * are not supported.
 */
void Editor::copy() {
}

/**
 * @brief Returns whether a paste action is currently possible.
 *
 * The default implementation returns false, meaning that clipboard operations
 * are not supported.
 */
bool Editor::can_paste() const {
  return false;
}

/**
 * @brief Inserts the content of the clipboard.
 *
 * The default implementation does nothing, meaning that clipboard operations
 * are not supported.
 */
void Editor::paste() {
}

/**
 * @brief Returns whether this editor supports selecting all.
 * @return @c true if selecting all is supported.
 */
bool Editor::is_select_all_supported() const {
  return select_all_supported;
}

/**
 * @brief Sets whether this editor supports selecting all.
 *
 * If your editor supports find operations, you are responsible to
 * reimplement select_all().
 *
 * @param select_all_supported @c true to support selecting all.
 */
void Editor::set_select_all_supported(bool select_all_supported) {
  this->select_all_supported = select_all_supported;
}

/**
 * @brief Returns whether this editor supports find operations.
 * @return @c true if find operations are supported.
 */
bool Editor::is_find_supported() const {
  return find_supported;
}

/**
 * @brief Sets whether this editor supports finding things.
 *
 * If your editor supports find operations, you are responsible to
 * reimplement find().
 *
 * @param find_supported @c true to support find operations.
 */
void Editor::set_find_supported(bool find_supported) {
  this->find_supported = find_supported;
}

/**
 * @brief Performs a select all operation.
 *
 * The default implementation does nothing.
 * Subclasses that support select all should reimplement this function.
 */
void Editor::select_all() {
}

/**
 * @brief Performs an unselect all operation.
 *
 * The default implementation does nothing.
 * Subclasses that support unselect all should reimplement this function.
 */
void Editor::unselect_all() {
}

/**
 * @brief Performs a find operation.
 *
 * The default implementation does nothing.
 * Subclasses that support finding should reimplement this function.
 */
void Editor::find() {
}

/**
 * @brief Reloads the editor settings.
 *
 * The default implementation does nothing, meaning that settings are not
 * supported.
 */
void Editor::reload_settings() {
}

/**
 * @brief Returns whether this editor supports zooming.
 * @return @c true if zoom operations are supported.
 */
bool Editor::is_zoom_supported() const {
  return zoom_supported;
}

/**
 * @brief Sets whether this editor supports zooming.
 *
 * If your editor supports zooming, you are responsible to apply the new
 * zoom value when the zoom_changed() is signal emitted.
 *
 * @param zoom_supported @c true if zoom operations are supported.
 */
void Editor::set_zoom_supported(bool zoom_supported) {
  this->zoom_supported = zoom_supported;
}

/**
 * @brief Returns whether this editor supports showing and hiding a grid.
 * @return @c true if a grid can be shown or hidden.
 */
bool Editor::is_grid_supported() const {
  return grid_supported;
}

/**
 * @brief Sets whether this editor supports showing and hiding a grid.
 *
 * If your editor supports this, you are responsible to apply the new
 * setting when the grid_visibility_changed() signal is emitted.
 *
 * @param zoom_supported @c true if zoom operations are supported.
 */
void Editor::set_grid_supported(bool grid_supported) {
  this->grid_supported = grid_supported;
}

/**
 * @brief Returns whether this editor supports showing and hiding layers.
 * @return @c true if layers can be shown or hidden.
 */
bool Editor::is_layer_supported() const {
  return max_layer_supported != -1;
}

/**
 * @brief Returns the range of layers, if visibility is supported.
 *
 * Returns an empty range if layer visibility is not supported.
 *
 * @param[out] min_layer The lowest layer.
 * @param[out] max_layer The highest layer.
 */
void Editor::get_layers_supported(int& min_layer, int& max_layer) const {

  min_layer = this->min_layer_supported;
  max_layer = this->max_layer_supported;
}

/**
 * @brief Sets the range of layers that can be shown or hidden.
 *
 * If your editor supports this, you are responsible to apply the new
 * setting when the layer_visibility_changed() signal is emitted.
 *
 * Set an empty range if you don't want to support layer visibility.
 *
 * @param min_layer The lowest layer.
 * @param max_layer The highest layer.
 */
void Editor::set_layers_supported(int min_layer, int max_layer) {

  if (min_layer > max_layer) {
    // Empty range: layer visibility is not supported.
    min_layer = 0;
    max_layer = -1;
  }

  this->min_layer_supported = min_layer;
  this->max_layer_supported = max_layer;
}

/**
 * @brief Returns whether this editor supports showing and hiding traversables.
 * @return @c true if traversables can be shown or hidden.
 */
bool Editor::is_traversables_visibility_supported() const {
  return traversables_visibility_supported;
}

/**
 * @brief Sets whether this editor supports showing and hiding traversables.
 *
 * If your editor supports this, you are responsible to apply the new
 * setting when the traversables_visibility_changed() signal is emitted.
 *
 * @param supported @c true if show traversables can be shown and hidden.
 */
void Editor::set_traversables_visibility_supported(bool supported) {
  this->traversables_visibility_supported = supported;
}

/**
 * @brief Returns whether this editor supports showing and hiding obstacles.
 * @return @c true if obstacles can be shown or hidden.
 */
bool Editor::is_obstacles_visibility_supported() const {
  return obstacles_visibility_supported;
}

/**
 * @brief Sets whether this editor supports showing and hiding obstacles.
 *
 * If your editor supports this, you are responsible to apply the new
 * setting when the obstacles_visibility_changed() signal is emitted.
 *
 * @param supported @c true if show obstacles can be shown and hidden.
 */
void Editor::set_obstacles_visibility_supported(bool supported) {
  this->obstacles_visibility_supported = supported;
}

/**
 * @brief Returns whether this editor supports showing and hiding entity types.
 * @return @c true if entity types can be shown or hidden.
 */
bool Editor::is_entity_type_visibility_supported() const {
  return entity_type_visibility_supported;
}

/**
 * @brief Sets whether this editor supports showing and hiding entity types.
 *
 * If your editor supports this, you are responsible to apply the new
 * setting when the entity_type_visibility_changed() signal is emitted.
 *
 * @param supported @c true if show entity types operations are
 * supported.
 */
void Editor::set_entity_type_visibility_supported(bool supported) {
  this->entity_type_visibility_supported = supported;
}

/**
 * @brief Returns the view settings of this editor.
 * @return The view settings.
 */
const ViewSettings& Editor::get_view_settings() const {
  return view_settings;
}

/**
 * @overload Non-const version.
 */
ViewSettings& Editor::get_view_settings() {
  return view_settings;
}

/**
 * @brief Function called when this editor becomes active.
 *
 * This happens if the application gets active and the editor is the active one.
 */
void Editor::editor_made_visible() {

}

/**
 * @brief Event called when this editor gets focused.
 * @param event The focuse in event.
 */
void Editor::focusInEvent(QFocusEvent* event) {

  Q_UNUSED(event);
  editor_made_visible();
}

/**
 * @brief Slot called when the application state changes.
 * @param event The new state.
 */
void Editor::application_state_changed(Qt::ApplicationState state) {

  if (state != Qt::ApplicationActive) {
    return;
  }

  // Check if the tileset image has changed.
  if (isVisible()) {
    editor_made_visible();
  }
}

}
