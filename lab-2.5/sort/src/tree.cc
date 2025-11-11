#include "tree.hh"
#include <sstream>

void Tree::insert(int new_element) {
  // TODO
  if (this->root == nullptr){ // or in C++ often written if(!this->root)
    this-root = new TreeNode(nullptr, nullptr, new_element);
  } else {
    auto p = this->root;
    auto v = p->value;
    if (new_element <= V){
      p = p->lhs;
    } else {
      p = p->rhs;
    }
  }
}

void insert_helper(TreeNode* p, int new_element){
  if (new_element <= p->value){
    
  }
}


std::string Tree::description() const {
  std::ostringstream result;
  result << "[";
  auto first = true;

  this->visit([&](auto i) {
    if (first) {
      first = false;
    } else {
      result << ", ";
    }
    result << i;
  });

  result << "]";
  return result.str();
}
