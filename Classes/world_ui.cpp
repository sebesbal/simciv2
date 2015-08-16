#include "world_ui.h"
#include "ui\UICheckBox.h"
#include "ui\UILayout.h"
#include "ui\UIHBox.h"
#include "ui\UIVBox.h"
#include "ui\UIText.h"
#include "ui\UIButton.h"
//#include "CC
#include "base\ccTypes.h"
#include <algorithm>
#include "economy.h"
#include "controls.h"

namespace simciv
{

USING_NS_CC;
using namespace std;

Scene* WorldUI::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    WorldUI *layer = WorldUI::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

WorldUI::WorldUI()
{
	this->schedule(schedule_selector(WorldUI::tick), 0.05, kRepeatForever, 0);
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(WorldUI::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(WorldUI::onTouchEnded, this);
	listener->onTouchMoved = CC_CALLBACK_2(WorldUI::onTouchMoved, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	load_from_tmx("simciv.tmx");

	auto sv = SpeciesView::create();
	this->addChild(sv);
	sv->setPosition(Vec2(500, 500));

	auto s = new Species();
	s->id = 0;
	s->build_cost.push_back(9);
	s->build_cost.push_back(8);
	s->build_cost.push_back(7);
	s->build_cost.push_back(6);
	s->build_cost.push_back(5);
	s->build_cost.push_back(4);
	//s->build_cost.push_back(3);
	//s->build_cost.push_back(2);
	//s->build_cost.push_back(1);
	sv->set_species(s);
}

void WorldUI::tick(float f)
{
	if (view_mode != new_view_mode)
	{
		auto old_view = views[view_mode];
		old_view->setVisible(false);

		view_mode = new_view_mode;
		auto new_view = views[view_mode];
		new_view->setVisible(true);
	}

	_model.end_turn();
}

void WorldUI::load_from_tmx(std::string tmx)
{
	_map = TMXTiledMap::create(tmx);
	TMXTiledMap* m = (TMXTiledMap*)_map;
	auto size = m->getMapSize();
	m->setAnchorPoint(Vec2(0, 0));
	this->addChild(_map);
	_map->setPosition(0, 0);

	_model.create_map(size.width, size.height, 4);

	Node* v = ProdView::create(&_model);
	v->setVisible(true);
	v->setAnchorPoint(Vec2(0, 0));
	v->setPosition(Vec2(0, 0));
	v->setContentSize(_map->getContentSize());
	views.push_back(v);
	_map->addChild(v);

	v = AnimalMapLayer::create(&_model);
	v->setVisible(false);
	v->setAnchorPoint(Vec2(0, 0));
	v->setPosition(Vec2(0, 0));
	v->setContentSize(_map->getContentSize());
	views.push_back(v);
	_map->addChild(v);
}

void WorldUI::onEnter()
{
	setContentSize(Size(500, 500));
	Layer::onEnter();
}

void WorldUI::menuCloseCallback(Ref* sender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

bool WorldUI::onTouchBegan(Touch* touch, Event  *event)
{
	return true;
}

void WorldUI::onTouchEnded(Touch* touch, Event  *event)
{

}

void WorldUI::onTouchMoved(Touch* touch, Event  *event)
{
	//if (is_map_point(touch->getLocationInView()))
	{
		auto diff = touch->getDelta();
		_map->setPosition(_map->getPosition() + diff);
		//_items->setPosition(_items->getPosition() + diff);
	}
}

}