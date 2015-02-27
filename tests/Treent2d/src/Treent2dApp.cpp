#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "entityx/Entity.h"
#include "treent/2d/Treent2d.h"

#include "cinder/svg/Svg.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct Name : public entityx::Component<Name>
{
  Name() = default;
  Name(const std::string &name)
  : _name(name)
  {}

  std::string _name;
};

class ChildClass : public treent::Treent2D
{
public:
  ChildClass (entityx::EntityManager &entities, const std::string &name)
  : Treent(entities)
  {
    assign<Name>( name );
  }
};

class Treent2dApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void Treent2dApp::setup()
{
  entityx::EventManager events;
  entityx::EntityManager entities(events);

  auto ent = make_shared<treent::Treent2D>(entities);
  auto &c = ent->createChild();

  assert( c.hasComponent<treent::TransformComponent>() );
  assert( c.hasComponent<treent::StyleComponent>() );

  auto child = ent->removeChild(&c);
  ent->appendChild(std::move(child));

  auto &b = c.createChild<ChildClass>("so fair");
  assert(b.hasComponent<treent::TransformComponent>());
  assert(b.hasComponent<treent::StyleComponent>());
  assert(b.get<Name>()->_name == "so fair");

  b.destroy();
}

void Treent2dApp::mouseDown( MouseEvent event )
{
}

void Treent2dApp::update()
{
}

void Treent2dApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( Treent2dApp, RendererGl )
