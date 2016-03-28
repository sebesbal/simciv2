#include "stdafx.h"

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

FactoryMapLayer* g_factory_layer;

bool FactoryMapLayer::init()
{
	if (!MapView::init())
	{
		return false;
	}

	_factories = Node::create();
	addChild(_factories);
	scheduleUpdate();

	return true;
}

void FactoryMapLayer::update(float dt)
{
	MapView::update(dt);
	for (auto fs : _factory_sprites)
	{
		fs->update(dt);
	}
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

Sprite * FactoryMapLayer::create_sprite(Factory * f)
{
	const int m = 4;
	Sprite* s = Sprites::create(&f->industry, Size(cell_size() - m, cell_size() - m), true);
	Area* a = f->area;
	auto p = get_point(a);;
	s->setPosition(p);
	_factories->addChild(s);

	FactorySprite* fs = new FactorySprite();
	fs->_factory = f;
	fs->_sprite = s;
	_factory_sprites.push_back(fs);

	return s;
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