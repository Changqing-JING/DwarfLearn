#ifndef TREE_HPP
#define TREE_HPP
#include <cstdint>
#include <memory>
#include <vector>

template <typename T>
class TreeNode {
public:
  TreeNode(T const &data, uint32_t const level) : data_(data), level_(level) {
  }

  TreeNode &addChild(T const &data) {
    TreeNode &child = children_.emplace_back(data, level_ + 1U);
    return child;
  }

private:
  T data_;
  uint32_t level_;
  std::vector<TreeNode> children_;
};

template <typename T>
class Tree {

public:
  TreeNode<T> &setRoot(T const &data) {
    root_ = std::make_unique<TreeNode<T>>(data, 0);
    return *root_;
  }

  bool hasRoot() const noexcept {
    return root_ != nullptr;
  }

private:
  std::unique_ptr<TreeNode<T>> root_;
};

#endif