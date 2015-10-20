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
#ifndef SOLARUSEDITOR_INDEXED_STRING_TREE_H
#define SOLARUSEDITOR_INDEXED_STRING_TREE_H

#include "natural_comparator.h"
#include <QString>
#include <map>

/**
 * @brief Tree of indexed string keys.
 * This class provides methods to manage a map indexed by string like a tree
 * for a QAbstractItemModel (see StringsModel and DialogsModel).
 */
class IndexedStringTree {

public:

  explicit IndexedStringTree(const QString& separator = ".");

  bool key_exists(const QString& key) const;
  int get_index(const QString& key) const;
  int get_row_count(const QString& key = "") const;
  QString* get_internal_key(const QString& key) const;

  QString get_row_key(int index, const QString& parent_key = "") const;

  bool has_parent(const QString& key) const;
  QString get_parent(const QString& key) const;

  bool add_key(const QString &key, QString& parent_key, int& index);
  bool add_key(const QString &key);

  bool can_remove_key(const QString& key, QString& parent_key, int& index);
  bool remove_key(const QString& key);

  bool add_ref(const QString &key, QString& parent_key, int& index);

  bool can_remove_ref(const QString& key, QString& parent_key, int& index);
  bool remove_ref(const QString& key, bool keep_key = false);

  void clear();

private:

  static constexpr int CONTAINER = 0;
  static constexpr int REAL_KEY = 1;
  static constexpr int REF_KEY = 2;

  /**
   * @brief A node of the indexed string tree.
   */
  struct Node {

    /** Constructor. */
    Node() : parent(nullptr), index(0), type(CONTAINER) {
    }

    Node* parent;   /**< The parent node. */
    int index;      /**< The index of the node. */

    QString key;    /**< The internal key of the node
                     * (complete key from the root). */

    int type;       /**< Type of the node. */

    std::map<QString, Node*, NaturalComparator>
      children;     /**< Children of the node. */
  };

  Node* get_child(const QString& key) const;

  Node* get_sub_child(const Node* node, const QString& sub_key) const;

  bool add_child(const QString& key, int type, QString& parent_key, int& index);

  bool can_remove_child(
      const QString& key, int type, QString& parent_key, int& index);
  bool remove_child(const QString& key, int type, bool keep_key = false);

  void build_index_map(const Node* node) const;

  void clear_children(Node *node);

  QString separator;  /**< The separator character. */
  Node* root;         /**< The root node of the tree. */

};

#endif
