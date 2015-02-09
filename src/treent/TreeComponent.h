
#pragma once

#include "entityx/Entity.h"

namespace treent
{

/// A single-parent, multiple-child tree structure.
/// Derived types must define the following methods:
/// updateChild(Derived &)    => called with child when traversing down.
/// updateParent(Derived &)   => called with parent when traversing up.
/// compose(const Derived &)  => called with parent when detaching to compose parent's effects into self.
///
/// Does not detach in destructor since that is handled only where needed by a Treent.
/// (top level is removed from its parent, others are safe to destroy since only ref is through top)
template <typename Derived>
struct TreeComponent : public entityx::Component<TreeComponent<Derived>>
{
public:
  /// Cast to derived type.
  Derived& self() { return static_cast<Derived&>(*this); }

  using Ref = entityx::ComponentHandle<Derived>;

  /// Associate a child component with a parent component.
  /// Static since we can't generate component handles from this.
  static void attachToParent (Ref child, Ref parent);

  void removeChild (Derived *child);
  void detachFromParent ();

  /// Visit all children depth-first.
  /// Calls updateChild(Derived &) for each.
  void descend();
  /// Visit all parents (depth-only).
  /// Calls updateParent(Derived &) for each.
  void ascend();

private:
  Ref               _parent;
  std::vector<Ref>  _children;
};

#pragma mark - TreeComponent Template Implementation

template <typename D>
void TreeComponent<D>::attachToParent (Ref child, Ref parent)
{
  child->_parent = parent;
  parent->_children.push_back(child);
}

template <typename D>
void TreeComponent<D>::removeChild (D *child)
{
  assert(child->_parent.get() == &self());
  child->_parent = Ref(); // make invalid

  auto comp = [child] (Ref &b) {
      return b.get() == child;
  };

  _children.erase(std::remove_if(_children.begin(), _children.end(), comp), _children.end());
}

template <typename D>
void TreeComponent<D>::detachFromParent()
{
  if (_parent)
  {
    self().compose(*_parent.get());
    _parent->removeChild(&self());
  }
}

template <typename D>
void TreeComponent<D>::descend()
{
  for (auto &c : _children)
  {
    self().updateChild(*c.get());
    c->descend();
  }
}

template <typename D>
void TreeComponent<D>::ascend()
{
  if (_parent)
  {
    self().updateParent(*_parent.get());
    _parent->ascend();
  }
}

} // namespace treent
