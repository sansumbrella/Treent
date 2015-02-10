
#pragma once

#include "entityx/Entity.h"
#include "TreentNodeComponent.h"

#include "2d/Components.h"

namespace treent
{

///
/// Treent manages a tree of entities that share a common set of tree components.
/// Use to create prefab-like objects in your source code.
/// Stores the entity manager so it can create its entity and child entities.
///
template <typename ... TreeComponents>
class Treent
{
public:
  using TreentRef = std::shared_ptr<Treent>;

  explicit Treent (entityx::EntityManager &entities);
  virtual ~Treent ();

  //
  // TODO: Mirror Entity methods. (add/remove components, getOrAssign convenience.)
  //

  entityx::Entity& entity() { return _entity; }

  //
  // Tree growing/pruning methods.
  //

  /// Creates and returns a treent node as a child of this Treent.
  TreentRef createChild ();
  /// Creates and returns a treent node as a child of this Treent.
  /// Args are passed to the constructor after the entity manager.
  template <typename TreentType, typename ... Args>
  std::shared_ptr<TreentType> createChild ();

  void      appendChild (const TreentRef &child);
  void      removeChild (const TreentRef &child) { removeChild(child.get()); }
  void      removeChild (Treent *child);

  /// Remove the Treent from its parent.
  /// If there are no other references, this destroys the Treent.
  void      destroy () { if (_parent) { _parent->remove(this); } }

private:
  entityx::EntityManager  &_entities;
  entityx::Entity         _entity;
  std::vector<TreentRef>  _children;
  Treent*                 _parent = nullptr;

  // Create necessary component connections and store reference to child.
  void      attachChild (const TreentRef &child);

  template <typename C>
  void      assignComponent ()
  {
    _entity.assign<C>();
  }

  template <typename C1, typename C2, typename ... Cs>
  void      assignComponent ()
  {
    assignComponent<C1>();
    assignComponent<C2, Cs...>();
  }

  template <typename C>
  void      attachChildComponent (const TreentRef &child);
  template <typename C1, typename C2, typename ... Components>
  void      attachChildComponent (const TreentRef &child);

  template <typename C>
  void      detachComponent ();
  template <typename C1, typename C2, typename ... Components>
  void      detachComponent ();
};

template <typename ... TreeComponents>
using TreentRef = std::shared_ptr<Treent<TreeComponents...>>;

#pragma mark - Treent Template Implementation

template <typename ... TreeComponents>
Treent<TreeComponents...>::Treent(entityx::EntityManager &entities)
: _entities(entities),
  _entity(entities.create())
{
  assignComponent<TreeComponents...>();
//  _entity.assign<TreentNodeComponent>(this);
}

template <typename ... TreeComponents>
Treent<TreeComponents...>::~Treent()
{
  // maybe prefer assert to conditional check;
  // though user _could_ invalidate entity manually, should be discouraged when using Treent.
  assert(_entity);
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
std::shared_ptr<Treent<TreeComponents...>> Treent<TreeComponents...>::createChild ()
{
  auto child = std::make_shared<Treent<TreeComponents...>>(_entities);

  attachChild(child);
  return child;
}

template <typename ... TreeComponents>
void Treent<TreeComponents...>::attachChild (const TreentRef &child)
{
  child->_parent = this;
  attachChildComponent<TreeComponents...>(child);
  _children.push_back(child);
}

template <typename ... TreeComponents>
void Treent<TreeComponents...>::appendChild (const TreentRef &child)
{
  if (child->_parent)
  {
    if (child->_parent == this)
    {
      return;
    }
    else
    {
      child->_parent->removeChild(child);
    }
  }

  attachChild(child);
}

template <typename ... TreeComponents>
void Treent<TreeComponents...>::removeChild (Treent *child)
{
  child->detachComponent<TreeComponents...>();

  auto comp = [child] (const TreentRef &c) {
    return c.get() == child;
  };
  _children.erase(std::remove_if(_children.begin(), _children.end(), comp), _children.end());
}

template <typename ... TreeComponents>
template <typename C>
void Treent<TreeComponents...>::attachChildComponent(const TreentRef &child)
{
  C::attachToParent(child->entity().template component<C>(), _entity.component<C>());
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void Treent<TreeComponents...>::attachChildComponent(const TreentRef &child)
{
  attachChildComponent<C1>(child);
  return attachChildComponent<C2, Components ...>(child);
}

template <typename ... TreeComponents>
template <typename C>
void Treent<TreeComponents...>::detachComponent()
{
  _entity.component<C>()->detachFromParent();
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void Treent<TreeComponents...>::detachComponent()
{
  detachComponent<C1>();
  return detachComponent<C2, Components ...>();
}

} // namespace treent
