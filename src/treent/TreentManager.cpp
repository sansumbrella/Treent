//
//  Copyright (c) 2015 David Wicks, sansumbrella.com.
//  All rights reserved.
//

#include "TreentManager.h"
#include "cinder/Log.h"

using namespace treent;

TreentManager::TreentManager()
{}

TreentManager::~TreentManager()
{}

void TreentManager::destroyChild(treent::TreentBase *child)
{
  auto comp = [child] (const TreentBaseURef &c) {
    return c.get() == child;
  };

  auto iter = std::find_if(_children.begin(), _children.end(), comp);

  if (iter != _children.end()) {
    _children.erase(iter);
  }
  else {
    CI_LOG_W("Attempt to destroy child not owned by manager: " << child);
  }
}
