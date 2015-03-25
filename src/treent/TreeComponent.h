
#pragma once

#include "entityx/Entity.h"

namespace treent
{

///
/// A component that can be part of a single-parent, multiple-child tree structure.
/// Derived types must define a `compose` method, which is called during traversal to
/// compose either the parent or the child into the current component.
/// `compose(const Derived &)`
/// It generally only makes sense to traverse the tree in one direction, so both ascend()
/// and descend() use the same compose() method, albeit passing different parameters.
///
/// Note that the relationship between TreeComponents is not automatically cleaned up when
/// a component is destroyed. Automatically cleaning up would cause a cascade of unnecessary
/// changes when a parent component was destroyed.
/// Instead, you can explicitly detach the entity you are destroying before destroying it:
///
/// TreentT<Your,Tree,Components>::detachFromParent(entity)
/// entity.destroy();
///
/// The above is only necessary if you might destroy a child before its parent.
///
template <typename Derived>
struct TreeComponent : public entityx::Component<Derived>
{
public:
  /// Cast to derived type.
  Derived& self() { return static_cast<Derived&>(*this); }

  using Ref = entityx::ComponentHandle<Derived>;

  /// Associate a child component with a parent component.
  /// Static since we can't generate component handles from this.
  static void attachToParent(Ref child, Ref parent);

  void removeChild(Derived *child);
  void removeChildren();
  void detachFromParent();

  /// Visit all children depth-first.
  /// Passes parent to each child's compose(const Derived &) method.
  void descend();
  /// Visit all parents (depth-only).
  /// Passes each child to parent's compose(const Derived &) method.
  void ascend();

  /// Convenience method for
  void compose(const Ref &handle) { compose(*handle.get()); }

  bool isRoot() { return ! _parent; }
  bool isLeaf() { return _children.empty(); }

  Ref getParent() { return _parent; }
private:
  Ref               _parent;
  std::vector<Ref>  _children;
};

#pragma mark - TreeComponent Template Implementation

template <typename D>
void TreeComponent<D>::attachToParent(Ref child, Ref parent)
{
  child->_parent = parent;
  parent->_children.push_back(child);
}

template <typename D>
void TreeComponent<D>::removeChild(D *child)
{
  assert(child->_parent.get() == &self());
  child->_parent = Ref(); // make invalid

  auto comp = [child] (Ref &b)
  {
    return b.get() == child;
  };

  _children.erase(std::remove_if(_children.begin(), _children.end(), comp), _children.end());
}

template <typename D>
void TreeComponent<D>::removeChildren()
{
  for (auto child : _children)
  {
    child->detachFromParent();
  }
  _children.clear();
}

template <typename D>
void TreeComponent<D>::detachFromParent()
{
  if (_parent)
  {
    _parent->removeChild(&self());
    _parent = Ref();
  }
}

template <typename D>
void TreeComponent<D>::descend()
{
  for (auto &c : _children)
  {
    c->compose(self());
    c->descend();
  }
}

template <typename D>
void TreeComponent<D>::ascend()
{
  if (_parent)
  {
    _parent->compose(self());
    _parent->ascend();
  }
}

} // namespace treent
