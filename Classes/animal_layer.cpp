#include "world_ui.h"
#include "economy.h"

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


AnimalMapLayer* AnimalMapLayer::create(AnimalWorld* model)
{
	AnimalMapLayer* result = new AnimalMapLayer();
	// result->_model = model;
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

bool AnimalMapLayer::init()
{
	if (!MapView::init())
	{
		return false;
	}

	_animals = Node::create();
	addChild(_animals);

	return true;
}

bool AnimalMapLayer::is_map_point(cocos2d::Vec2& p)
{
	return p.x > 370 || p.y > 222;
}

//bool AnimalMapLayer::onTouchBegan(Touch* touch, Event  *event)
//{
//	auto p = touch->getLocation();
//	p = convertToNodeSpace(p);
//	Area* a = get_area(p);
//	Animal* ani = model().find_animal(a);
//	if (ani)
//	{
//
//	}
//	else
//	{
//		create_animal(a, model().get_species()[0]);
//	}
//	return true;
//}
//
//void AnimalMapLayer::onTouchEnded(Touch* touch, Event  *event)
//{
//	auto s = touch->getStartLocation();
//	auto p = touch->getLocation();
//	if (is_map_point(touch->getLocationInView()) && (p - s).length() < 10)
//	{
//		//p = _items->convertToNodeSpace(p);
//		//add_item(_mode, p.x, p.y);
//	}
//}
//
//
//void AnimalMapLayer::onTouchMoved(Touch* touch, Event  *event)
//{
//	if (is_map_point(touch->getLocationInView()))
//	{
//		auto diff = touch->getDelta();
//		_map->setPosition(_map->getPosition() + diff);
//		//_items->setPosition(_items->getPosition() + diff);
//	}
//}

Animal* AnimalMapLayer::create_animal(Area* a, Species& s)
{
	Animal* ani = _model.create_animal(a, s);
	if (ani)
	{
		create_sprite(ani);
	}
	return ani;
}

Sprite* AnimalMapLayer::create_sprite(Animal* ani)
{
	Area* a = ani->area; 
	//Sprite* sprite = Sprite::create(get_animal_texture(ani->species.id));
	Sprite* sprite = Sprite::create(ani->species.icon_file);
	Rect r = get_rect(a->x, a->y);
	sprite->setPosition(r.getMidX(), r.getMidY());
	// sprite->setScale(0.1f);
	auto size = sprite->getContentSize();
	auto m = std::max(size.width, size.height);
	sprite->setScale((cell_size() - 10) / m);


	_animals->addChild(sprite);
	return sprite;
}

void AnimalMapLayer::create_sprites_from_model()
{
	_animals->removeAllChildrenWithCleanup(true);
	for (Animal* ani : _model.get_animals())
	{
		Sprite* s = create_sprite(ani);
		_animals->addChild(s);
	}
}

void AnimalMapLayer::onDraw(const Mat4 &transform, uint32_t flags)
{
	// calculate roads
	//_model->trade_maps()[_product_id]->routes_to_areas(_product_id);

}

}