#include <string>

/// A node in a binary search.
struct TreeNode {

  /// The left child of the tree, or `nullptr` if there's none.
  TreeNode *lhs;

  /// The right child of the tree, or `nullptr` if there's none.
  TreeNode *rhs;

  /// The value of this node.
  int value;

  /// Traverses the tree rooted at `this` in in-order and calls `callback` on each visited element.
  template<typename F>
  void visit(F&& callback) const {
    if (lhs != nullptr) { lhs->visit(callback); }
    callback(value);
    if (rhs != nullptr) { rhs->visit(callback); }
  }

};

/// A binary search tree.
struct Tree {

  /// The root of the tree, or `nullptr` if the tree is empty.
  TreeNode* root;

  /// Creates an empty tree.
  Tree() : root(nullptr) {};

  /// Inserts `new_element` in the tree.
  ///
  /// - Complexity: O(log n) where n is the number of element in the tree.
  void insert(int new_element);

  /// Traverses this tree in in-order and calls `callback` on each visited element.
  template<typename F>
  void visit(F&& callback) const {
    if (root != nullptr) { root->visit(callback); }
  }

  /// Returns a textual description of this tree.
  std::string description() const;

};
