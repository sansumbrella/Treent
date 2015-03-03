
#pragma once

#include "entityx/Entity.h"
#include "TreentNodeComponent.h"

namespace treent
{

using entityx::Entity;
using entityx::EntityManager;
using entityx::ComponentHandle;

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
  using TreentRef = std::unique_ptr<Treent>;
  using Component = TreentNodeComponent<Treent>;

  /// Construct a Treent with an EntityManager.
  explicit Treent (EntityManager &entities);
  virtual ~Treent ();

  /// Treent has unique ownership of entity, so shouldn't be copied.
  /// Additionally, we get much better compiler errors when doing this explicitly.
  Treent (const Treent &other) = delete;

  /// Factory function for creating Treents.
  template<typename TreentType, typename ... Parameters>
  static TreentRef create (EntityManager &entities, Parameters&& ... parameters);
  static TreentRef create (EntityManager &entities);

  /// Create and return a Treent that is not a child of this Treent. (Spawn a rhizome...)
  template<typename TreentType, typename ... Parameters>
  TreentRef create (Parameters&& ... parameters) { create<TreentType>(_entities, std::forward<Parameters>(parameters)...); }

  //
  // TODO: Mirror Entity methods. (add/remove components, getOrAssign convenience.)
  //

  /// Returns the underlying entity. Prefer the use of the mirroring interface to this.
  Entity&            entity() { return _entity; }

  /// Assign a component to entity, forwarding params to the component constructor.
  template <typename C, typename ... Params>
  ComponentHandle<C> assign (Params&& ... params) { return _entity.assign<C>(std::forward<Params>(params)...); }
  /// Producer for assigning multiple components.
  template <typename C1, typename C2, typename ... Cs>
  void      assign ();

  /// Get a handle to an existing component of the entity.
  template <typename C>
  ComponentHandle<C> component () { return _entity.component<C>(); }

  /// Get a handle to an existing component of the entity.
  template <typename C>
  ComponentHandle<C> get () { return _entity.component<C>(); }

  template <typename C>
  typename C::Handle getOrAssign() { if( hasComponent<C>() ) { return component<C>(); } return assign<C>(); }

  template <typename C>
  bool               hasComponent() const { return _entity.has_component<C>(); }

  //
  // Tree growing/pruning methods.
  //

  /// Creates and returns a treent node as a child of this Treent.
  Treent&     createChild ();
  /// Creates and returns a subclass of treent node as a child of this Treent.
  /// Args are passed to the constructor after the entity manager.
  template <typename TreentType, typename ... Parameters>
  TreentType& createChild (Parameters&& ... parameters);

  /// Appends a child to Treent, transferring ownership to the parent.
  void        appendChild (TreentRef &&child);

  /// Removes child from Treent and transfers ownership to caller in a unique_ptr.
  TreentRef   removeChild (Treent &child) { return removeChild(&child); }
  /// Removes child from Treent and transfers ownership to caller in a unique_ptr.
  TreentRef   removeChild (Treent *child);
  /// Remove all children from Treent.
  void        destroyChildren();

  bool        hasChildren() const { return ! _children.empty(); }

  /// Remove the Treent from its parent.
  /// If it was parented, this destroys the Treent as it removes the last reference from scope.
  void        destroy() __deprecated { if (_parent) { _parent->removeChild(this); } }
  TreentRef   removeFromParent() { if (_parent) { return _parent->removeChild(this); } return nullptr; }

  //
  // Child iteration methods.
  // Use for animating each child with a delay or similar tasks.
  // To manipulate the children, use Systems that act on the relevant Components.
  //

  typename std::vector<TreentRef>::const_iterator begin() const { return _children.begin(); }
  typename std::vector<TreentRef>::const_iterator end() const { return _children.end(); }

protected:
  void attachChildComponents(Treent &child) { attachChildComponent<TreeComponents...>(child); }
  void detachComponentsFromParent() { detachComponentFromParent<TreeComponents...>(); }

private:
  entityx::EntityManager  &_entities;
  entityx::Entity         _entity;
  std::vector<TreentRef>  _children;
  Treent*                 _parent = nullptr;

  // Create necessary component connections and store reference to child.
  void      attachChild (TreentRef &&child);

  template <typename C>
  void      attachChildComponent (Treent &child);
  template <typename C1, typename C2, typename ... Components>
  void      attachChildComponent (Treent &child);

  template <typename C>
  void      detachComponentFromParent ();
  template <typename C1, typename C2, typename ... Components>
  void      detachComponentFromParent ();

  friend class TreentNodeComponent<Treent>;
};

#pragma mark - Treent Template Implementation

template <typename ... TreeComponents>
Treent<TreeComponents...>::Treent(EntityManager &entities)
: _entities(entities),
  _entity(entities.create())
{
  assign<TreeComponents...>();
  _entity.assign<TreentNodeComponent<Treent>>(this);
}

template <typename ... TreeComponents>
Treent<TreeComponents...>::~Treent()
{
  if (_entity)
  {
    get<TreentNodeComponent<Treent>>()->_treent = nullptr;
    _entity.destroy();
  }

  // Note: All the children will be destroyed with the parent, since we have unique_ptr's to them. No need to notify.
  // Also: don't need to notify parent, because we wouldn't be destroyed if it still cared about us.
}

template <typename ... TreeComponents>
template<typename TreentType, typename ... Parameters>
TreentRef<TreeComponents...> Treent<TreeComponents...>::create (EntityManager &entities, Parameters&& ... parameters)
{
  return std::unique_ptr<TreentType>(new TreentType(entities, std::forward<Parameters>(parameters)...));
}

template <typename ... TreeComponents>
TreentRef<TreeComponents...> Treent<TreeComponents...>::create (EntityManager &entities)
{
  return std::unique_ptr<Treent>(new Treent(entities));
}

// TODO: implement createChild() methods in terms of create().
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
template <typename C1, typename C2, typename ... Cs>
void Treent<TreeComponents...>::assign ()
{
  assign<C1>();
  assign<C2, Cs...>();
}

template <typename ... TreeComponents>
void Treent<TreeComponents...>::attachChild (TreentRef &&child)
{
  child->_parent = this;
  attachChildComponent<TreeComponents...>(*child);
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
  child->detachComponentFromParent<TreeComponents...>();
  child->_parent = nullptr;

  auto comp = [child] (const TreentRef &c) {
    return c.get() == child;
  };
  auto iter = std::find_if(_children.begin(), _children.end(), comp);
  if (iter != _children.end()) {
    auto ptr = iter->release();
    _children.erase(iter);
    return TreentRef(ptr);
  }

  // Wasn't already a child, return nullptr.
  return nullptr;
}

template <typename ... TreeComponents>
void Treent<TreeComponents...>::destroyChildren()
{
  for (auto &child : _children) {
    child->template detachComponentFromParent<TreeComponents...>();
  }
  _children.clear();
}

template <typename ... TreeComponents>
template <typename C>
void Treent<TreeComponents...>::attachChildComponent(Treent &child)
{
  C::attachToParent(child.template component<C>(), _entity.component<C>());
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void Treent<TreeComponents...>::attachChildComponent(Treent &child)
{
  attachChildComponent<C1>(child);
  return attachChildComponent<C2, Components ...>(child);
}

template <typename ... TreeComponents>
template <typename C>
void Treent<TreeComponents...>::detachComponentFromParent()
{
  _entity.component<C>()->detachFromParent();
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void Treent<TreeComponents...>::detachComponentFromParent()
{
  detachComponentFromParent<C1>();
  return detachComponentFromParent<C2, Components ...>();
}

} // namespace treent
