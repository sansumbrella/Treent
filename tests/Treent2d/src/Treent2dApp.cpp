#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "entityx/Entity.h"
#include "treent/2d/Treent2d.h"
#include "treent/TreentBase.h"
#include "treent/ScopedTreent.h"

#include "cinder/svg/Svg.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Treent2dApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

using ScopedTreent = treent::ScopedTreentT<treent::TransformComponent, treent::StyleComponent>;

void Treent2dApp::setup()
{
  entityx::EventManager events;
  entityx::EntityManager entities(events);
  treent::SharedEntities::instance().setup(entities);

  auto ent = treent::Treent(entities.create());
  auto c = ent.createChild();

  assert( c.hasComponent<treent::TransformComponent>() );
  assert( c.hasComponent<treent::StyleComponent>() );

  ent.removeChild(c.entity());
  ent.removeChild(c.entity());
  ent.appendChild(c.entity());

  assert(ent.hasComponent<treent::TransformComponent>());
  assert(ent.hasComponent<treent::StyleComponent>());

  auto e = entities.create();
  {
    auto scp = ScopedTreent(e);
    auto b = std::move(scp);
    assert(e.valid());
    assert(b.valid());
    assert(scp.valid() == false);
  }
  assert(e.valid() == false);

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
