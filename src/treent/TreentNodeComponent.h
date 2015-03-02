
#pragma once

#include "entityx/Entity.h"

namespace treent
{

///
/// Provides a reference back to treent from entity system.
/// In case you want to destroy the node in an update function.
///
template <typename TreeType>
struct TreentNodeComponent : public entityx::Component<TreentNodeComponent<TreeType>>
{
  explicit TreentNodeComponent(TreeType *treent)
  : _treent(treent)
  {}

	~TreentNodeComponent()
	{
		// Mark entity as invalid.
		if (_treent) {
			auto t = _treent->removeFromParent();
			t->_entity = entityx::Entity();
		}
	}

  TreeType *_treent = nullptr;
};

} // namespace treent
