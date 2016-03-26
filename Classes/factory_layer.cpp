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

//Sprite* FactoryMapLayer::create_sprite(Factory* f)
//{
//	Area* a = f->area; 
//	//Sprite* sprite = Sprite::create(get_factory_texture(f->industry.id));
//	Sprite* sprite = Sprite::create(f->industry.icon_file);
//	//Rect r = get_rect(a->x, a->y);
//	//sprite->setPosition(r.getMidX(), r.getMidY());
//	auto p = get_point(a->x, a->y);
//	sprite->setPosition(p.x, p.y);
//	// sprite->setScale(0.1f);
//	auto size = sprite->getContentSize();
//	auto m = std::max(size.width, size.height);
//	sprite->setScale((cell_size() - 10) / m);
//
//
//	_factories->addChild(sprite);
//	return sprite;
//}

Sprite* FactoryMapLayer::create_sprite(Factory* f)
{
	Area* a = f->area;
	auto p = get_point(a->x, a->y);

	//Sprite* bck = Sprite::create("res/img/Circle_Orange.png");
	Sprite* bck = Sprite::create("res/img/shapes/white_circle.png");
	bck->setPosition(p.x, p.y);
	auto size = bck->getContentSize();
	auto m = std::max(size.width, size.height);
	bck->setScale((cell_size() - 2) / m);

	Sprite* sprite = Sprite::create(f->industry.icon_file);
	//sprite->setPosition(p.x, p.y);
	sprite->setPosition(size / 2);
	//sprite->setAnchorPoint(Vec2(0.5, 0.5));
	auto size2 = sprite->getContentSize();
	auto m2 = std::max(size2.width, size2.height);
	sprite->setScale(0.8 * m / m2);

	bck->addChild(sprite);
	_factories->addChild(bck);
	return bck;
}

void FactoryMapLayer::create_sprites_from_model()
{
	_factories->removeAllChildrenWithCleanup(true);
	for (Factory* f : world.get_factories())
	{
		create_sprite(f);
	}
}

}