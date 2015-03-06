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

#include "Treent.h"

namespace treent
{

///
/// ScopedTreents destroy their composed treent when falling out of scope.
/// They are move-only types.
/// Use these iff you need to store some member data that won't fit in a component.
///
template <typename ... T>
class ScopedTreentT : public TreentT<T...>
{
public:
  ScopedTreentT() = default;

  explicit ScopedTreentT(const entityx::Entity &entity)
  : TreentT<T...>(entity)
  {}

  ScopedTreentT(const ScopedTreentT &other) = delete;

  ~ScopedTreentT()
  {
    if (this->entity())
    {
      this->entity().destroy();
    }
  }

  ScopedTreentT(ScopedTreentT &&other)
  : TreentT<T...>(other.entity())
  {
    other._entity = entityx::Entity();
  }
private:

};

} // namespace treent
