//
//  Copyright (c) 2015 David Wicks, sansumbrella.com.
//  All rights reserved.
//

#pragma once

#include "TreentBase.h"
#include "treent/Owner.h"

namespace treent
{

class TreentManager : public Owner
{
public:
  TreentManager();
  ~TreentManager();

  void destroyChild(TreentBase *child) final override;

  /// Creates and returns a treent node as a child of this Treent.
  TreentBase&     createChild();
  /// Creates and returns a subclass of treent node as a child of this Treent.
  /// Args are passed to the constructor after the entity manager.
  template <typename TreentType, typename ... Parameters>
  TreentType& createChild(Parameters&& ... parameters);
  /// Appends a child to Treent, transferring ownership to the parent.
  void        appendChild(TreentBaseURef &&child);

private:
  std::vector<TreentBaseURef> _children;
};

} // namespace treent
