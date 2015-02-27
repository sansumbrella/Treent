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

namespace treent
{

using entityx::Entity;
using entityx::EntityManager;
using entityx::ComponentHandle;

class SharedEntities
{
public:
  static SharedEntities& instance();
  void   setup(entityx::EntityManager &entities) { _entities = &entities; }

private:
  SharedEntities() = default;
  entityx::EntityManager& entities() { return *_entities; }
  entityx::EntityManager *_entities = nullptr;

  friend class TreentBase;
};

SharedEntities& SharedEntities::instance()
{
  static SharedEntities sInstance;
  return sInstance;
}

template <typename C>
ComponentHandle<C> getOrAssign(entityx::Entity &entity)
{
  if (entity.has_component<C>()) {
    return entity.component<C>();
  }
  return entity.assign<C>();
}

///
/// Non-templated base class for Treents.
/// Mirrors and extends the Entity interface.
/// Treents can have children, whose lifetime is tied to the parent.
///
class TreentBase
{
public:
  /// Constructs an invalid Treent.
  TreentBase() = default;

  /// Constructs a Treent that provides a facade to an entity.
  explicit TreentBase(const Entity &entity): _entity(entity) {}

  virtual ~TreentBase() = default;

  //
  // Mirror Entity methods. (add/remove components, getOrAssign convenience.)
  //

  /// Returns the underlying entity.
  Entity&             entity() { return _entity; }

  /// Assign a component to entity, forwards params to the component constructor.
  template <typename C, typename ... Params>
  ComponentHandle<C>  assign(Params&& ... params) { return _entity.assign<C>(std::forward<Params>(params)...); }
  /// Assign multiple components to entity. No constructor arguments can be passed.
  template <typename C1, typename C2, typename ... Cs>
  void                assign();

  /// Add a component to entity if it doesn't already have a component of that type.
  template <typename C>
  void                assignIfMissing() { if (! hasComponent<C>()) { assign<C>(); } }
  template <typename C1, typename C2, typename ... Cs>
  void                assignIfMissing();

  /// Get a handle to an existing component of the entity.
  template <typename C>
  ComponentHandle<C>  component() { return _entity.component<C>(); }
  /// Get a handle to an existing component of the entity. Same as .component<C>.
  template <typename C>
  ComponentHandle<C>  get() { return _entity.component<C>(); }

  /// Get a handle to a component of the entity.
  /// If the component doesn't exist, it will be default-constructed.
  template <typename C>
  typename C::Handle  getOrAssign() { if( hasComponent<C>() ) { return component<C>(); } return assign<C>(); }

  /// Returns true iff the entity has a component of the given type.
  template <typename C>
  bool                hasComponent() const { return _entity.has_component<C>(); }

  /// Remove a component from the entity.
  template <typename C>
  void                remove() { _entity.remove<C>(); }

  /// Returns true iff this Treent refers to a valid entity.
  bool                valid() const { return _entity.valid(); }
  operator bool       () const { return valid(); }

  /// Destroy the underlying entity. This invalidates this Treent.
  void                destroy() { _entity.destroy(); }

protected:
  static EntityManager&   entities() { return SharedEntities::instance().entities(); }
  entityx::Entity         _entity;
};

#pragma mark - Template method implementation

template <typename C1, typename C2, typename ... Cs>
void TreentBase::assign()
{
  assign<C1>();
  assign<C2, Cs...>();
}

template <typename C1, typename C2, typename ... Cs>
void TreentBase::assignIfMissing()
{
  assignIfMissing<C1>();
  assignIfMissing<C2, Cs...>();
}

} // namespace treent
