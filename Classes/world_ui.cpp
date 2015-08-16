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

// on "init" you need to initialize your instance
bool WorldUI::init()
{
	// auto rootNode = CSLoader::createNode("MainScene.csb");
	// addChild(rootNode);

	//auto tileMap = TMXTiledMap::create("simciv.tmx"); // createWithXML(, "./");
	//addChild(tileMap);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto w = visibleSize.width;
	auto h = visibleSize.height;

	this->schedule(schedule_selector(WorldUI::tick), 0.05, kRepeatForever, 0);

	int hh = 30;
	int marginy = 20;
	view_mode = 0;
	new_view_mode = 0;
	defvec(vec9, "Prods", "Animals");
	auto rb = RadioBox::create(&new_view_mode, vec9, hh, marginy);
	rb->setZOrder(10);

	//auto left_menu = ui::VBox::create();
	rb->setAnchorPoint(Vec2(0, 1));
	rb->setPosition(Vec2(0, h));
	//left_menu->setSize(Size(100, 100));
	//left_menu->setContentSize(Size(100, 100));
	this->addChild(rb);
	//this->addChild(left_menu);
	//this->addChild(ui::Text::create("lofusz", "arial", 12));



	//_model.create_map(30, 20, 4);

	//Node* v = ProdView::create(&_model);
	//v->setVisible(true);
	//views.push_back(v);
	//this->addChild(v);

	//v = AnimalMapLayer::create(&_model);
	//v->setVisible(false);
	//views.push_back(v);
	//this->addChild(v);

	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(WorldUI::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(WorldUI::onTouchEnded, this);
	listener->onTouchMoved = CC_CALLBACK_2(WorldUI::onTouchMoved, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	load_from_tmx("simciv.tmx");

    return true;
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