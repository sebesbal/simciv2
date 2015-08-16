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


AnimalMapLayer* AnimalMapLayer::create(WorldModel* model)
{
	AnimalMapLayer* result = new AnimalMapLayer();
	result->_model = model;
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

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto w = visibleSize.width;
	auto h = visibleSize.height;

	left_menu = VBox::create();
	auto p = LinearLayoutParameter::create();
	p->setMargin(Margin(2, 2, 2, 2));
	p->setGravity(LinearLayoutParameter::LinearGravity::LEFT);
}

bool AnimalMapLayer::is_map_point(cocos2d::Vec2& p)
{
	return p.x > 370 || p.y > 222;
}

void AnimalMapLayer::onTouchEnded(Touch* touch, Event  *event)
{
	auto s = touch->getStartLocation();
	auto p = touch->getLocation();
	if (is_map_point(touch->getLocationInView()) && (p - s).length() < 10)
	{
		//p = _items->convertToNodeSpace(p);
		//add_item(_mode, p.x, p.y);
	}
}


void AnimalMapLayer::onTouchMoved(Touch* touch, Event  *event)
{
	if (is_map_point(touch->getLocationInView()))
	{
		auto diff = touch->getDelta();
		_map->setPosition(_map->getPosition() + diff);
		//_items->setPosition(_items->getPosition() + diff);
	}
}

void AnimalMapLayer::onDraw(const Mat4 &transform, uint32_t flags)
{
	// calculate roads
	//_model->products()[_product_id]->routes_to_areas(_product_id);

}

}