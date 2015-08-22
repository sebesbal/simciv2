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

const int spec_count = 6;
const int mat_count = 6;

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

WorldUI::WorldUI() : _menu_size(64, 64)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto w = visibleSize.width;
	auto h = visibleSize.height;

	this->schedule(schedule_selector(WorldUI::tick), 0.05, kRepeatForever, 0);
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(WorldUI::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(WorldUI::onTouchEnded, this);
	listener->onTouchMoved = CC_CALLBACK_2(WorldUI::onTouchMoved, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	load_from_tmx("simciv.tmx");



	_left_menu = create_left_menu();
	this->addChild(_left_menu);
	

	_species_browser = create_species_browser();
	_species_browser->setVisible(false);
	this->addChild(_species_browser);


	_species_view = SpeciesView::create();
	_species_view->setAnchorPoint(Vec2(1, 1));
	this->addChild(_species_view);
	auto s = new Species();
	s->id = 0;
	s->build_cost.push_back(9);
	s->build_cost.push_back(8);
	s->build_cost.push_back(7);
	s->build_cost.push_back(6);
	s->build_cost.push_back(5);
	s->build_cost.push_back(4);
	_species_view->set_species(s);
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
	m->setAnchorPoint(Vec2(0.5, 0.5));
	m->setScale(1.5);
	this->addChild(_map);

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
	// setContentSize(Size(500, 500));
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

RadioMenu* WorldUI::create_left_menu()
{
	RadioMenu* result = RadioMenu::create();
	result->set_toggle(true);

	auto btn = MenuButton::create(get_animal_texture(0));
	result->add_radio_button(btn);
	btn = MenuButton::create("img/layers.png");
	result->add_radio_button(btn);

	result->set_on_changed([this](int id) {
		if (id == 0)
		{
			this->_species_browser->setVisible(true);
		}
		else
		{
			this->_species_browser->setVisible(false);
		}
	});

	return result;
}

RadioMenu* WorldUI::create_species_browser()
{
	RadioMenu* result = RadioMenu::create();
	for (int i = 0; i < spec_count; ++i)
	{
		auto btn = MenuButton::create(get_animal_texture(i));
		result->add_radio_button(btn);
	}
	result->set_selected_btn(0);
	return result;
}

void WorldUI::setContentSize(const Size & var)
{
	Layer::setContentSize(var);
	auto s = Director::getInstance()->getWinSize();
	int h = var.height;

	_map->setPosition(var / 2);

	int m = 20;
	_left_menu->setPosition(Vec2(m, h - m));
	_species_browser->setPosition(Vec2(m + 64 + 10, h - m));
	_species_view->setPosition(Vec2(var.width, h));

}

}