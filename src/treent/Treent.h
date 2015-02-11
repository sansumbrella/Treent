
#pragma once

#include "entityx/Entity.h"
#include "TreentNodeComponent.h"

#include "2d/Components.h"

namespace treent
{

template <typename ... TreeComponents> class Treent;

template <typename ... TreeComponents>
using TreentRef = std::unique_ptr<Treent<TreeComponents...>>;

///
/// Treent manages a tree of entities that share a common set of tree components.
/// Use to create prefab-like objects in your source code.
/// Stores the entity manager so it can create its entity and child entities.
///
template <typename ... TreeComponents>
class Treent
{
public:
  using TreentRef = TreentRef<TreeComponents...>;

  /// Construct a Treent with an EntityManager.
  explicit Treent (entityx::EntityManager &entities);
  virtual ~Treent ();

  /// Treent has unique ownership of entity, so shouldn't be copied.
  /// Additionally, we get much better compiler errors when doing this explicitly.
  Treent (const Treent &other) = delete;

  //
  // TODO: Mirror Entity methods. (add/remove components, getOrAssign convenience.)
  //

  entityx::Entity& entity() { return _entity; }

  //
  // Tree growing/pruning methods.
  //

  /// Creates and returns a treent node as a child of this Treent.
  Treent& createChild ();
  /// Creates and returns a treent node as a child of this Treent.
  /// Args are passed to the constructor after the entity manager.
  template <typename TreentType, typename ... Parameters>
  TreentType& createChild (Parameters&& ... parameters);

  /// Appends a child to Treent, transferring ownership to the parent.
  void      appendChild (TreentRef &&child);

  /// Removes child from Treent and transfers ownership to caller in a unique_ptr.
  TreentRef removeChild (Treent &child) { return removeChild(&child); }
  /// Removes child from Treent and transfers ownership to caller in a unique_ptr.
  TreentRef removeChild (Treent *child);

  /// Remove the Treent from its parent.
  /// If there are no other references, this destroys the Treent.
  void      destroy () { if (_parent) { _parent->remove(this); } }

  // Enable iteration over the children (for doing animations on each child with an offset, etc.)
  typename std::vector<TreentRef>::iterator begin() { return _children.begin(); }
  typename std::vector<TreentRef>::iterator end() { return _children.end(); }

private:
  entityx::EntityManager  &_entities;
  entityx::Entity         _entity;
  std::vector<TreentRef>  _children;
  Treent*                 _parent = nullptr;

  // Create necessary component connections and store reference to child.
  void      attachChild (TreentRef &&child);

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

  // Note: All the children will be destroyed with the parent, since we have unique_ptr's to them. No need to notify.
  // Also: don't need to notify parent, because we wouldn't be destroyed if it still cared about us.
}

template <typename ... TreeComponents>
Treent<TreeComponents...>& Treent<TreeComponents...>::createChild ()
{
  auto child = new Treent<TreeComponents...>(_entities);
  attachChild(std::unique_ptr<Treent<TreeComponents...>>(child));
  return *child;
}

template <typename ... TreeComponents>
template <typename TreentType, typename ... Parameters>
TreentType& Treent<TreeComponents...>::createChild (Parameters&& ... parameters)
{
  auto child = new TreentType(_entities, std::forward<Parameters>(parameters)...);
  attachChild(std::unique_ptr<TreentType>(child));
  return *child;
}

template <typename ... TreeComponents>
void Treent<TreeComponents...>::attachChild (TreentRef &&child)
{
  child->_parent = this;
  attachChildComponent<TreeComponents...>(child);
  _children.push_back(std::move(child));
}

template <typename ... TreeComponents>
void Treent<TreeComponents...>::appendChild (TreentRef &&child)
{
  // If the child was owned by a parent, the user wouldn't have the unique_ptr to pass in here.
  assert(! child->_parent);

  attachChild(std::move(child));
}

template <typename ... TreeComponents>
TreentRef<TreeComponents...> Treent<TreeComponents...>::removeChild (Treent *child)
{
  child->detachComponent<TreeComponents...>();
  child->_parent = nullptr;

  auto comp = [child] (const TreentRef &c) {
    return c.get() == child;
  };
  auto begin = std::remove_if(_children.begin(), _children.end(), comp);
  if (begin != _children.end()) {
    begin->release();
    _children.erase(begin, _children.end());
    return TreentRef(child);
  }

  // Wasn't already a child, return nullptr.
  return nullptr;
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
