# Treent [WIP]

Trees within/around and entity+component system.

Based on ideas initially explored inside Pockets/Treent. Provides object lifetime management and a tree structure for grouping related objects.

Treent is comprised of three main parts:

TreeComponent template provides methods for maintaining and traversing a hierarchy of components.
(Maybe broken down further to just a Tree<Derived>, where it gets the reftype from the derived class.)

A Treent template manages the lifetime of an entity and makes sure the entity is initialized with all the specified TreeComponents.

TreentComponent stores a reference to the Treent node so it is easy to get back to Treent from within a normal entity iteration.
