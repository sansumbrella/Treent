
#pragma once

#include "TreentNodeComponent.h"
#include "TreentBase.h"

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
class Treent : public TreentBase
{
public:
	using TreentRef = std::unique_ptr<Treent>;
	using Component = TreentNodeComponent<Treent>;
  // Owner is equivalent until we have a manager class that can also be the owner.
  using Owner = Treent;

  /// Construct a Treent with an EntityManager.
  explicit Treent(EntityManager &entities);
  virtual ~Treent();

  /// Treent has unique ownership of entity, so shouldn't be copied.
  /// Additionally, we get much better compiler errors when doing this explicitly.
  Treent(const Treent &other) = delete;

	/// Factory function for creating Treents.
	template<typename TreentType, typename ... Parameters>
	static TreentRef create(EntityManager &entities, Parameters&& ... parameters);
	static TreentRef create(EntityManager &entities);

	/// Create and return a Treent that is not a child of this Treent. (Spawn a rhizome...)
	template<typename TreentType, typename ... Parameters>
	TreentRef create(Parameters&& ... parameters) { create<TreentType>(entities(), std::forward<Parameters>(parameters)...); }

  //
  // Tree growing/pruning methods.
  //

  /// Creates and returns a treent node as a child of this Treent.
  Treent&     createChild();
  /// Creates and returns a subclass of treent node as a child of this Treent.
  /// Args are passed to the constructor after the entity manager.
  template <typename TreentType, typename ... Parameters>
  TreentType& createChild(Parameters&& ... parameters);

  /// Appends a child to Treent, transferring ownership to the parent.
  void        appendChild(TreentRef &&child);

  /// Removes child from Treent and transfers ownership to caller in a unique_ptr.
  TreentRef   removeChild(Treent &child) { return removeChild(&child); }
  /// Removes child from Treent and transfers ownership to caller in a unique_ptr.
  TreentRef   removeChild(Treent *child);
	/// Remove all children from Treent.
	void				destroyChildren();

	bool				hasChildren() const { return ! _children.empty(); }

  /// Remove the Treent from its parent.
  /// If it was parented, this destroys the Treent as it removes the last reference from scope.
  void        destroy() { assert(_owner); _owner->removeChild(this); }

  //
  // Child iteration methods.
  // Use for animating each child with a delay or similar tasks.
  // To manipulate the children, use Systems that act on the relevant Components.
  //

  typename std::vector<TreentRef>::const_iterator begin() const { return _children.begin(); }
  typename std::vector<TreentRef>::const_iterator end() const { return _children.end(); }

private:
  std::vector<TreentRef>  _children;
  Treent*                 _parent = nullptr;
  Owner*                  _owner = nullptr;

  // Create necessary component connections and store reference to child.
  void      attachChild (TreentRef &&child);

  template <typename C>
  void      attachChildComponent(const TreentRef &child);
  template <typename C1, typename C2, typename ... Components>
  void      attachChildComponent(const TreentRef &child);

  template <typename C>
  void      detachComponent();
  template <typename C1, typename C2, typename ... Components>
  void      detachComponent();
};

#pragma mark - Treent Template Implementation

template <typename ... TreeComponents>
Treent<TreeComponents...>::Treent(EntityManager &entities)
: TreentBase(entities)
{
  assign<TreeComponents...>();
  assign<TreentNodeComponent<Treent>>(*this);
}

template <typename ... TreeComponents>
Treent<TreeComponents...>::~Treent()
{
  // maybe prefer assert to conditional check;
  // though user _could_ invalidate entity manually, should be discouraged when using Treent.

  // Note: All the children will be destroyed with the parent, since we have unique_ptr's to them. No need to notify.
  // Also: don't need to notify parent, because we wouldn't be destroyed if it still cared about us.
}

template <typename ... TreeComponents>
template<typename TreentType, typename ... Parameters>
TreentRef<TreeComponents...> Treent<TreeComponents...>::create(EntityManager &entities, Parameters&& ... parameters)
{
	return std::unique_ptr<TreentType>(new TreentType(entities, std::forward<Parameters>(parameters)...));
}

template <typename ... TreeComponents>
TreentRef<TreeComponents...> Treent<TreeComponents...>::create(EntityManager &entities)
{
	return std::unique_ptr<Treent>(new Treent(entities));
}

// TODO: implement createChild() methods in terms of create().
template <typename ... TreeComponents>
Treent<TreeComponents...>& Treent<TreeComponents...>::createChild()
{
  auto child = new Treent<TreeComponents...>(entities());
  attachChild(std::unique_ptr<Treent<TreeComponents...>>(child));
  return *child;
}

template <typename ... TreeComponents>
template <typename TreentType, typename ... Parameters>
TreentType& Treent<TreeComponents...>::createChild(Parameters&& ... parameters)
{
  auto child = new TreentType(entities(), std::forward<Parameters>(parameters)...);
  attachChild(std::unique_ptr<TreentType>(child));
  return *child;
}

template <typename ... TreeComponents>
void Treent<TreeComponents...>::attachChild(TreentRef &&child)
{
  child->_parent = this;
  child->_owner = this;
  attachChildComponent<TreeComponents...>(child);
  _children.push_back(std::move(child));
}

template <typename ... TreeComponents>
void Treent<TreeComponents...>::appendChild(TreentRef &&child)
{
  // If the child was owned by a parent, the user wouldn't have the unique_ptr to pass in here.
  assert(! child->_parent);

  attachChild(std::move(child));
}

template <typename ... TreeComponents>
TreentRef<TreeComponents...> Treent<TreeComponents...>::removeChild(Treent *child)
{
  child->detachComponent<TreeComponents...>();
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
		child->template detachComponent<TreeComponents...>();
	}
	_children.clear();
}

template <typename ... TreeComponents>
template <typename C>
void Treent<TreeComponents...>::attachChildComponent(const TreentRef &child)
{
  C::attachToParent(child->template component<C>(), component<C>());
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
  component<C>()->detachFromParent();
}

template <typename ... TreeComponents>
template <typename C1, typename C2, typename ... Components>
void Treent<TreeComponents...>::detachComponent()
{
  detachComponent<C1>();
  return detachComponent<C2, Components ...>();
}

} // namespace treent
