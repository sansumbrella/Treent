
#include "TreentNodeComponent.h"

namespace treent
{

///
/// Treent manages a tree of entities that share a common set of tree components.
///
template <typename ... TreeComponents>
class Treent
{
public:
  using TreentRef = std::shared_ptr<Treent>;

  explicit Treent (entityx::Entity entity);
  virtual ~Treent ();

  //
  // TODO: Mirror Entity methods. (add/remove components, getOrAssign convenience.)
  //

  entityx::Entity entity() { return _entity; }

  //
  // Tree growing/pruning methods.
  //

  TreentRef createChild ();
  void      appendChild (const TreentRef &child);
  void      removeChild (Treent *child);
  void      removeChild (const TreentRef &child) { removeChild(child.get()); }

  /// Remove the Treent from its parent.
  /// If there are no other references, this destroys the Treent.
  void      destroy () { if (_parent) { _parent->remove(this); } }

private:
  entityx::Entity         _entity;
  std::vector<TreentRef>  _children;
  Treent*                 _parent = nullptr;

  template <typename C>
  void      assignComponent ();
  template <typename C1, typename C2, typename ... Components>
  void      assignComponents ();

  template <typename C>
  void      attachChildComponent (const TreentRef &child);
  template <typename C1, typename C2, typename ... Components>
  void      attachChildComponents (const TreentRef &child);

  template <typename C>
  void      detachComponent ();
  template <typename C1, typename C2, typename ... Components>
  void      detachComponents ();
};

#pragma mark - Treent Template Implementation

template <typename ... TreeComponents>
Treent::Treent(entityx::Entity entity)
: _entity(entity)
{
  assignComponents<TreeComponents>();
  _entity.assign<TreentNodeComponent>(this);
}

template <typename ... TreeComponents>
Treent::~Treent()
{
  if (_entity)
  {
    _entity.destroy();
  }

  // In general all the children will be destroyed with the parent.
  for (auto &c : _children)
  {
    // Invalidate possible dangling pointers in case the children are referenced elsewhere and kept alive.
    // Note that we don't need to detach the component parent handles here, since they are invalidated when the parent component is destroyed.
    c->_parent = nullptr;
  }

  // We don't need to remove self from parent, since we wouldn't be destructed if we were still there.
}

template <typename ... TreeComponents>
TrentRef Treent::createChild ()
{
  auto child = std::make_shared<Treent>();

  child->_parent = this;
  attachChildComponents<TreeComponents>(child);
  _children.push_back(child);
  return child;
}

template <typename ... TreeComponents>
void Treent::appendChild (const TreentRef &child)
{
  if (child->_parent) {
    if (child->_parent == this)
    {
      return;
    }
    else
    {
      child->_parent->removeChild(child);
    }
  }

  attachChildComponents<TreeComponents>(child);
  _children.push_back(child);

}

template <typename ... TreeComponents>
void Treent::removeChild (Treent *child)
{
  child->detachComponents<TreeComponents>();

  auto comp = [child] (const TreentRef &c) {
    return c.get() == child;
  };
  _children.erase(std::remove_if(_children.begin(), _children.end(), comp), _children.end());
}

template <typename ... TreeComponents>
template <typename C>
void Treent::assignComponent()
{
  _entity.assign<C>();
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void Treent::assignComponents()
{
  assignComponent<C1>();
  return assignComponents<C2, Components ...>();
}

template <typename ... TreeComponents>
template <typename C>
void Treent::attachChildComponent(const TreentRef &child)
{
  _entity.component<C>->appendChild(child->component<C>());
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void Treent::attachChildComponents(const TreentRef &child)
{
  attachChildComponent<C1>(child);
  return attachChildComponents<C2, Components ...>(child);
}

template <typename ... TreeComponents>
template <typename C>
void Treent::detachComponent()
{
  _entity.component<C>->detachFromParent();
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void Treent::detachComponents()
{
  detachComponent<C1>();
  return detachComponents<C2, Components ...>();
}

} // namespace treent
