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
using namespace ui;

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

	_layers_panel = create_layers_panel();
	_layers_panel->setAnchorPoint(Vec2(1, 1));
	this->addChild(_layers_panel);

	//update_panels(false, false);
	set_state(UIS_NONE);
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

	Node* v = _product_view = ProdView::create(&_model);
	v->setVisible(true);
	v->setAnchorPoint(Vec2(0, 0));
	v->setPosition(Vec2(0, 0));
	v->setContentSize(_map->getContentSize());
	views.push_back(v);
	_map->addChild(v);

	v = _animal_view = AnimalMapLayer::create(&_model);
	_animal_view->create_sprites_from_model();
	v->setVisible(true);
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
	if (event->isStopped()) return;



	switch (_state)
	{
	case simciv::UIS_NONE:
		break;
	case simciv::UIS_ANIMAL:
		{
			auto p = touch->getLocation();
			p = _animal_view->convertToNodeSpace(p);
			Area* a = _animal_view->get_area(p);
			
			//_animal_view->create_animal(a, );
		}
		break;
	case simciv::UIS_PLANTS:
		break;
	default:
		break;
	}
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
		//bool animals = id == 0;
		//bool plants = id == 1;
		// this->update_panels(animals, plants);
		switch (id)
		{
		case 0:
			this->set_state(UIS_ANIMAL);
			break;
		case 1:
			this->set_state(UIS_PLANTS);
			break;
		default:
			this->set_state(UIS_NONE);
			break;
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
	result->set_on_changed([this](int id) {
		this->_product_view->get_info().product_id = id;
	});
	return result;
}

cocos2d::Node* WorldUI::create_layers_panel()
{
	// Node* result = Node::create();
	auto& info = _product_view->get_info();

	auto s = Size(20, 20);
	LinearLayoutParameter* p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::TOP);
	p->setMargin(Margin(2, 2, 2, 2));
	LinearLayoutParameter* q = LinearLayoutParameter::create();
	q->setGravity(LinearLayoutParameter::LinearGravity::LEFT);
	//q->setMargin(Margin(2, 2, 2, 2));

	// left menu
	// auto 
	auto left_menu = VBox::create();
	left_menu->setContentSize(Size(300, 300));
	left_menu->setBackGroundColor(def_bck_color3B);
	left_menu->setBackGroundColorType(Layout::BackGroundColorType::SOLID);

	p = LinearLayoutParameter::create();
	p->setMargin(Margin(10, 5, 2, 5));
	p->setGravity(LinearLayoutParameter::LinearGravity::LEFT);

	int hh = 30;
	int marginy = 0;

	//// ==============================================================================================
	//// PRODUCT
	//defvec(vec0, "img/_factory_red.png", "img/_factory_blue.png", "img/_factory_green.png", "img/_factory_yellow.png")
	//	auto rb = RadioBox::create(&info.product_id, vec0, hh, marginy);
	//left_menu->addChild(rb);

	// ==============================================================================================
	// PRICE - VOL - RES
	info.show_price_vol_mode = 0;
	defvec(vec1, "Price", "Volume", "Res.")
	auto rb = RadioBox::create(&info.show_price_vol_mode, vec1, hh, marginy);
	rb->setLayoutParameter(p);
	left_menu->addChild(rb);

	// ==============================================================================================
	// SUPPLY - CONSUMPTION
	info.show_sup_con_mode = 2;
	defvec(vec2, "Supply", "Cons.", "Both")
	rb = RadioBox::create(&info.show_sup_con_mode, vec2, hh, marginy);
	rb->setLayoutParameter(p);
	left_menu->addChild(rb);

	// ==============================================================================================
	// BACKGROUND
	auto cb_bck = labelled_cb("Background", false, [this](Ref* pSender, CheckBox::EventType type) {
		_map->setVisible(type == CheckBox::EventType::SELECTED);
	});
	cb_bck->setLayoutParameter(p);
	left_menu->addChild(cb_bck);

	// ==============================================================================================--
	// TRANSPORT
	info.show_transport = true;
	auto cb_transport = labelled_cb("Routes", info.show_transport, [this, &info](Ref* pSender, CheckBox::EventType type) {
		info.show_transport = !info.show_transport;
	});
	cb_transport->setLayoutParameter(p);
	left_menu->addChild(cb_transport);

	left_menu->setAnchorPoint(Vec2(0, 1));
	//left_menu->setPosition(Vec2(0, size.height - 50));

	// this->addChild(left_menu);

	//set_price_vol_mode(0);
	//set_sup_con_mode(2);
	info.show_grid = false;


	////Node*
	//_items = Node::create();
	//_items->setAnchorPoint(Vec2(0, 0));
	////_items->setPosition(Vec2(size / 2) - _table / 2);
	////_items->setPosition(Vec2(0, 0));

	//this->addChild(_items, 0, 1);
	//_items->setLocalZOrder(1);
	////_items->setContentSize(_table);


	info.mode = IT_FACTORY;
	info.product_id = 0;

	//add_item(IT_FACTORY, _table.width / 3, _table.height / 2);
	//add_item(IT_MINE, 2 * _table.width / 3, _table.height / 2);

	return left_menu;
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
	_layers_panel->setPosition(Vec2(var.width, h));
}

void WorldUI::update_panels(bool animals, bool plants)
{
	_species_browser->setVisible(animals);
	_species_view->setVisible(animals);
	_layers_panel->setVisible(plants);
	_product_view->setVisible(plants);
}

void WorldUI::set_state(UIState state)
{
	_state = state;
	bool animals = _state == UIS_ANIMAL;
	bool plants = _state == UIS_PLANTS;
	_species_browser->setVisible(animals);
	_species_view->setVisible(animals);
	_layers_panel->setVisible(plants);
	_product_view->setVisible(plants);
}

}