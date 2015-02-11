# Treent [WIP]

Trees within/around an entity+component system.

Based on ideas initially explored inside Pockets/Treent. Provides object lifetime management and a tree structure for grouping related objects.

## Treent is comprised of three main parts:

The Treent template manages the lifetime of an entity and makes sure the entity is initialized with all the specified TreeComponents. Think of Treents and their subclasses as code-defined prefabs of entities and components.

The TreeComponent template provides methods for maintaining and traversing a hierarchy of components. It is used as a base class for those few components that behave best in a tree, like a transformation matrix or object transparency.

TreentComponent stores a reference to the Treent node so it is easy to get back to Treent from within a normal entity iteration. Not yet implemented.

## Concepts:

Trees are good for hierarchical groupings of homogeneous content.

Entities are good for non-hierarchical groupings of heterogeneous content.

Treents are a bit of both; they wire up the hierarchical components and let you add whatever other components you like just like a regular entity.
