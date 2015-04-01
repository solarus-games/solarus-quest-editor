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
#include "indexed_string_tree.h"
#include "editor_exception.h"

/**
 * @brief Creates an indexed string tree.
 * @param separator The separator of keys to split them in the tree.
 */
IndexedStringTree::IndexedStringTree(const QString& separator) :
  separator(separator),
  root(new Node()) {
}

/**
 * @brief Returns whether a key exists.
 * @param key The key to test.
 * @return @c true if the key exists.
 */
bool IndexedStringTree::key_exists(const QString& key) const {

  return !key.isEmpty() && get_child(key) != nullptr;
}

/**
 * @brief Returns the index of a specific key in his parent.
 * @param key The key to test.
 * @return The index of the key or -1 if the key no exists.
 */
int IndexedStringTree::get_index(const QString& key) const {

  Node* node = get_child(key);
  if (node == nullptr) {
    return -1;
  }
  return node->index;
}

/**
 * @brief Returns the row count of a specific key.
 * @param key The key to test.
 * @return The row count of the key or -1 if the key no exists.
 */
int IndexedStringTree::get_row_count(const QString& key) const {

  Node* node = get_child(key);
  if (node == nullptr) {
    return -1;
  }
  return node->children.size();
}

/**
 * @brief Returns the internal key of a specific key.
 *
 * This methods is useful to create QModelIndex with the key as the
 * internal pointer for each node of the tree.
 *
 * @param key The key.
 * @return The internal key.
 */
QString* IndexedStringTree::get_internal_key(const QString& key) const {

  Node* node = get_child(key);
  if (node == nullptr) {
    return nullptr;
  }
  return &node->key;
}

/**
 * @brief Returns the key at a specific row of a specific key.
 * @param index The index of the row to test.
 * @param parent_key The key to test.
 * @return The key or an empty string if the no exists.
 */
QString IndexedStringTree::get_row_key(
    int index, const QString& parent_key) const {

  Node* node = get_child(parent_key);

  if (node == nullptr || index < 0 || index >= (int) node->children.size()) {
    return "";
  }

  auto it = node->children.begin();
  std::advance(it, index);
  return it->second->key;
}

/**
 * @brief Returns whether a key have a parent node in the tree.
 * @param key The key to test.
 * @return @c true if the key has a parent.
 * Returns @c nullptr if the key no exists or has no parent.
 */
bool IndexedStringTree::has_parent(const QString& key) const {

  Node* node = get_child(key);
  return node != nullptr && node->parent != nullptr;
}

/**
 * @brief Return the parent key of a specific key.
 * @param key The key.
 * @return The parent key.
 */
QString IndexedStringTree::get_parent(const QString& key) const {

  Node* node = get_child(key);
  if (node == nullptr || node->parent == nullptr) {
    return "";
  }
  return node->parent->key;
}

/**
 * @brief Add a key in the tree.
 * @param key[in] The key to add.
 * @param parent_key[out] The key of the first node that add a new child.
 * @param index[out] The index of the first new child that was added.
 * @return @c true in case of success, @c false if already exists.
 */
bool IndexedStringTree::add_key(
    const QString& key, QString& parent_key, int& index) {

  return add_child(key, REAL_KEY, parent_key, index);
}

/**
 * @brief Add a key in the tree.
 * @param key The key to add.
 * @return @c true in case of success, @c false if already exists.
 */
bool IndexedStringTree::add_key(const QString &key) {

  QString parent_key;
  int index;
  return add_key(key, parent_key, index);
}

/**
 * @brief Check if can remove a key from the tree.
 * @param key[in] The key to remove.
 * @param parent_key[out] The key of the parent that contains the key to remove.
 * @param index[out] The index of the key to remove in his parent.
 * @return @c true if the key can be removed, @c false if the key have childs.
 */
bool IndexedStringTree::can_remove_key(
    const QString& key, QString& parent_key, int& index) {

  return can_remove_child(key, REAL_KEY, parent_key, index);
}

/**
 * @brief Removes a key from the tree.
 * @param key The key to remove.
 * @return @c true in case of success.
 */
bool IndexedStringTree::remove_key(const QString& key) {

  return remove_child(key, REAL_KEY);
}

/**
 * @brief Add a ref in the tree.
 * @param key[in] The key of the ref to add.
 * @param parent_key[out] The key of the first node that add a new child.
 * @param index[out] The index of the first new child that was added.
 * @return @c true in case of success, @c false if already exists.
 */
bool IndexedStringTree::add_ref(
    const QString &key, QString& parent_key, int& index) {

  return add_child(key, REF_KEY, parent_key, index);
}

/**
 * @brief Check if can remove a ref from the tree.
 * @param key[in] The key of the ref to remove.
 * @param parent_key[out] The key of the parent that contains the key to remove.
 * @param index[out] The index of the ref to remove in his parent.
 * @return @c true if the ref can be removed, @c false if the ref have childs.
 */
bool IndexedStringTree::can_remove_ref(
    const QString& key, QString& parent_key, int& index) {

  return can_remove_child(key, REF_KEY, parent_key, index);
}

/**
 * @brief Removes a ref from the tree.
 * @param key The key of the ref to remove.
 * @param keep_key To transform the ref to a simple value node.
 * @return @c true in case of success.
 */
bool IndexedStringTree::remove_ref(const QString& key, bool keep_key) {

  return remove_child(key, REF_KEY, keep_key);
}

/**
 * @brief Clears the tree.
 */
void IndexedStringTree::clear() {
  clear_children(root);
}

/**
 * @brief Returns a node of the tree.
 * @param key The key of the node to get.
 * @return The node with the specified key.
 */
IndexedStringTree::Node* IndexedStringTree::get_child(const QString& key) const {

  Node* node = root;

  if (key.isEmpty()) {
    return node;
  }

  QStringList key_list = key.split(separator);
  while (node != nullptr && !key_list.isEmpty()) {

    QString sub_key = key_list.front();
    node = get_sub_child(node, sub_key);
    key_list.pop_front();
  }
  return node;
}

/**
 * @brief Returns a child of a node.
 * @param node The parent node.
 * @param sub_key The sub key of the node to get.
 * @return The node with the specified sub key.
 */
IndexedStringTree::Node* IndexedStringTree::get_sub_child(
    const Node *node, const QString& sub_key) const {

  auto it = node->children.find(sub_key);
  if (it == node->children.end()) {
    return nullptr;
  }
  return it->second;
}

/**
 * @brief Add a child (key or ref) in the tree.
 * @param key[in] The key of the child to add.
 * @param type[in] The type of the child to add.
 * @param parent_key[out] The key of the first node that add a new child.
 * @param index[out] The index of the first new child that was added.
 * @return @c true in case of success, @c false if already exists.
 */
bool IndexedStringTree::add_child(
    const QString& key, int type, QString& parent_key, int& index) {

  // Split the key with the separator.
  QStringList key_list = key.split(separator);

  // Get the first existing parent.
  Node* parent = root;
  Node* node = parent;

  while (node != nullptr && !key_list.isEmpty()) {

    node = get_sub_child(parent, key_list.front());

    if (node != nullptr) {
      parent = node;
      key_list.pop_front();
    }
  }

  // If the node already exists.
  if (node != nullptr) {
    // Set the node type, return false.
    if (type == REF_KEY || node->type == CONTAINER) {
      node->type = type;
    }
    return false;
  }

  // Set the key of the first existing parent, clear the index.
  parent_key = parent->key;
  index = -1;

  // Add missing childs.
  while (!key_list.isEmpty()) {

    QString sub_key = key_list.front();

    // Create the new node.
    node = new Node();
    node->parent = parent;
    if (!parent->key.isEmpty()) {
      node->key = parent->key + separator + sub_key;
    } else {
      node->key = sub_key;
    }

    // Add the node and rebuild the index map.
    parent->children.emplace(sub_key, node);
    build_index_map(parent);

    // Set the index of the first added child.
    if (index == -1) {
      index = node->index;
    }

    parent = node;
    key_list.pop_front();
  }

  // Set the last added node real.
  node->type = type;
  return true;
}

/**
 * @brief Check if can remove a child of a specific type from the tree.
 * @param key[in] The key of the child to remove.
 * @param type[in] The to of the child to remove.
 * @param parent_key[out] The key of the parent that contains the key to remove.
 * @param index[out] The index of the key to remove in his parent.
 * @return @c true if the key can be removed, @c false if the key have childs.
 */
bool IndexedStringTree::can_remove_child(
    const QString& key, int type, QString& parent_key, int& index) {

  // Check if the key exists.
  if (!key_exists(key)) {
    return false;
  }

  // Check if the node have childs.
  Node* node = get_child(key);
  if (node->children.size() > 0 || node->type != type) {
    return false;
  }

  // Get the first real parent in the hierarchy (or root).
  Node* parent = node->parent;
  while (parent != root && parent != nullptr &&
         parent->type == CONTAINER && parent->children.size() == 1) {

    node = parent;
    parent = node->parent;
  }

  // Set the parent_key and the index, return true.
  parent_key = parent != nullptr ? parent->key : "";
  index = node->index;
  return true;
}

/**
 * @brief Removes a child with a specific key and specific type from the tree.
 * @param key The key of the child to remove.
 * @param type The type of the child to remove.
 * @return @c true in case of success.
 */
bool IndexedStringTree::remove_child(const QString& key, int type, bool keep_key) {

  QString parent_key;
  int index;

  // If no need to be removed.
  if (!can_remove_child(key, type, parent_key, index)) {

    // Check if the key exists.
    if (!key_exists(key)) {
      return false;
    }

    Node* node = get_child(key);
    if (type == node->type) {
      if (type == REF_KEY && keep_key) {
        node->type = REAL_KEY;
      } else {
        node->type = CONTAINER;
      }
    }
    return true;
  }

  if (type == REF_KEY && keep_key) {
    Node* node = get_child(key);
    node->type = REAL_KEY;
    return true;
  }

  // Get the parent and the iterator of the node.
  Node* parent = get_child(parent_key);
  auto it = parent->children.begin();
  std::advance(it, index);

  // Remove the node and rebuild the index map.
  delete it->second;
  parent->children.erase(it);
  build_index_map(parent);
  return true;
}

/**
 * @brief Builds or rebuilds the internal mapping of a node that gives
 * indexes from keys.
 *
 * Nodes are indexed by string keys, but nodes also treats them as a map,
 * so we need an additional integer index.
 *
 * @param node The node where to rebuild the index.
 */
void IndexedStringTree::build_index_map(const Node* node) const {

  int index = 0;
  for (auto& kvp : node->children) {
    kvp.second->index = index++;
  }
}

/**
 * @brief Clears childs of a node.
 * @param node The node.
 */
void IndexedStringTree::clear_children(Node* node) {

  auto it = node->children.begin();
  while (it != node->children.end()) {
    clear_children(it->second);
    delete it->second;
    it++;
  }
  node->children.clear();
}
