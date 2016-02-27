#include "world_ui.h"

namespace simciv
{

USING_NS_CC;
using namespace std;


MapView* MapView::create(WorldModel* model)
{
	MapView* result = new MapView();
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

bool MapView::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
    _map = Sprite::create("img/map.png");
	_table = getContentSize();

    // position the sprite on the center of the screen
	//_map->setAnchorPoint(Vec2(0, 0));
    // _map->setPosition(Vec2(visibleSize / 2));
	// _map->setScale(1);
    // add the sprite as a child to this layer
    this->addChild(_map, 0, 0);
	_map->setLocalZOrder(-1);
	_map->setVisible(false);
	//_model->create_map(_table.width / cs, _table.height / cs, 4);

	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(MapView::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(MapView::onTouchEnded, this);
	listener->onTouchMoved = CC_CALLBACK_2(MapView::onTouchMoved, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	return true;
}

void MapView::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    _customCommand.init(_globalZOrder);
    _customCommand.func = CC_CALLBACK_0(MapView::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}


void MapView::draw_rect(int x, int y, double rate, double alpha)
{
	Rect r = get_rect(x, y);
	
	//DrawPrimitives::drawSolidRect( Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - rate, rate, 0, alpha));
	DrawPrimitives::drawSolidRect( Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - rate, rate, 0, 1));
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(rate, rate, rate, 1));
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(rate, 1- rate, 1-rate, 1));
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1-rate, 1 - rate, rate, 1));
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(rate / 2, 0, rate / 2, 1));
}

void MapView::draw_rect_green(int x, int y, double rate, double alpha)
{
	Rect r = get_rect(x, y);
	//DrawPrimitives::drawSolidRect( Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(0, rate, 0, alpha));
	DrawPrimitives::drawSolidRect( Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(0, rate, 0, 1));
}

Rect MapView::get_rect(int x, int y)
{
	// auto b = _map->getBoundingBox();
	auto b = getBoundingBox();
	return Rect(b.getMinX() + cs * x, b.getMinY() + cs * y, cs, cs);
}

Area* MapView::get_area(Vec2 p)
{
	int x = p.x / cs;
	int y = p.y / cs;
	return _model->get_area(x, y);
}

void MapView::onDraw(const Mat4 &transform, uint32_t flags)
{

}

bool MapView::onTouchBegan(Touch* touch, Event  *event)
{
	return true;
}

void MapView::onTouchEnded(Touch* touch, Event  *event)
{

}

void MapView::onTouchMoved(Touch* touch, Event  *event)
{
	////if (is_map_point(touch->getLocationInView()))
	//{
	//	auto diff = touch->getDelta();
	//	_map->setPosition(_map->getPosition() + diff);
	//	//_items->setPosition(_items->getPosition() + diff);
	//}
}

}