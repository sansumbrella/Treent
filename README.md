# Treent [WIP]

Trees within/around an entity+component system.

Based on ideas initially explored inside Pockets/Treent. Provides object lifetime management and a tree structure for grouping related objects.

## Main parts:

Treent decorates an entity to provide familiar-feeling methods for adding and removing children in a hierarchy.

TreeComponents manage the hierarchical data connected between components.

ChildrenComponent manages the lifetime of child entities relative to their parent entity.

Treents by default have no lifetime management: they are valid as long as the entity they compose is valid. When you are done using the Treent interface to an entity, it is safe to let the Treent fall out of scope as the entity and its hierarchy will live on in the underlying entity system.

ScopedTreents provide a means of providing scoped lifetime for an entity. Unlike regular Treents, they will destroy their entity when falling out of scope. For this reason, ScopedTreents are move-only. Note that we cannot prevent entities from being destroyed through other means than falling out of scope. ScopedTreents will become invalid (and return a falsy value on conversion to bool) if their composed entity is destroyed.

## Concepts:

Trees are good for hierarchical groupings of homogeneous content.

Entities are good for non-hierarchical groupings of heterogeneous content.

Treents are a bit of both; they wire up the hierarchical components and let you add whatever other components you like just like a regular entity.
