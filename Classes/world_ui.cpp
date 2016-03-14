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
#include "trade.h"
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

WorldUI::WorldUI() : _menu_size(64, 64), view_mode(0), new_view_mode(0), _drag_start(false), _paused(false), _speed(1), _popup(NULL)
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

	auto mouse = EventListenerMouse::create();
	//mouse->onMouseDown = CC_CALLBACK_1(WorldUI::onMouseDown, this);
	mouse->onMouseMove = CC_CALLBACK_1(WorldUI::onMouseMove, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);

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
	//auto s = new Industry();
	//// s->id = 0;
	//s->build_cost.push_back(9);
	//s->build_cost.push_back(8);
	//s->build_cost.push_back(7);
	//s->build_cost.push_back(6);
	//s->build_cost.push_back(5);
	//s->build_cost.push_back(4);
	//_species_view->set_species(s);
	_species_view->set_species(_model.get_species().at(0));

	create_plant_layers_panel();
	create_factory_layers_panel();

	_factory_view = AnimalView::create();
	_factory_view->setAnchorPoint(Vec2(1, 1));
	_factory_view->setVisible(false);
	this->addChild(_factory_view);

	create_play_panel();

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
	if (!_paused && k % _speed == 0)
	{
		_model.update();
	}

	//if (k % 10 == 0)
	{
		_plant_layer->update(f);
	}

	if (_factory_view->isVisible())
	{
		//_factory_view->scheduleUpdate();
		//_factory_view->upda
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

	_model.create(size.width, size.height, product_count);

	Node* v = _plant_layer = PlantMapLayer::create(&_model, info);
	v->setVisible(true);
	v->setAnchorPoint(Vec2(0, 0));
	v->setPosition(Vec2(0, 0));
	v->setContentSize(_map->getContentSize());
	views.push_back(v);
	_map->addChild(v);

	v = _factory_layer = AnimalMapLayer::create(&_model);
	_factory_layer->create_sprites_from_model();
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

	Area* a = _factory_layer->get_area(p);
	Factory* ani = _model.find_factory(a);
	if (ani)
	{
		_factory_view->set_factory(ani);
		_factory_view->setVisible(true);
		Industry* s = &ani->industry;
		_species_view->set_species(s);
		_species_view->setVisible(true);
		this->info.industry = s;
		info.plant = _model.get_plants()[s->id]; //s->color + s->level * level_count;
		set_state(UIS_factory);
	}
	else
	{
		_factory_view->setVisible(false);
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
	case simciv::UIS_factory:
	{
		auto p = touch->getLocation();
		// p = _factory_layer->convertToNodeSpace(p);
		p = _map->convertToNodeSpace(p);
		Area* a = _factory_layer->get_area(p);
		Factory* ani = _model.find_factory(a);
		if (ani)
		{

		}
		else
		{
			if (!_drag_start)
			{
				Industry* s = info.industry;
				if (s)
				{
					_factory_layer->create_factory(a, *s);
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

void WorldUI::onMouseMove(Event *event)
{
	EventMouse* mouseEvent = dynamic_cast<EventMouse*>(event);
	update_popup(mouseEvent->getLocationInView());
}


RadioMenu* WorldUI::create_left_menu()
{
	RadioMenu* result = RadioMenu::create();
	result->set_toggle(true);

	result->add_row();
	auto btn = MenuButton::create(get_factory_texture(0));
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
			this->set_state(UIS_factory);
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

void WorldUI::create_play_panel()
{
	_play_panel = HBox::create();

	auto f = [this](string s){
		auto btn = ui::Button::create(s);
		btn->setSize(Size(64, 64));
		btn->ignoreContentAdaptWithSize(false);
		_play_panel->addChild(btn);
		return btn;
	};

	auto btn = f("img/play.png");
	btn->addClickEventListener([this](Ref* ref) {
		_paused = !_paused;
	});
	

	btn = f("img/plus.png");
	btn->addClickEventListener([this](Ref* ref) {
		++_speed;
	});

	btn = f("img/minus.png");
	btn->addClickEventListener([this](Ref* ref) {
		--_speed;
		_speed = std::max(1, _speed);
	});

	this->addChild(_play_panel);
	_play_panel->setAnchorPoint(Vec2(0, 0));
	_play_panel->setSize(btn->getSize());
}

RadioMenu* WorldUI::create_species_browser()
{
	RadioMenu* result = RadioMenu::create();

	auto& industry = _model.get_species();
	int i = 0;
	for (auto s : industry)
	{
		if (i++ % 3 == 0) result->add_row();
		auto btn = MenuButton::create(s->icon_file);
		btn->setUserData(s);
		result->add_radio_button(btn);
	}

	result->set_selected_btn(0);
	result->set_on_changed([this](MenuButton* btn) {
		Industry* s = (Industry*)btn->getUserData();
		this->info.industry = s;
		_species_view->set_species(s);
		set_state(_state);
	});
	return result;
}

RadioMenu* WorldUI::create_plants_browser()
{
	RadioMenu* result = RadioMenu::create();
	auto& plants = _model.get_plants();
	int i = 0;
	for (auto plant : plants)
	{
		if (i++ % 3 == 0) result->add_row();
		auto btn = MenuButton::create(plant->icon_file);
		btn->setUserData(plant);
		result->add_radio_button(btn);
	}

	result->set_selected_btn(0);
	result->set_on_changed([this](MenuButton* btn) {
		info.plant = (Product*)btn->getUserData(); // _model.get_plants()[btn->getTag()];
		set_state(_state);
	});
	return result;
}

void WorldUI::create_plant_layers_panel()
{
	auto s = Size(20, 20);
	LinearLayoutParameter* p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::TOP);
	p->setMargin(Margin(2, 2, 2, 2));
	LinearLayoutParameter* q = LinearLayoutParameter::create();
	q->setGravity(LinearLayoutParameter::LinearGravity::LEFT);

	_plant_layers_panel = VBox::create();
	_plant_layers_panel->setContentSize(Size(300, 80));
	_plant_layers_panel->setBackGroundColor(def_bck_color3B);
	_plant_layers_panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);

	p = LinearLayoutParameter::create();
	p->setMargin(Margin(10, 5, 2, 5));
	p->setGravity(LinearLayoutParameter::LinearGravity::LEFT);

	int hh = 30;
	int marginy = 0;

	// ==============================================================================================
	// PRICE - VOL - RES
	info.mode = MM_PRICE_SELL;
	defvec(vec1, "Sell", "Buy", "Res.")
		auto rb = RadioBox::create(vec1, hh, marginy);
	rb->setLayoutParameter(p);
	rb->changed = [this](int id) {
		switch (id)
		{
		case 0:
			info.mode = MM_PRICE_SELL;
			break;
		case 1:
			info.mode = MM_PRICE_BUY;
			break;
		case 2:
			info.mode = MM_PLANT_RESOURCES;
			break;
		default:
			break;
		}
	};
	_plant_layers_panel->addChild(rb);
	_plant_layers_panel->setAnchorPoint(Vec2(1, 0));
	
	_on_state_plant = [=](){
		rb->update();
	};

	this->addChild(_plant_layers_panel);
}

void WorldUI::create_factory_layers_panel()
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
	_factory_layers_panel = VBox::create();
	_factory_layers_panel->setContentSize(Size(300, 80));
	_factory_layers_panel->setBackGroundColor(def_bck_color3B);
	_factory_layers_panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);

	p = LinearLayoutParameter::create();
	p->setMargin(Margin(10, 5, 2, 5));
	p->setGravity(LinearLayoutParameter::LinearGravity::LEFT);

	int hh = 30;
	int marginy = 0;

	// ==============================================================================================
	// PRICE - VOL - RES
	defvec(vec1, "Profit", "Cost", "Res.", "Both")
	auto rb = RadioBox::create(vec1, hh, marginy);
	rb->setLayoutParameter(p);
	rb->changed = [this](int id) {
		switch (id)
		{
		case 0:
			info.mode = MM_PROFIT;
			break;
		case 1:
			info.mode = MM_BUILD_COST;
			break;
		case 2:
			info.mode = MM_SPECIES_RESOURCES;
			break;
		case 3:
			info.mode = MM_PROFIT_RES;
			break;
		default:
			break;
		}
	};

	_on_state_factory = [=](){
		rb->update();
	};

	_factory_layers_panel->addChild(rb);
	_factory_layers_panel->setAnchorPoint(Vec2(1, 0));
	this->addChild(_factory_layers_panel);
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
	
	auto r = _species_view->getBoundingBox();
	_factory_view->setPosition(Vec2(r.getMaxX(), r.getMinY()));
	_play_panel->setPosition(Vec2(m, m));

	_plant_layers_panel->setPosition(Vec2(var.width, 0));
	_factory_layers_panel->setPosition(Vec2(var.width, 0));
}

void WorldUI::set_state(UIState state)
{
	_state = state;
	bool factories = _state == UIS_factory;
	bool plants = _state == UIS_PLANTS;
	_species_browser->setVisible(factories);
	_species_view->setVisible(factories);
	_plants_browser->setVisible(plants);
	_plant_layers_panel->setVisible(plants);
	_factory_layers_panel->setVisible(factories);
	_plant_layer->setVisible(true);

	if (factories)
	{
		if (_popup) _popup->removeFromParent();
		_popup = new AnimalPopup();
		this->addChild(_popup);
		_on_state_factory();
	}
	else if (plants)
	{
		_on_state_plant();
	}
}

void WorldUI::find_child(const cocos2d::Node* n, const Vec2& wp, cocos2d::Node*& child, int& z_order)
{
	auto q = n->convertToNodeSpace(wp);
	for (auto c : n->getChildren())
	{
		if (_popup && c == _popup) continue;

		auto r = c->getBoundingBox();
		if (r.containsPoint(q))
		{
			if (c->getZOrder() >= z_order)
			{
				child = c;
				z_order = c->getZOrder();
			}
		}
		find_child(c, wp, child, z_order);
	}
}

cocos2d::Node* WorldUI::find_child(const cocos2d::Node* node, const Vec2& wp)
{
	cocos2d::Node* child(NULL);
	int z_order = std::numeric_limits<int>::min();
	find_child(node, wp, child, z_order);
	return child;
}

void WorldUI::update_popup(const Vec2& wp)
{
	if (!_popup) return;

	auto q = _map->convertToNodeSpace(wp);
	Area* a = _plant_layer->get_area(q);

	//if (_state == UIState::UIS_factory)
	//{
	//	
	//}

	auto n = find_child(this, wp);
	if (n == _factory_layer)
	{
		if (info.industry)
		{
			_popup->setPosition(wp + Vec2(10, 10));
			((AnimalPopup*)_popup)->set_profit(Info::profit(a, info.industry));
			((AnimalPopup*)_popup)->set_cost(Info::build_cost(a, info.industry));
			_popup->setVisible(true);
		}
	}
	else
	{
		_popup->setVisible(false);
	}
}

}