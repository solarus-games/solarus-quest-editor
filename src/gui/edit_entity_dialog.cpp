/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#include "include/gui/edit_entity_dialog.h"
#include "map_model.h"

namespace {

// Put field names in constants to avoid repeated identical literals.

const QString destination_field_name = "destination";
const QString destination_map_field_name = "destination_map";
const QString sound_field_name = "sound";
const QString sprite_field_name = "sprite";
const QString transition_field_name = "transition";
const QString treasure_name_field_name = "treasure_name";
const QString treasure_variant_field_name = "treasure_variant";
const QString treasure_savegame_variable_field_name = "treasure_savegame_variable";

}  // Anonymous namespace.

/**
 * @brief Creates an edit entity dialog.
 * @param entity_before The entity to edit. It may or may not already be on the map.
 * @param parent The parent widget or nullptr.
 */
EditEntityDialog::EditEntityDialog(EntityModel& entity_before, QWidget* parent) :
  QDialog(parent),
  entity_before(entity_before) {

  ui.setupUi(this);

  initialize();
}

/**
 * @brief Returns the quest the map belongs to.
 * @return The quest.
 */
Quest& EditEntityDialog::get_quest() const {
  return get_map().get_quest();
}

/**
 * @brief Returns the map the entity edited belongs to.
 * @return The map.
 */
MapModel& EditEntityDialog::get_map() const {
  return get_entity_before().get_map();
}

/**
 * @brief Returns the index of the entity being edited.
 * @return The entity index on the map before any change.
 */
EntityIndex EditEntityDialog::get_entity_index() const {
  return entity_before.get_index();
}

/**
 * @brief Returns the entity being edited, before any change.
 * @return The entity edited.
 */
EntityModel& EditEntityDialog::get_entity_before() const {
  return entity_before;
}

/**
 * @brief Creates and returns an entity representing the new input data.
 *
 * The created entity is not on the map.
 *
 * @return The new data.
 */
EntityModelPtr EditEntityDialog::get_entity_after() {

  entity_after = std::move(EntityModel::clone(get_map(), entity_before.get_index()));
  apply();
  return std::move(entity_after);
}

/**
 * @brief Fills the fields from the existing entity.
 */
void EditEntityDialog::initialize() {

  initialize_simple_booleans();

  initialize_destination();
  initialize_destination_map();
  initialize_direction();
  initialize_layer();
  initialize_name();
  initialize_size();
  initialize_sound();
  initialize_sprite();
  initialize_subtype();
  initialize_transition();
  initialize_treasure();
  initialize_type();
  initialize_xy();

  adjustSize();
}

/**
 * @brief Applies the data in the GUI to the entity.
 */
void EditEntityDialog::apply() {

  apply_simple_booleans();

  apply_destination();
  apply_destination_map();
  apply_direction();
  apply_layer();
  apply_name();
  apply_size();
  apply_sound();
  apply_sprite();
  apply_subtype();
  apply_transition();
  apply_treasure();
  apply_type();
  apply_xy();
}

/**
 * @brief Sets up the behavior of a field with a checkbox or a label depending
 * on whether it is optional.
 * @param field_name The field to initialize.
 * @param label_layout Parent layout of the checkbox and label to handle
 * (one of them is removed). If nullptr, nothing is removed.
 * @param label A label to be used when the field is mandatory.
 * Can be nullptr.
 * @param checkbox A checkbox to be used instead of the label when the field
 * is optional. Can be nullptr.
 * @param field The field widget to disable when the checkbox is disabled.
 * Can be nullptr.
 */
void EditEntityDialog::initialize_possibly_optional_field(const QString& field_name,
                                                          QLayout* label_layout,
                                                          QWidget* label,
                                                          QCheckBox* checkbox,
                                                          QWidget* field) {

  if (!entity_before.is_field_optional(field_name)) {
    // Mandatory field: remove the checkbox if any, keep the label.
    if (label_layout != nullptr && checkbox != nullptr) {
      checkbox->setChecked(true);  // Make it check even if hidden to simplify apply_xxx() functions.
      checkbox->hide();
      label_layout->removeWidget(checkbox);
    }
    return;
  }

  // Optional field: remove the label if any, keep the checkbox.
  if (label_layout != nullptr && label != nullptr) {
    label->hide();
    label_layout->removeWidget(label);
  }

  bool unset = entity_before.is_field_unset(field_name);

  if (unset) {
    if (field != nullptr) {
      field->setEnabled(false);
    }
  }
  else {
    if (checkbox != nullptr) {
      checkbox->setChecked(true);
    }
  }
  if (checkbox != nullptr && field != nullptr) {
    connect(checkbox, SIGNAL(toggled(bool)),
            field, SLOT(setEnabled(bool)));
  }
}

/**
 * @brief Removes a row of the form layout.
 * @param label Label of the row.
 * @param field Field of the row.
 */
void EditEntityDialog::remove_field(QWidget* label, QWidget* field) {

  label->hide();
  ui.form_layout->removeWidget(label);
  field->hide();
  ui.form_layout->removeWidget(field);
}

/**
 * @brief Initializes the simple boolean fields.
 */
void EditEntityDialog::initialize_simple_booleans() {

  // TODO when setting a destination as the default one, unset the previous one.
  simple_boolean_fields <<
    SimpleBooleanField("enabled_at_start", tr("Visiblity"), tr("Enabled at start")) <<
    SimpleBooleanField("default", tr("Default"), tr("Set as the default destination")) <<
    SimpleBooleanField("can_be_cut", tr("Cutting the object"), tr("Can be cut")) <<
    SimpleBooleanField("can_explode", tr("Exploding"), tr("Can explode")) <<
    SimpleBooleanField("can_regenerate", tr("Regeneration"), tr("Can regenerate")) <<
    SimpleBooleanField("pushable", tr("Interactions"), tr("Can be pushed")) <<
    SimpleBooleanField("pullable", "", tr("Can be pulled")) <<
    SimpleBooleanField("needs_block", tr("Activation"), tr("Requires a block to be activated")) <<
    SimpleBooleanField("inactivate_when_leaving", tr("Stay on switch"), tr("Inactivate the switch when leaving")) <<
    SimpleBooleanField("stops_hero", tr("Hero"), tr("Obstacle for the hero")) <<
    SimpleBooleanField("stops_enemies", tr("Enemies"), tr("Obstacle for enemies")) <<
    SimpleBooleanField("stops_npcs", tr("NPCs"), tr("Obstacle for NPCs")) <<
    SimpleBooleanField("stops_blocks", tr("Blocks"), tr("Obstacle for blocks")) <<
    SimpleBooleanField("stops_projectiles", tr("Projectiles"), tr("Obstacle for projectiles")) <<
    SimpleBooleanField("allow_movement", tr("Movements"), tr("Allow to move")) <<
    SimpleBooleanField("allow_attack", tr("Sword"), tr("Allow to use the sword")) <<
    SimpleBooleanField("allow_item", tr("Items"), tr("Allow to use equipment items"));

  for (SimpleBooleanField& field : simple_boolean_fields) {
    if (entity_before.has_field(field.field_name)) {
      QLabel* label = new QLabel(field.label_text, this);
      QCheckBox* checkbox = new QCheckBox(field.checkbox_text, this);
      checkbox->setChecked(entity_before.get_field(field.field_name).toBool());
      field.checkbox = checkbox;
      ui.form_layout->addRow(label, checkbox);
    }
  }
}

/**
 * @brief Updates the entity from the simple boolean fields.
 */
void EditEntityDialog::apply_simple_booleans() {

  for (const SimpleBooleanField& field : simple_boolean_fields) {
    if (entity_before.has_field(field.field_name)) {
      entity_after->set_field(field.field_name, field.checkbox->isChecked());
    }
  }
}

/**
 * @brief Initializes the destination field.
 */
void EditEntityDialog::initialize_destination() {

  if (!entity_before.has_field(destination_field_name)) {
    remove_field(ui.destination_label, ui.destination_field);
    return;
  }

  QString destination_map_id = entity_before.get_field(destination_map_field_name).toString();
  ui.destination_field->set_map_id(get_quest(), destination_map_id);
  ui.destination_field->set_filtered_by_entity_type(true);
  ui.destination_field->set_entity_type_filter(EntityType::DESTINATION);
  ui.destination_field->add_special_value("", "(Default destination)");
  ui.destination_field->add_special_value("_same", "(Same point)");
  ui.destination_field->add_special_value("_side", "(Side of the map)");
  ui.destination_field->build();
  ui.destination_field->set_selected_name(entity_before.get_field(destination_field_name).toString());
}

/**
 * @brief Updates the entity from the destination field.
 */
void EditEntityDialog::apply_destination() {

  if (entity_before.has_field(destination_field_name)) {
    entity_after->set_field(destination_field_name, ui.destination_field->get_selected_name());
  }
}

/**
 * @brief Initializes the destination map field.
 */
void EditEntityDialog::initialize_destination_map() {

  if (!entity_before.has_field(destination_map_field_name)) {
    remove_field(ui.destination_map_label, ui.destination_map_field);
    return;
  }

  ui.destination_map_field->set_quest(get_quest());
  ui.destination_map_field->set_resource_type(ResourceType::MAP);
  ui.destination_map_field->set_selected_id(entity_before.get_field(destination_map_field_name).toString());

  connect(ui.destination_map_field, &QComboBox::currentTextChanged, [&](const QString&) {
    QString map_id = ui.destination_map_field->currentData().toString();
    ui.destination_field->set_map_id(get_quest(), map_id);
    ui.destination_field->build();
  });
}

/**
 * @brief Updates the entity from the destination map field.
 */
void EditEntityDialog::apply_destination_map() {

  if (entity_before.has_field(destination_map_field_name)) {
    entity_after->set_field(destination_map_field_name, ui.destination_map_field->get_selected_id());
  }
}

/**
 * @brief Initializes the direction field.
 */
void EditEntityDialog::initialize_direction() {

  if (!entity_before.has_direction_field()) {
    remove_field(ui.direction_label, ui.direction_layout);
    return;
  }

  int num_directions = entity_before.get_num_directions();
  if (entity_before.is_no_direction_allowed()) {
    ui.direction_field->addItem(entity_before.get_no_direction_text(), -1);
  }

  QStringList texts;
  if (num_directions == 4) {
    texts = QStringList{
      tr("Right"),
      tr("Up"),
      tr("Left"),
      tr("Down")
    };
  }
  else if (num_directions == 8) {
    texts = QStringList{
      tr("Right"),
      tr("Right-up"),
      tr("Up"),
      tr("Left-up"),
      tr("Left"),
      tr("Left-down"),
      tr("Down"),
      tr("Right-down"),
    };
  }
  else {
    for (int i = 0; i < num_directions; ++num_directions) {
      texts.append(QString::number(i));
    }
  }
  for (int i = 0; i < texts.size(); ++i) {
    ui.direction_field->addItem(texts[i], i);
  }

  int index = entity_before.get_direction();
  if (entity_before.is_no_direction_allowed()) {
    ++index;
  }
  ui.direction_field->setCurrentIndex(index);
}

/**
 * @brief Updates the entity from the direction field.
 */
void EditEntityDialog::apply_direction() {

  if (entity_after->has_direction_field()) {
    entity_after->set_direction(ui.direction_field->currentData().toInt());
  }
}

/**
 * @brief Initializes the layer field.
 */
void EditEntityDialog::initialize_layer() {

  ui.layer_field->set_selected_value(entity_before.get_layer());
}

/**
 * @brief Updates the entity from the layer field.
 */
void EditEntityDialog::apply_layer() {

  entity_after->set_layer(ui.layer_field->get_selected_value());
}

/**
 * @brief Initializes the name field.
 */
void EditEntityDialog::initialize_name() {

  if (!entity_before.is_dynamic()) {
    remove_field(ui.name_label, ui.name_field);
    return;
  }

  ui.name_field->setText(entity_before.get_name());
}

/**
 * @brief Updates the entity from the name field.
 */
void EditEntityDialog::apply_name() {

  entity_after->set_name(ui.name_field->text());
}

/**
 * @brief Initializes the size fields.
 */
void EditEntityDialog::initialize_size() {

  if (!entity_before.has_size_fields()) {
    remove_field(ui.size_label, ui.size_layout);
    return;
  }

  // Show the current size in the spinboxes.
  const QSize& size = entity_before.get_size();
  ui.width_field->setValue(size.width());
  ui.height_field->setValue(size.height());

  // Tell spinboxes to only make multiples of the base size.
  const QSize& base_size = entity_before.get_base_size();
  ui.width_field->setMinimum(base_size.width());
  ui.width_field->setSingleStep(base_size.width());
  ui.height_field->setMinimum(base_size.height());
  ui.height_field->setSingleStep(base_size.height());
}

/**
 * @brief Updates the entity from the size fields.
 */
void EditEntityDialog::apply_size() {

  if (entity_after->has_size_fields()) {
    entity_after->set_size(QSize(ui.width_field->value(), ui.height_field->value()));
  }
}

/**
 * @brief Initializes the sound field.
 */
void EditEntityDialog::initialize_sound() {

  if (!entity_before.has_field(sound_field_name)) {
    remove_field(ui.sound_checkbox, ui.sound_field);
    return;
  }

  ui.sound_field->set_quest(get_quest());
  ui.sound_field->set_resource_type(ResourceType::SOUND);
  initialize_possibly_optional_field(
        sound_field_name,
        nullptr,
        nullptr,
        ui.sound_checkbox,
        ui.sound_field);
  QString sound = entity_before.get_field(sound_field_name).toString();
  ui.sound_field->set_selected_id(sound);
}

/**
 * @brief Updates the entity from the sound field.
 */
void EditEntityDialog::apply_sound() {

  if (!entity_after->has_field(sound_field_name)) {
    return;
  }

  entity_after->set_field(sound_field_name, ui.sound_checkbox->isChecked() ?
                            ui.sound_field->get_selected_id() : "");
}

/**
 * @brief Initializes the sprite field.
 */
void EditEntityDialog::initialize_sprite() {

  if (!entity_before.has_field(sprite_field_name)) {
    remove_field(ui.sprite_label_checkbox, ui.sprite_field);
    return;
  }

  initialize_possibly_optional_field(
        sprite_field_name,
        ui.sprite_label_checkbox->layout(),
        ui.sprite_label,
        ui.sprite_checkbox,
        ui.sprite_field);
  ui.sprite_field->set_quest(get_quest());
  ui.sprite_field->set_resource_type(ResourceType::SPRITE);
  QString sprite = entity_before.get_field(sprite_field_name).toString();
  ui.sprite_field->set_selected_id(sprite);
}

/**
 * @brief Updates the entity from the sprite field.
 */
void EditEntityDialog::apply_sprite() {

  if (entity_after->has_field(sprite_field_name)) {
    entity_after->set_field(sprite_field_name, ui.sprite_checkbox->isChecked() ?
                              ui.sprite_field->get_selected_id() : "");
  }
}

/**
 * @brief Initializes the subtype field.
 */
void EditEntityDialog::initialize_subtype() {

  if (!entity_before.has_subtype_field()) {
    remove_field(ui.subtype_label, ui.subtype_field);
    return;
  }

  int i = 0;
  const SubtypeList& subtypes = entity_before.get_existing_subtypes();
  for (const QPair<QString, QString>& subtype : subtypes) {
    ui.subtype_field->addItem(subtype.second, subtype.first);

    if (entity_before.get_subtype() == subtype.first) {
      ui.subtype_field->setCurrentIndex(i);
    }
    ++i;
  }
}

/**
 * @brief Updates the entity from the subtype field.
 */
void EditEntityDialog::apply_subtype() {

  if (entity_after->has_subtype_field()) {
    entity_after->set_subtype(ui.subtype_field->currentData().toString());
  }
}

/**
 * @brief Initializes the transition field.
 */
void EditEntityDialog::initialize_transition() {

  if (!entity_before.has_field(transition_field_name)) {
    remove_field(ui.transition_label, ui.transition_field);
    return;
  }

  QString transition_name = entity_before.get_field(transition_field_name).toString();
  ui.transition_field->set_selected_value(TransitionTraits::get_by_lua_name(transition_name));
}

/**
 * @brief Updates the entity from the transition field.
 */
void EditEntityDialog::apply_transition() {

  if (entity_after->has_field(transition_field_name)) {
    entity_after->set_field(transition_field_name, TransitionTraits::get_lua_name(ui.transition_field->get_selected_value()));
  }
}

/**
 * @brief Initializes the treasure field.
 */
void EditEntityDialog::initialize_treasure() {

  if (!entity_before.has_field(treasure_name_field_name) ||
      !entity_before.has_field(treasure_variant_field_name) ||
      !entity_before.has_field(treasure_savegame_variable_field_name)
  ) {
    remove_field(ui.treasure_label, ui.treasure_layout);
    return;
  }

  ui.treasure_name_field->set_quest(get_quest());
  ui.treasure_name_field->set_resource_type(ResourceType::ITEM);
  ui.treasure_name_field->add_special_value("", tr("(None)"), 0);  // Add the special value "None".
  QString treasure_name = entity_before.get_field(treasure_name_field_name).toString();
  ui.treasure_name_field->set_selected_id(treasure_name);
  ui.treasure_variant_field->setValue(entity_before.get_field(treasure_variant_field_name).toInt());
  QString treasure_savegame_variable = entity_before.get_field(treasure_savegame_variable_field_name).toString();
  if (treasure_savegame_variable.isEmpty()) {
    ui.treasure_savegame_variable_label->setEnabled(false);
    ui.treasure_savegame_variable_field->setEnabled(false);
  }
  else {
    ui.treasure_savegame_variable_field->setText(treasure_savegame_variable);
    ui.save_treasure_checkbox->setChecked(true);
  }
  connect(ui.save_treasure_checkbox, SIGNAL(toggled(bool)),
          ui.treasure_savegame_variable_label, SLOT(setEnabled(bool)));
  connect(ui.save_treasure_checkbox, SIGNAL(toggled(bool)),
          ui.treasure_savegame_variable_field, SLOT(setEnabled(bool)));
}

/**
 * @brief Updates the entity from the tresaure fields.
 */
void EditEntityDialog::apply_treasure() {

  if (!entity_after->has_field(treasure_name_field_name) ||
      !entity_after->has_field(treasure_variant_field_name) ||
      !entity_after->has_field(treasure_savegame_variable_field_name)) {
    return;
  }

  entity_after->set_field(treasure_name_field_name, ui.treasure_name_field->get_selected_id());
  entity_after->set_field(treasure_variant_field_name, ui.treasure_variant_field->value());
  entity_after->set_field(treasure_savegame_variable_field_name, ui.save_treasure_checkbox->isChecked() ?
                            ui.treasure_savegame_variable_field->text() : "");
}

/**
 * @brief Initializes the type field.
 */
void EditEntityDialog::initialize_type() {

  ui.type_field->setText(EntityTraits::get_friendly_name(entity_before.get_type()));
}

/**
 * @brief Updates the entity from the type field.
 */
void EditEntityDialog::apply_type() {

  // Nothing to do: the type is a read-only field.
  Q_ASSERT(EntityTraits::get_friendly_name(entity_after->get_type()) == ui.type_field->text());
}

/**
 * @brief Initializes the position field.
 */
void EditEntityDialog::initialize_xy() {

  QPoint xy = entity_before.get_xy();
  ui.x_field->setValue(xy.x());
  ui.y_field->setValue(xy.y());
}

/**
 * @brief Updates the entity from the position fields.
 */
void EditEntityDialog::apply_xy() {

  entity_after->set_xy(QPoint(ui.x_field->value(), ui.y_field->value()));
}

