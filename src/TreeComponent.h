
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
struct TreeComponent
{
public:
  using Ref = typename Derived::Handle;

  void appendChild (const Ref &child);
  void removeChild (Derived &child);
  void detachFromParent ();

  /// Visit all children depth-first.
  /// Calls updateChild(Derived &) for each.
  void traverseDown();
  /// Visit all parents (depth-only).
  /// Calls updateParent(Derived &) for each.
  void traverseUp();

  /// Cast to derived type.
  Derived& self() const { return static_cast<Derived&>(*this); }

private:
  Ref               _parent;
  std::vector<Ref>  _children;
};

#pragma mark - TreeComponent Template Implementation

template <typename Derived>
void TreeComponent::appendChild (const Ref &child)
{
  _children.push_back(child);
}

template <typename Derived>
void TreeComponent::removeChild (Derived &child)
{
  assert(child._parent.get() == this);
  child._parent = nullptr;

  auto comp = [&child] (const Ref &b) {
      return *b.get() == child;
  };

  _children.erase(std::remove_if(_children.begin(), _children.end(), comp), _children.end());
}

template <typename Derived>
void TreeComponent::detachFromParent()
{
  if (_parent)
  {
    self().compose(*_parent);
    _parent->removeChild(*this);
  }
}

template <typename Derived>
void TreeComponent::traverseDown()
{
  for (auto &c : _children)
  {
    self().updateChild(*c.get());
    c->traverseDown();
  }
}

template <typename Derived>
void TreeComponent::traverseUp()
{
  if (_parent)
  {
    self().updateParent(*c.get());
    _parent->traverseUp();
  }
}

} // namespace treent
