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

Factory* FactoryMapLayer::create_factory(Area* a, Industry * s)
{
	Factory* f = world.create_factory(a, s);
	if (f)
	{
		create_sprite(f);
	}
	return f;
}

Factory * FactoryMapLayer::update_or_create_factory(Area * a, Industry * industry)
{
	auto v = world.find_factories(a);
	for (auto f: v)
	{
		if (f->industry->can_upgrade_to(industry))
		{
			f->start_upgrade_to(industry);
			create_sprite(f);
			return f;
		}
	}

	return create_factory(a, industry);
}

Sprite * FactoryMapLayer::create_sprite(Factory * f)
{
	const int m = 4;
	Sprite* s = Sprites::create(f->industry, Size(cell_size() - m, cell_size() - m), true);
	Area* a = f->area;
	auto p = get_point(a);;
	_factories->addChild(s);

	FactorySprite* fs = new FactorySprite();
	fs->_factory = f;
	fs->_nodes.push_back(s);
	fs->setPosition(p);
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

void FactoryMapLayer::delete_factory(Factory * f)
{
	auto it = find_if(_factory_sprites.begin(), _factory_sprites.end(), [f](FactorySprite* fs) {
		return fs->_factory == f;
	});

	if (it != _factory_sprites.end())
	{
		delete *it;
		_factory_sprites.erase(it);
	}

	world.delete_factory(f);
}

}