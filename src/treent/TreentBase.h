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
#include "treent/Owner.h"

namespace treent
{

using entityx::Entity;
using entityx::EntityManager;
using entityx::ComponentHandle;

class TreentBase;
using TreentBaseURef = std::unique_ptr<TreentBase>;

///
/// Non-templated base class for Treents.
/// Manages the lifetime of a single entityx::Entity.
///
class TreentBase
{
public:
  TreentBase(EntityManager &entities);
  virtual ~TreentBase();

  /// Treent has unique ownership of an entity, so it shouldn't be copied.
  /// We get much better compiler errors when this constructor is explicitly deleted.
  TreentBase(const TreentBase &other) = delete;
  TreentBase(TreentBase &&other) = delete;

  //
  // Mirror Entity methods. (add/remove components, getOrAssign convenience.)
  //

  /// Returns the underlying entity. Will likely be removed in favor of mirroring interface (so you can't inadvertantly destroy the entity as easily).
  Entity&            entity() __deprecated { return _entity; }

  /// Assign a component to entity, forwarding params to the component constructor.
  template <typename C, typename ... Params>
  ComponentHandle<C> assign(Params&& ... params) { return _entity.assign<C>(std::forward<Params>(params)...); }
  /// Producer for assigning multiple components.
  template <typename C1, typename C2, typename ... Cs>
  void      assign ();

  /// Get a handle to an existing component of the entity.
  template <typename C>
  ComponentHandle<C> component() { return _entity.component<C>(); }

  /// Get a handle to an existing component of the entity.
  template <typename C>
  ComponentHandle<C> get() { return _entity.component<C>(); }

  /// Get a handle to a component of the entity. If the component doesn't exist, it will be default-constructed.
  template <typename C>
  typename C::Handle getOrAssign() { if( hasComponent<C>() ) { return component<C>(); } return assign<C>(); }

  /// Returns true iff the entity has a component of the given type.
  template <typename C>
  bool               hasComponent() const { return _entity.has_component<C>(); }

  /// Remove a component from the entity.
  template <typename C>
  void               remove() { _entity.remove<C>(); }

  /// Destroy the Treent. Accomplished by removing parent's reference to this, which invokes destructor.
  void               destroy() { assert(_owner); _owner->destroyChild(this); }

protected:
  EntityManager&          entities() { return _entities; }
  Owner*                  _owner = nullptr;

private:
  entityx::EntityManager& _entities;
  entityx::Entity         _entity;
};

#pragma mark - Template method implementation

template <typename C1, typename C2, typename ... Cs>
void TreentBase::assign()
{
  assign<C1>();
  assign<C2, Cs...>();
}

} // namespace treent
