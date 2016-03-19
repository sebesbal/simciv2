#include "world_ui.h"
#include "trade.h"

#if _MSC_VER > 1800
#pragma comment(lib,"libcocos2d_2015.lib")
#pragma comment(lib,"libbox2d_2015.lib")
#pragma comment(lib,"libSpine_2015.lib")
#pragma comment(lib,"libbullet_2015.lib")
#pragma comment(lib,"librecast_2015.lib")
#else
#pragma comment(lib,"libcocos2d_2013.lib")
#pragma comment(lib,"libbox2d_2013.lib")
#pragma comment(lib,"libSpine_2013.lib")
#pragma comment(lib,"libbullet_2013.lib")
#pragma comment(lib,"librecast_2013.lib")
#endif

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;


FactoryMapLayer* FactoryMapLayer::create(World* model)
{
	FactoryMapLayer* result = new FactoryMapLayer();
	// result->world = model;
	if (result && result->init())
	{
		result->autorelease();
		return result;
	}
	else
	{
		CC_SAFE_DELETE(result);
		return nullptr;
	}
}

bool FactoryMapLayer::init()
{
	if (!MapView::init())
	{
		return false;
	}

	_factories = Node::create();
	addChild(_factories);

	return true;
}

Factory* FactoryMapLayer::create_factory(Area* a, Industry& s)
{
	Factory* f = world.create_factory(a, s);
	if (f)
	{
		create_sprite(f);
	}
	return f;
}

Sprite* FactoryMapLayer::create_sprite(Factory* f)
{
	Area* a = f->area; 
	//Sprite* sprite = Sprite::create(get_factory_texture(f->industry.id));
	Sprite* sprite = Sprite::create(f->industry.icon_file);
	Rect r = get_rect(a->x, a->y);
	sprite->setPosition(r.getMidX(), r.getMidY());
	// sprite->setScale(0.1f);
	auto size = sprite->getContentSize();
	auto m = std::max(size.width, size.height);
	sprite->setScale((cell_size() - 10) / m);


	_factories->addChild(sprite);
	return sprite;
}

void FactoryMapLayer::create_sprites_from_model()
{
	_factories->removeAllChildrenWithCleanup(true);
	for (Factory* f : world.get_factories())
	{
		Sprite* s = create_sprite(f);
		_factories->addChild(s);
	}
}

}