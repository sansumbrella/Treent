
namespace treent
{

///
/// Provides a reference back to treent from entity system.
/// In case you want to destroy the node in an update function.
///
template <typename TreeType>
struct TreentNodeComponent : public <entityx::Component>
{
  explicit TreentNodeComponent(TreeType *treent)
  : _treent(treent)
  {}

  TreeType *_treent = nullptr;
};

} // namespace treent