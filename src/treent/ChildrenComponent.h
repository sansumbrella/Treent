/*
 * Copyright (c) 2015 David Wicks, sansumbrella.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "entityx/Entity.h"
#include <algorithm>

namespace treent
{

struct ChildrenComponent : public entityx::Component<ChildrenComponent>
{
  ChildrenComponent() = default;
  ~ChildrenComponent();

  /// Add a child to be managed by this ChildrenComponent. It will be destroyed when this component is destroyed.
  void addChild(const entityx::Entity &child) { _children.push_back(child); }
  /// Removes a child from management. Does not destroy the child.
  void removeChild(const entityx::Entity &child);

  std::vector<entityx::Entity>  _children;
};

inline ChildrenComponent::~ChildrenComponent()
{
  for (auto &e : _children) {
		if( e ) {
			e.destroy();
		}
  }
}

inline void ChildrenComponent::removeChild(const entityx::Entity &child)
{
  auto begin = std::remove_if(_children.begin(), _children.end(), [&](const entityx::Entity &e) { return e == child; });
  _children.erase(begin, _children.end());
}

} // namespace treent
