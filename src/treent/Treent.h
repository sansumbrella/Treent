
#pragma once

#include "ChildrenComponent.h"
#include "ParentComponent.h"
#include "TreentNodeComponent.h"
#include "TreentBase.h"
#include "detail/Logging.h"

namespace treent
{

using entityx::Entity;
using entityx::EntityManager;
using entityx::ComponentHandle;

///
/// Treent manages a tree of entities that share a common set of tree components.
/// Use to create prefab-like objects in your source code.
/// Stores the entity manager so it can create its entity and child entities.
///
template <typename ... TreeComponents>
class TreentT : public TreentBase
{
public:

  explicit TreentT(const Entity &entity);
  static TreentT create() { return TreentT(entities().create()); }

  //
  // Tree growing/pruning methods.
  //

  /// Creates and returns a treent node as a child of this Treent.
  TreentT     createChild();

  /// Creates and returns a subclass of treent node as a child of this Treent.
  /// Args are passed to the constructor after the entity manager.
  template <typename Derived, typename ... Parameters>
  Derived     createChild(Parameters&& ... parameters);

  /// Appends a child to Treent, transferring ownership to the parent entity.
  void        appendChild(Entity &child);

  /// Removes child from Treent.
  void        removeChild(Entity &child);

  /// Remove and destroy all children of Treent.
	void				destroyChildren();

  //
  // Child iteration methods.
  // Use for animating each child with a delay or similar tasks.
  // To manipulate the children, use Systems that act on the relevant Components.
  //

  const std::vector<Entity>& getChildren() const { return component<ChildrenComponent>()->_children; }

  std::vector<Entity>::const_iterator begin() const { return getChildren().begin(); }
  std::vector<Entity>::const_iterator end() const { return getChildren().end(); }

  /// Detach all Tree components.
  static void detachFromParent(Entity &child);
  void        detachFromParent() { detachFromParent(entity()); }

private:
  /// Connect child tree components and set parent/children component relationship.
  void        attachChild(Entity &child);

  template <typename C>
  void        attachChildTreeComponent(Entity &child);
  template <typename C1, typename C2, typename ... Components>
  void        attachChildTreeComponent(Entity &child);

  template <typename C>
  static void detachTreeComponentFromParent(Entity &entity);
  template <typename C1, typename C2, typename ... Components>
  static void detachTreeComponentFromParent(Entity &entity);
};

#pragma mark - Treent Template Implementation

template <typename ... TreeComponents>
TreentT<TreeComponents...>::TreentT(const Entity &entity)
: TreentBase(entity)
{
  assignIfMissing<ChildrenComponent>();
  assignIfMissing<TreeComponents...>();
}

template <typename ... TreeComponents>
void TreentT<TreeComponents...>::attachChild(entityx::Entity &child)
{
  auto pc = treent::getOrAssign<ParentComponent>(child);
  auto cc = component<ChildrenComponent>();

  pc->_parent = entity();
  cc->addChild(child);

  attachChildTreeComponent<TreeComponents...>(child);
}

template <typename ... TreeComponents>
void TreentT<TreeComponents...>::detachFromParent(entityx::Entity &child)
{
  auto pc = child.component<ParentComponent>();

  if (pc) {
    pc->_parent.component<ChildrenComponent>()->removeChild(child);
    pc.remove();
    detachTreeComponentFromParent<TreeComponents...>(child);
  }
}

template <typename ... TreeComponents>
void TreentT<TreeComponents...>::removeChild(entityx::Entity &child)
{
  auto pc = child.component<ParentComponent>();

  if (pc && pc->_parent == entity()) {
    detachFromParent(child);
  }
  else {
    TREENT_WARN( "Attempt to remove child not belonging to this treent." );
  }
}

template <typename ... TreeComponents>
TreentT<TreeComponents...> TreentT<TreeComponents...>::createChild()
{
  auto child = TreentT(entities().create());
  attachChild(child.entity());
  return child;
}

template <typename ... TreeComponents>
template <typename Derived, typename ... Parameters>
Derived TreentT<TreeComponents...>::createChild(Parameters&& ... parameters)
{
  auto child = TreentType(entities().create(), std::forward<Parameters>(parameters)...);
  attachChild(child.entity());
  return child;
}

template <typename ... TreeComponents>
void TreentT<TreeComponents...>::appendChild(Entity &child)
{
  detachFromParent(child);
  attachChild(child);
}

template <typename ... TreeComponents>
void TreentT<TreeComponents...>::destroyChildren()
{
  auto cc = component<ChildrenComponent>();
  auto children = cc->_children;
  cc->_children.clear();

	for (auto &child : children) {
    detachTreeComponentFromParent<TreeComponents...>(child);
	}
}

template <typename ... TreeComponents>
template <typename C>
void TreentT<TreeComponents...>::attachChildTreeComponent(Entity &child)
{
  C::attachToParent(child.component<C>(), component<C>());
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void TreentT<TreeComponents...>::attachChildTreeComponent(Entity &child)
{
  attachChildTreeComponent<C1>(child);
  return attachChildTreeComponent<C2, Components ...>(child);
}

template <typename ... TreeComponents>
template <typename C>
void TreentT<TreeComponents...>::detachTreeComponentFromParent(Entity &child)
{
  child.component<C>()->detachFromParent();
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void TreentT<TreeComponents...>::detachTreeComponentFromParent(Entity &child)
{
  detachTreeComponentFromParent<C1>(child);
  return detachTreeComponentFromParent<C2, Components ...>(child);
}

} // namespace treent
