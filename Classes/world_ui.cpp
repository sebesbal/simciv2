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

//const int spec_count = 6;
//const int mat_count = 4;


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

WorldUI::WorldUI() : _menu_size(64, 64), view_mode(0), new_view_mode(0), _drag_start(false)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto w = visibleSize.width;
	auto h = visibleSize.height;

	this->schedule(schedule_selector(WorldUI::tick), 0.05f, kRepeatForever, 0.f);
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(WorldUI::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(WorldUI::onTouchEnded, this);
	listener->onTouchMoved = CC_CALLBACK_2(WorldUI::onTouchMoved, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	load_from_tmx("simciv.tmx");

	_main_menu = create_left_menu();
	this->addChild(_main_menu);

	_species_browser = create_species_browser();
	this->addChild(_species_browser);

	_plants_browser = create_plants_browser();
	this->addChild(_plants_browser);

	_species_view = SpeciesView::create();
	_species_view->setAnchorPoint(Vec2(1, 1));
	this->addChild(_species_view);
	//auto s = new Species();
	//// s->id = 0;
	//s->build_cost.push_back(9);
	//s->build_cost.push_back(8);
	//s->build_cost.push_back(7);
	//s->build_cost.push_back(6);
	//s->build_cost.push_back(5);
	//s->build_cost.push_back(4);
	//_species_view->set_species(s);
	_species_view->set_species(&_model.get_species().at(0));

	_layers_panel = create_layers_panel();
	_layers_panel->setAnchorPoint(Vec2(1, 1));
	this->addChild(_layers_panel);

	_animal_view = AnimalView::create();
	_animal_view->setAnchorPoint(Vec2(1, 1));
	_animal_view->setVisible(false);
	this->addChild(_animal_view);

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

	static int k = 0;
	//if (k % 10 == 0)
	{
		_model.update();
	}

	//if (k % 10 == 0)
	{
		_plant_layer->update(f);
	}

	++k;
}

void WorldUI::load_from_tmx(std::string tmx)
{
	_map = TMXTiledMap::create(tmx);
	TMXTiledMap* m = (TMXTiledMap*)_map;
	auto size = m->getMapSize();
	m->setAnchorPoint(Vec2(0.5, 0.5));
	m->setScale(1.5);
	this->addChild(_map);

	_model.create_map(size.width, size.height, material_count);

	Node* v = _plant_layer = PlantMapLayer::create(&_model, info);
	v->setVisible(true);
	v->setAnchorPoint(Vec2(0, 0));
	v->setPosition(Vec2(0, 0));
	v->setContentSize(_map->getContentSize());
	views.push_back(v);
	_map->addChild(v);

	v = _animal_layer = AnimalMapLayer::create(&_model);
	_animal_layer->create_sprites_from_model();
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
	if (event->isStopped()) return false;
	_drag_start = false;
	auto p = touch->getLocation();
	_mouse_down_pos = p;
	p = _map->convertToNodeSpace(p);

	Area* a = _animal_layer->get_area(p);
	Animal* ani = _model.find_animal(a);
	if (ani)
	{
		_animal_view->set_animal(ani);
		_animal_view->setVisible(true);
		Species* s = &ani->species;
		_species_view->set_species(s);
		_species_view->setVisible(true);
		this->info.species = s;
		info.plant_id = s->color + s->level * level_count;
		set_state(UIS_ANIMAL);
	}
	else
	{
		_animal_view->setVisible(false);
	}

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
		// p = _animal_layer->convertToNodeSpace(p);
		p = _map->convertToNodeSpace(p);
		Area* a = _animal_layer->get_area(p);
		Animal* ani = _model.find_animal(a);
		if (ani)
		{

		}
		else
		{
			if (!_drag_start)
			{
				Species* s = info.species;
				if (s)
				{
					_animal_layer->create_animal(a, *s);
				}
			}
		}
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
	Vec2 p = touch->getLocation();
	Vec2 d(0, 0);
	if (_drag_start)
	{
		d = touch->getDelta();
	}
	else
	{
		d = p - _mouse_down_pos;
		if (d.length() > 5)
		{
			_drag_start = true;
		}
	}
	if (_drag_start)
	{
		_map->setPosition(_map->getPosition() + d);
	}
}

RadioMenu* WorldUI::create_left_menu()
{
	RadioMenu* result = RadioMenu::create();
	result->set_toggle(true);

	result->add_row();
	auto btn = MenuButton::create(get_animal_texture(0));
	result->add_radio_button(btn);

	result->add_row();
	btn = MenuButton::create(get_plant_texture(0));
	result->add_radio_button(btn);

	result->set_on_changed([this](MenuButton* btn) {
		if (!btn)
		{
			this->set_state(UIS_NONE);
			return;
		}

		switch (btn->getTag())
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
	for (int level = 0; level < level_count; ++level)
	{
		result->add_row();
		for (int color = 0; color < color_count; ++color)
		{
			Species* s = _model.get_species(level, color);
			auto btn = MenuButton::create(s->icon_file);
			btn->setUserData(s);
			result->add_radio_button(btn);
		}
	}
	result->add_row();
	Species* s = _model.get_storage_species();
	auto btn = MenuButton::create(s->icon_file);
	btn->setUserData(s);
	result->add_radio_button(btn);

	result->set_selected_btn(0);
	result->set_on_changed([this](MenuButton* btn) {
		//int id = btn->getTag();
		// this->info.animal_id = id;
		Species* s = (Species*)btn->getUserData();
		this->info.species = s;
		_species_view->set_species(s);
	});
	return result;
}

RadioMenu* WorldUI::create_plants_browser()
{
	RadioMenu* result = RadioMenu::create();
	for (int level = 0; level < level_count; ++level)
	{
		result->add_row();
		for (int color = 0; color < color_count; ++color)
		{
			auto btn = MenuButton::create(_model.get_species().at(mat_id(level, color)).icon_file);
			result->add_radio_button(btn);
		}
	}


	//for (int i = 0; i < mat_count; ++i)
	//{
	//	auto btn = MenuButton::create(get_plant_texture(i));
	//	result->add_radio_button(btn);
	//}

	result->set_selected_btn(0);
	result->set_on_changed([this](MenuButton* btn) {
		info.plant_id = btn->getTag();
		set_state(_state);
	});
	return result;
}

cocos2d::Node* WorldUI::create_layers_panel()
{
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

	// ==============================================================================================
	// PRICE - VOL - RES
	info.price_vol_mode = 0;
	defvec(vec1, "Price", "Volume", "Res.")
	auto rb = RadioBox::create(&info.price_vol_mode, vec1, hh, marginy);
	rb->setLayoutParameter(p);
	left_menu->addChild(rb);

	// ==============================================================================================
	// SUPPLY - CONSUMPTION
	info.produce_consume_mode = 2;
	defvec(vec2, "Supply", "Cons.", "Both")
	rb = RadioBox::create(&info.produce_consume_mode, vec2, hh, marginy);
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
	auto cb_transport = labelled_cb("Routes", info.show_transport, [this](Ref* pSender, CheckBox::EventType type) {
		info.show_transport = !info.show_transport;
	});
	cb_transport->setLayoutParameter(p);
	left_menu->addChild(cb_transport);

	info.show_grid = false;
	info.plant_id = 0;
	// info.animal_id = 0;

	return left_menu;
}

void WorldUI::setContentSize(const Size & var)
{
	Layer::setContentSize(var);
	auto s = Director::getInstance()->getWinSize();
	int h = var.height;

	_map->setPosition(var / 2);

	int m = 20;
	_main_menu->setPosition(Vec2(m, h - m));
	_species_browser->setPosition(Vec2(m + 64 + 10, h - m));
	_plants_browser->setPosition(Vec2(m + 64 + 10, h - m));
	_species_view->setPosition(Vec2(var.width, h));
	_layers_panel->setPosition(Vec2(var.width, h));
	auto r = _species_view->getBoundingBox();
	_animal_view->setPosition(Vec2(r.getMaxX(), r.getMinY()));
}

void WorldUI::set_state(UIState state)
{
	_state = state;
	bool animals = _state == UIS_ANIMAL;
	bool plants = _state == UIS_PLANTS;
	_species_browser->setVisible(animals);
	_species_view->setVisible(animals);
	_plants_browser->setVisible(plants);
	_layers_panel->setVisible(plants);
	_plant_layer->setVisible(info.plant_id > -1);
}

}