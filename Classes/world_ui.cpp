#include "stdafx.h"

#include "world_ui.h"

#include "trade.h"
#include "controls.h"
#include "map_view.h"

#define defvec(vec, ...) \
	static const string arr ## vec[] = { __VA_ARGS__ }; \
	vector<string> vec (arr ## vec, arr ## vec + sizeof(arr ## vec) / sizeof(arr ## vec[0]) );
 
namespace simciv
{
	void RUNUI(std::function<void(void)> f)
	{
		Director::getInstance()->getScheduler()->performFunctionInCocosThread(f);
	}

	Scene* WorldUI::createScene(const std::string & config, const std::string & tmx_map)
	{
		// 'scene' is an autorelease object
		auto scene = Scene::create();

		// 'layer' is an autorelease object
		WorldUI *layer = WorldUI::create(config, tmx_map);
		layer->setContentSize(scene->getContentSize());

		// add layer as a child to scene
		scene->addChild(layer);

		// return the scene
		return scene;
	}

	WorldUI * WorldUI::create(const std::string & config, const std::string & tmx_map)
	{
		WorldUI *pRet = new WorldUI();
		if (pRet && pRet->init(config, tmx_map))
		{
			pRet->autorelease();
			return pRet;
		}
		else
		{
			delete pRet;
			pRet = NULL;
			return NULL;
		}
	}

	bool WorldUI::init(const std::string& config, const std::string& tmx_map)
	{
		if (!Layout::init()) return false;
		_menu_size = Size(64, 64);
		_drag_start = false;
		_paused = false;
		_speed = 5;
		_popup = NULL;
		auto visibleSize = Director::getInstance()->getVisibleSize();
		setContentSize(visibleSize);

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

		load_from_file(config, tmx_map);

		_main_menu = create_left_menu();
		this->addChild(_main_menu);

		_industry_browser = create_industry_browser();
		this->addChild(_industry_browser);

		_products_browser = create_products_browser();
		this->addChild(_products_browser);

		_roads_menu = create_roads_menu();
		this->addChild(_roads_menu);

		_military_menu = create_military_menu();
		this->addChild(_military_menu);

		_industry_view = IndustryView::create();
		_industry_view->setAnchorPoint(Vec2(1, 1));
		this->addChild(_industry_view);
		_industry_view->set_industry(world->get_industries().at(0));

		_factory_view = FactoryView::create();
		_factory_view->setAnchorPoint(Vec2(1, 1));
		_factory_view->setVisible(false);
		this->addChild(_factory_view);

		_military_view = MilitaryView::create();
		_military_view->setAnchorPoint(Vec2(1, 1));
		_military_view->setVisible(false);
		this->addChild(_military_view);

		_economy_view = EconomyView::create();
		//_economy_view->setAnchorPoint(Vec2(0, 1));
		_economy_view->setAnchorPoint(Vec2(0.5, 1));
		_economy_view->setVisible(true);
		this->addChild(_economy_view);

		create_option_panels();
		create_play_panel();

		_cursor = Sprite::create("cursor.png");
		double cs = _factory_layer->cell_size();
		_cursor->setScale(cs / _cursor->getContentSize().width);
		_cursor->setAnchorPoint(Vec2(0.5, 0.5));
		_map->addChild(_cursor);

		//set_state(UIS_BROWSING);

		_main_menu->set_selected_btn(_main_menu->get_btn(0, 0));
		_industry_browser->set_selected_btn(_industry_browser->get_btn(0, 0));

		set_state(UIS_FACTORY);
		info.action = UIA_FACT_CREATE;

		return true;
	}

	Vec2 WorldUI::get_tile(Area * a)
	{
		return Vec2(a->x, _grid_size.height - a->y - 1);
	}

	void WorldUI::tick(float f)
	{
		static int k = 0;
		static bool working = false;
		if (!_paused && k % _speed == 0 && !working)
		{
#if 1
			if (_worker.joinable()) _worker.join();
			_worker = std::thread([]() {
				working = true;
				GUARD_LOCK_WORLD
				world->update();
				working = false;
			});
#else
			world->update();
#endif
		}

		//if (k % 10 == 0)
		{
			_color_layer->update(f);
		}

		if (_factory_view->isVisible())
		{
			//_factory_view->scheduleUpdate();
			//_factory_view->upda
		}

		++k;
	}

	void WorldUI::load_from_file(const std::string& config, const std::string& tmx_map)
	{
		TMXTiledMap* m = TMXTiledMap::create(tmx_map);
		_map = m;
		_grid_size = m->getMapSize();
		m->setAnchorPoint(Vec2(0.5, 0.5));
		m->setScale(1.5);
		this->addChild(_map);
		_map->setPosition(getContentSize() / 2);
		auto layer = m->getLayer("Background");
		//_map->setVisible(false);

		world = World::create_from_file(config, _grid_size.width, _grid_size.height);
		world->on_area_changed = [layer, this](Area* a) {
			int gid;
			switch (a->mil_state)
			{
			case MILS_EXPLORABLE:
				gid = 12;
				break;
			case MILS_EXPLORED:
				gid = a->ori_tile_gid;
				break;
			case MILS_UNEXPLORED:
			default:
				gid = 0;
				break;
			}

			RUNUI([layer, this, gid, a] {
				layer->setTileGID(gid, get_tile(a));
			});
		};

		vector<int> w(world->areas().size());
		for (int i = 0; i < _grid_size.width; ++i)
		{
			for (int j = 0; j < _grid_size.height; ++j)
			{
				Area* a = world->get_area(i, j);
				auto v = Vec2(i, _grid_size.height - j - 1);
				a->ori_tile_gid = layer->getTileGIDAt(v);
				
				//a->tile_gid = 0;
				//a->mil_state = MILS_UNEXPLORED;

				a->tile_gid = a->ori_tile_gid;
				a->mil_state = MILS_EXPLORED;
				
				layer->setTileGID(a->tile_gid, v);
				int& level = w[a->id];
				switch (a->ori_tile_gid - 1)
				{
				case 0:
				case 1:
				case 2:
				case 8:
				case 9:
					level = 500;
					a->type = AT_SEA;
					break;
				// mountain
				case 16:
				case 17:
				case 18:
				case 24:
				case 25:
					level = 100;
					a->type = AT_MOUNTAIN;
					break;
				default:
					level = 1;
					a->type = AT_PLAIN;
					break;
				}
			}
		}

		auto a = world->get_area(12, 10);
		world->set_explored(a);
		world->generate_resources();

		for (Road* r : world->roads())
		{
			//r->base_cost = (r->a->tile_gid + r->b->tile_gid) / 2;
			r->base_cost = (w[r->a->id] + w[r->b->id]) / 2;
			if (r->dir % 2 == 1) r->base_cost *= 1.414;
		}

		world->update_roads();


		_color_layer = ColorMapLayer::create(info);
		Node* v = _color_layer;
		//v->setVisible(true);
		//v->setAnchorPoint(Vec2(0, 0));
		//v->setPosition(Vec2(0, 0));
		v->setContentSize(_map->getContentSize());
		views.push_back(v);
		_map->addChild(v);


		// ---------------------- TEST ----------------------

		//int n = 3;
		//int x = 10, y = 10;
		//for (int i = -n; i <= n; ++i)
		//{
		//	for (int j = -n; j <= n; ++j)
		//	{
		//		Area* a = world->get_area(x + i+j, y + i-j);
		//		a->road_level = 1;
		//	}
		//}
		//for (int i = -n + 1; i <= n; ++i)
		//{
		//	for (int j = -n + 1; j <= n; ++j)
		//	{
		//		Area* a = world->get_area(x - 1 + i + j, y + i - j);
		//		a->road_level = 1;
		//	}
		//}
		//for (int i = -n; i <= 2 * n; ++i)
		//{
		//	for (int j = -n; j <= n; ++j)
		//	{
		//		Area* a = world->get_area(x + i + n, y + j);
		//		a->road_level = 1;
		//	}
		//}
		//for (int i = -n * 2; i <= 2 * n; ++i)
		//{
		//	Area* a = world->get_area(x + i, y + i);
		//	a->road_level = 3;
		//}
		//_road_layer->update_roads();





		v = _factory_layer = g_factory_layer = FactoryMapLayer::create();
		//_factory_layer->create_sprites_from_model();
		v->setContentSize(_map->getContentSize()); // without this, find_child doesn't find _factory_layer
		views.push_back(v);
		_map->addChild(v);


		_road_layer = RoadLayer::create();
		_road_layer->setAnchorPoint(Vec2(0, 0));
		_road_layer->setPosition(Vec2(0, 0));
		_road_layer->setContentSize(_map->getContentSize());
		_map->addChild(_road_layer);


		v = _map_labels = Node::create();
		_map->addChild(v);


		for (auto& a : world->areas())
		{
			if (a->has_factory)
			{
				auto v = world->find_factories(a);
				_factory_layer->create_sprite(v[0]);
			}
			else if (a->industry)
			{
				_factory_layer->create_sprite(nullptr, a->industry, a);
			}
		}
	}

	void WorldUI::menuCloseCallback(Ref* sender)
	{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
		MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.", "Alert");
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
		p = _map->convertToNodeSpace(p);
		_mouse_down_pos = p;

		Area* a = _factory_layer->get_area(p);
		if (!a->is_explored()) return true;

		if (_state == UIS_ROAD_AREA || _state == UIS_ROAD_ROUTE)
		{

		}
		else if (_state == UIS_BROWSING || _state == UIS_PRODUCT || _state == UIS_FACTORY)
		{
			auto v = world->find_factories(a);
			Factory* f = v.size() > 0 ? v[0] : nullptr;
			_factory_view->set_factory(f);
			_factory_view->setVisible(true);
			show_routes(f);
			if (f)
			{
				Industry* s = f->industry;
				_industry_view->set_industry(s);
				this->info.industry = s;
				Product* p = s->get_product();
				if (p)
				{
					info.product = world->get_products()[p->id];
				}
				set_state(UIS_FACTORY);
			}
			else
			{
				update_ui();
			}
		}

		return true;
	}

	void WorldUI::onTouchEnded(Touch* touch, Event  *event)
	{
		if (event->isStopped()) return;

		auto p = touch->getLocation();
		p = _map->convertToNodeSpace(p);
		Area* a = _factory_layer->get_area(p);

		if (try_start_exploration(a)) return;
		if (!a->is_explored()) return;

		auto& ml = a->mil_level;

		switch (info.action)
		{
		case UIA_FACT_CREATE:
			if (!_drag_start)
			{
				Industry* s = info.industry;
				//Industry* s = a->industry;
				if (s && a->type != AT_SEA)
				{
					_factory_layer->update_or_create_factory(a, s);
				}
			}
			break;
		case UIA_ROAD_PLUS:
			_road_layer->add_road(a);
			break;
		case UIA_ROAD_MINUS:
			_road_layer->remove_road(a);
			break;
		case UIA_MIL_PLUS:
			if (ml < 3) ++ml;
			break;
		case UIA_MIL_MINUS:
			if (ml > 0) --ml;
			break;
		case UIA_ROAD_ROUTE:
			_road_layer->finish_route();
			break;
		case UIA_EXPLORE:
			// _road_layer->finish_route();
			break;
		default:
			break;
		}
	}

	void WorldUI::onTouchMoved(Touch* touch, Event  *event)
	{
		Vec2 p = touch->getLocation();
		Vec2 d(0, 0);
		p = _map->convertToNodeSpace(p);
		Area* a = _factory_layer->get_area(p);

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
				_drag_start_area = a;
			}
		}
		
		if (_drag_start)
		{
			if (try_start_exploration(a)) return;
			

			if (info.action == UIA_ROAD_PLUS || info.action == UIA_ROAD_MINUS)
			{
				static Area* last_area = NULL;
				if (last_area == a) return;
				if (!is_inside_cell(p, a)) return;
				last_area = a;
			}

			switch (info.action)
			{
			case UIA_ROAD_PLUS:
				if (!a->is_explored()) return;
				_road_layer->add_road(a);
				break;
			case UIA_ROAD_MINUS:
				if (!a->is_explored()) return;
				_road_layer->remove_road(a);
				break;
			case UIA_ROAD_ROUTE:
				if (!a->is_explored()) return;
				if (a != _drag_start_area)
				{
					auto route = world->create_route(_drag_start_area, a);
					_road_layer->add_route(route, 3);
					delete route;
				}
				break;
			default:
				_map->setPosition(_map->getPosition() + d);
				break;
			}
		}
	}

	void WorldUI::onMouseMove(Event *event)
	{
		EventMouse* mouseEvent = dynamic_cast<EventMouse*>(event);
		update_popup(mouseEvent->getLocationInView());
	}

	bool simciv::WorldUI::try_start_exploration(Area * a)
	{
		if (a->mil_state == MILS_EXPLORABLE)
		{
			auto f = _factory_layer->create_factory(a, world->_explorer);
			if (f)
			{
				f->on_state_changed = [](Factory* f, FactoryState olds, FactoryState news) {
					if (news == FS_RUN)
					{
						RUNUI([f] {
							GUARD_LOCK_WORLD
								world->set_explored(f->area);
							g_factory_layer->delete_factory(f);
						});
					}
				};
			}
			return true;
		}

		return false;
	}

	bool WorldUI::is_inside_cell(Vec2 & p, Area* a)
	{
		if (!a)
		{
			a = _factory_layer->get_area(p);
		}
		if (!a) return false;
		auto q = _factory_layer->get_point(a);
		return (q - p).length() < 15;
	}

	RadioMenu* WorldUI::create_left_menu()
	{
		RadioMenu* result = RadioMenu::create();
		result->set_toggle(true);

		result->add_row();
		auto btn = MenuButton::create("res/img/icons/info.png");
		result->add_radio_button(btn);

		result->add_row();
		btn = MenuButton::create("res/img/icons/build.png");
		result->add_radio_button(btn);

		result->add_row();
		btn = MenuButton::create("res/road.png");
		result->add_radio_button(btn);

		result->add_row();
		btn = MenuButton::create("res/sword.png");
		result->add_radio_button(btn);

		result->set_on_changed([this](MenuButton* btn) {
			if (!btn)
			{
				this->set_state(UIS_NONE);
				this->info.action = UIA_NONE;
				return;
			}

			switch (btn->getTag())
			{
			case 0:
				this->set_state(UIS_PRODUCT);
				break;
			case 1:
				this->set_state(UIS_FACTORY);
				this->info.action = UIA_FACT_CREATE;
				break;
			case 2:
				this->set_state(UIS_ROAD_AREA);
				this->info.action = UIA_ROAD_PLUS;
				break;
			case 3:
				this->set_state(UIS_MILITARY);
				this->info.action = UIA_MIL_PLUS;
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

		auto f = [this](string s) {
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

	RadioMenu* WorldUI::create_industry_browser()
	{
		RadioMenu* result = RadioMenu::create();

		auto& industry = world->get_industries();
		string group = "";
		for (auto i: industry)
		{
			if (i->is_infra()) continue;
			if (i->group != group)
			{
				group = i->group;
				result->add_row();
			}
			auto btn = MenuButton::create(i->icon_file);
			btn->setUserData(i);
			result->add_radio_button(btn);
		}

		result->set_on_changed([this](MenuButton* btn) {
			if (btn)
			{
				Industry* s = (Industry*)btn->getUserData();
				this->info.industry = s;
				_industry_view->set_industry(s);
				set_state(_state);
			}
		});
		return result;
	}

	RadioMenu* WorldUI::create_products_browser()
	{
		RadioMenu* result = RadioMenu::create();
		auto& products = world->get_products();
		int i = 0;
		string group;
		for (auto product : products)
		{
			if (product->group != group)
			{
				group = product->group;
				result->add_row();
			}
			auto btn = MenuButton::create(product->icon_file);
			btn->setUserData(product);
			result->add_radio_button(btn);
		}

		result->set_selected_btn(0);
		result->set_on_changed([this](MenuButton* btn) {
			info.product = (Product*)btn->getUserData();
			set_state(_state);
		});
		return result;
	}

	RadioMenu * WorldUI::create_roads_menu()
	{
		RadioMenu* result = RadioMenu::create();
		result->add_row();
		auto area_plus = MenuButton::create("res/road_plus.png");
		result->add_radio_button(area_plus);
		auto area_minus = MenuButton::create("res/road_minus.png");
		result->add_radio_button(area_minus);

		result->add_row();

		auto route = MenuButton::create("res/route.png");
		result->add_radio_button(route);

		result->set_selected_btn(0);
		result->set_on_changed([=](MenuButton* btn) {
			if (btn == area_plus)
			{
				set_state(UIS_ROAD_AREA);
				info.action = UIA_ROAD_PLUS;
			}
			if (btn == area_minus)
			{
				set_state(UIS_ROAD_AREA);
				info.action = UIA_ROAD_MINUS;
			}
			else if (btn == route)
			{
				set_state(UIS_ROAD_ROUTE);
			}
		});
		return result;
	}

	RadioMenu * WorldUI::create_military_menu()
	{
		RadioMenu* result = RadioMenu::create();
		result->add_row();

		auto plus = MenuButton::create("res/sword_plus.png");
		result->add_radio_button(plus);

		auto minus = MenuButton::create("res/sword_minus.png");
		result->add_radio_button(minus);

		result->add_row();

		auto explore = MenuButton::create("res/explore.png");
		result->add_radio_button(explore);

		result->set_selected_btn(explore);
		result->set_on_changed([=](MenuButton* btn) {
			set_state(UIS_MILITARY);
			if (btn == explore)
			{
				info.action = UIA_EXPLORE;
			}
			else if (btn == plus)
			{
				info.action = UIA_MIL_PLUS;
			}
			else if (btn == minus)
			{
				info.action = UIA_MIL_MINUS;
			}
		});
		return result;
	}

	void WorldUI::doLayout()
	{
		Layout::doLayout();
		auto var = getContentSize();
		auto s = Director::getInstance()->getWinSize();
		int h = var.height;

		int m = 20;
		_main_menu->setPosition(Vec2(m, h - m));
		_industry_browser->setPosition(Vec2(m + 64 + 10, h - m));
		_products_browser->setPosition(Vec2(m + 64 + 10, h - m));
		_roads_menu->setPosition(Vec2(m + 64 + 10, h - m));
		_military_menu->setPosition(Vec2(m + 64 + 10, h - m));

		//_economy_view->setPosition(Vec2(m + 64 + 200, h - m));
		_economy_view->setPosition(Vec2(var.width / 2, h - m - 50));

		_factory_layers_options->setPosition(Vec2(m + 64 + 600, h - m));
		_color_layers_options->setPosition(Vec2(m + 64 + 600, h - m));
		

		_industry_view->setPosition(Vec2(var.width, h));
		_military_view->setPosition(Vec2(var.width, h));

		auto r = _industry_view->getBoundingBox();
		_factory_view->setPosition(Vec2(r.getMaxX(), r.getMinY()));
		_play_panel->setPosition(Vec2(m, m));
	}

	void WorldUI::create_option_panels()
	{
		_factory_layers_options = Panel::create();
		_factory_layers_options->setContentSize(Size(280, 40));

		defvec(vec1, "Res.", "Profit", "Cost")
		auto rb = RadioBox::create(vec1, 30, 0);
		rb->changed = [this](int id) {
			switch (id)
			{
			case 0:
				info.mode = MM_SPECIES_RESOURCES;
				break;
			case 1:
				info.mode = MM_PROFIT;
				break;
			case 2:
				info.mode = MM_BUILD_COST;
				break;
			default:
				break;
			}
		};

		_on_state_factory = [=]() {
			rb->update();
		};

		_factory_layers_options->addChild(rb);
		_factory_layers_options->setAnchorPoint(Vec2(0, 1));
		this->addChild(_factory_layers_options);


		_color_layers_options = Panel::create();
		_color_layers_options->setContentSize(Size(300, 40));
		info.mode = MM_PRICE_SELL;
		defvec(vec2, "Sell", "Buy", "Res.")
		rb = RadioBox::create(vec2, 30, 0);
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
		_color_layers_options->addChild(rb);
		_color_layers_options->setAnchorPoint(Vec2(0, 1));

		_on_state_product = [=]() {
			rb->update();
		};

		this->addChild(_color_layers_options);
	}

	void WorldUI::set_state(UIState state)
	{
		_state = state;
		update_ui();
	}

	void WorldUI::update_ui()
	{
		bool factories = _state == UIS_FACTORY;
		bool products = _state == UIS_PRODUCT;
		bool roads = _state == UIS_ROAD_AREA;
		bool military = _state == UIS_MILITARY;
		_industry_browser->setVisible(factories);
		_industry_view->setVisible(factories);
		_products_browser->setVisible(products);
		_color_layers_options->setVisible(products);
		_factory_layers_options->setVisible(factories);
		_factory_view->setVisible(factories && _factory_view->get_factory());
		_roads_menu->setVisible(roads);
		_military_menu->setVisible(military);
		_military_view->setVisible(military);

		_color_layer->setVisible(true);

		switch (_state)
		{
		case simciv::UIS_FACTORY:
		{
			if (_popup) _popup->removeFromParent();
			_popup = FactoryPopup::create();
			this->addChild(_popup);
			_on_state_factory();
		}
		break;
		case simciv::UIS_PRODUCT:
		{
			_on_state_product();
		}
		break;
		case simciv::UIS_ROAD_AREA:
		{
			info.mode = MM_ROAD;
		}
		break;
		case simciv::UIS_MILITARY:
		{
			info.mode = MM_MIL_LEVEL;
		}
		break;
		case simciv::UIS_NONE:
		{
			info.mode = MM_NONE;
		}
		break;
		default:
			break;
		}
	}

	void WorldUI::find_child(const cocos2d::Node* n, const Vec2& wp, cocos2d::Node*& child, int& z_order)
	{
		auto q = n->convertToNodeSpace(wp);
		for (auto c : n->getChildren())
		{
			if (_popup && c == _popup) continue;
			if (_cursor && c == _cursor) continue;

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

	void WorldUI::show_routes(Factory * f)
	{
		static vector<Node*> fos;
		for (auto a : fos) a->setVisible(true);
			
		_road_layer->clear_tmp_roads();
		_map_labels->removeAllChildren();

		if (f)
		{
			vector<Transport*> v; // in, out;
			world->get_transports(f->area, v, v);

			//typedef pair<ProductMap, ProductMap> partner_data;
			//map<Area*, partner_data> partners;
			//for (auto t : v)
			//{
			//	_road_layer->add_tmp_route(t->route, 1, t->seller->product->id);
			//	int prod_id = t->seller->product->id;

			//	auto b = t->seller->area;
			//	auto a = f->area;
			//	if (b == a)
			//	{
			//		b = t->buyer->area;
			//		partners[a].second[prod_id] = t->volume;
			//		partners[b].first[prod_id] = t->volume;
			//	}
			//	else
			//	{
			//		partners[a].first[prod_id] = t->volume;
			//		partners[b].second[prod_id] = t->volume;
			//	}
			//}

			//for (auto& p : partners)
			//{
			//	Area* a = p.first;
			//	partner_data& data = p.second;
			//	auto tv = TradePartnerPanel::create(data.first, data.second);
			//	tv->setPosition(_color_layer->get_point(a));
			//	_map_labels->addChild(tv);
			//}

			map<Area*, Products> partners;
			for (auto t : v)
			{
				if (t->smooth_volume < 0.01) continue;
				_road_layer->add_tmp_route(t->route, 1, t->seller->product->id);
				int prod_id = t->seller->product->id;

				auto b = t->seller->area;
				auto a = f->area;
				auto d = 10 * t->smooth_volume;
				if (b == a)
				{
					b = t->buyer->area;
					partners[a][prod_id] += d;
					partners[b][prod_id] -= d;
				}							
				else
				{
					partners[a][prod_id] -= d;
					partners[b][prod_id] += d;
				}
			}

			for (auto& p : partners)
			{
				Area* a = p.first;
				Products& ps = p.second;
				auto tv = AreaProductPanel::create(ps);
				tv->setPosition(_color_layer->get_point(a));
				_map_labels->addChild(tv);

				auto s = _factory_layer->get_sprite(a);
				if (s)
				{
					//for (auto& n : s->_nodes) n->setPosition(n->getPosition() + Vec2(20,20));
					auto c = s->_nodes[0];
					//c->setVisible(false);
					//fos.push_back(c);
				}
			}
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
		if (!_popup && !_cursor->isVisible()) return;

		auto q = _map->convertToNodeSpace(wp);
		Area* a = _color_layer->get_area(q);

		if (!_drag_start || _state == UIS_ROAD_ROUTE || is_inside_cell(q, a))
		{
			auto p = _factory_layer->get_point(a);
			_cursor->setPosition(p);
		}

		if (!_popup) return;

		cocos2d::Node* n = find_child(this, wp);
		if (n == _factory_layer)
		{
			if (info.industry)
			{
				_popup->setPosition(wp + Vec2(10, 10));
				((FactoryPopup*)_popup)->set_profit(Info::profit(a, info.industry));
				((FactoryPopup*)_popup)->set_cost(Info::build_cost(a, info.industry));
				_popup->setVisible(true);
			}
		}
		else
		{
			_popup->setVisible(false);
		}
	}


}