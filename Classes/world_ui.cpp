#include "world_ui.h"

#include <algorithm>

#include "ui/UICheckBox.h"
#include "ui/UILayout.h"
#include "ui/UIHBox.h"
#include "ui/UIVBox.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "base/ccTypes.h"

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

	WorldUI::WorldUI() : _menu_size(64, 64), view_mode(0), new_view_mode(0), _drag_start(false), _paused(false), _speed(10), _popup(NULL)
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

		load_from_tmx("simciv2.tmx");

		_main_menu = create_left_menu();
		this->addChild(_main_menu);

		_industry_browser = create_industry_browser();
		this->addChild(_industry_browser);

		_products_browser = create_products_browser();
		this->addChild(_products_browser);

		_industry_view = IndustryView::create();
		_industry_view->setAnchorPoint(Vec2(1, 1));
		this->addChild(_industry_view);
		_industry_view->set_industry(world.get_industries().at(0));

		create_color_layers_panel();
		create_factory_layers_panel();

		_factory_view = FactoryView::create();
		_factory_view->setAnchorPoint(Vec2(1, 1));
		_factory_view->setVisible(false);
		this->addChild(_factory_view);

		create_play_panel();

		_cursor = Sprite::create("cursor.png");
		double cs = _factory_layer->cell_size();
		_cursor->setScale(cs / _cursor->getContentSize().width);
		_cursor->setAnchorPoint(Vec2(0.5, 0.5));
		_map->addChild(_cursor);

		set_state(UIS_NONE);
	}

	void WorldUI::tick(float f)
	{
		//if (view_mode != new_view_mode)
		//{
		//	auto old_view = views[view_mode];
		//	old_view->setVisible(false);

		//	view_mode = new_view_mode;
		//	auto new_view = views[view_mode];
		//	new_view->setVisible(true);
		//}

		static int k = 0;
		if (!_paused && k % _speed == 0)
		{
			// world.update();
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

	void WorldUI::load_from_tmx(std::string tmx)
	{
		TMXTiledMap* m = TMXTiledMap::create(tmx);
		_map = m;
		auto size = m->getMapSize();
		m->setAnchorPoint(Vec2(0.5, 0.5));
		m->setScale(1.5);
		this->addChild(_map);

		world.create(size.width, size.height, product_count);

		_road_layer = RoadLayer::create();
		_road_layer->setAnchorPoint(Vec2(0, 0));
		_road_layer->setPosition(Vec2(0, 0));
		_road_layer->setContentSize(_map->getContentSize());
		_map->addChild(_road_layer);
		// ---------------------- TEST ----------------------
		int l = 3;
		auto a1 = world.get_area(10, 10);
		auto a2 = world.get_area(11, 11);
		auto a3 = world.get_area(12, 12);
		auto a4 = world.get_area(13, 12);
		auto a5 = world.get_area(12, 11);
		auto a6 = world.get_area(13, 10);
		_road_layer->add_road(a1, a2, a3, l);
		_road_layer->add_road(a2, a3, a4, l);
		_road_layer->add_road(a3, a4, a5, l);
		_road_layer->add_road(a4, a5, a6, l);


		_color_layer = ColorMapLayer::create(info);
		Node* v = _color_layer;
		//v->setVisible(true);
		//v->setAnchorPoint(Vec2(0, 0));
		//v->setPosition(Vec2(0, 0));
		//v->setContentSize(_map->getContentSize());
		views.push_back(v);
		_map->addChild(v);


		v = _factory_layer = FactoryMapLayer::create();
		_factory_layer->create_sprites_from_model();
		//v->setAnchorPoint(Vec2(0, 0));
		//v->setPosition(Vec2(0, 0));
		//v->setContentSize(_map->getContentSize());
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
		_mouse_down_pos = p;
		p = _map->convertToNodeSpace(p);

		Area* a = _factory_layer->get_area(p);
		Factory* f = world.find_factory(a);
		if (f)
		{
			_factory_view->set_factory(f);
			_factory_view->setVisible(true);
			Industry* s = &f->industry;
			_industry_view->set_industry(s);
			_industry_view->setVisible(true);
			this->info.industry = s;
			info.product = world.get_products()[s->id]; //s->color + s->level * level_count;
			set_state(UIS_FACTORY);
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

		auto p = touch->getLocation();
		p = _map->convertToNodeSpace(p);
		Area* a = _factory_layer->get_area(p);

		switch (_state)
		{
		case simciv::UIS_NONE:
			break;
		case simciv::UIS_FACTORY:
		{
			Factory* f = world.find_factory(a);
			if (f)
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
		case simciv::UIS_PRODUCT:
			break;
		case simciv::UIS_ROAD:
			_road_layer->add_road(a);
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

		p = _map->convertToNodeSpace(p);
		Area* a = _factory_layer->get_area(p);
		
		if (_drag_start)
		{
			if (_state == UIS_ROAD)
			{
				static Area* last_area = NULL;
				if (last_area == a) return;

				if (!is_inside_cell(p, a)) return;

				last_area = a;
				_road_layer->add_road(a);
			}
			else
			{
				_map->setPosition(_map->getPosition() + d);
			}
		}
	}

	void WorldUI::onMouseMove(Event *event)
	{
		EventMouse* mouseEvent = dynamic_cast<EventMouse*>(event);
		update_popup(mouseEvent->getLocationInView());
	}

	bool WorldUI::is_inside_cell(Vec2 & p, Area* a)
	{
		if (!a)
		{
			a = _factory_layer->get_area(p);
		}
		auto q = _factory_layer->get_point(a);
		return (q - p).length() < 15;
	}

	RadioMenu* WorldUI::create_left_menu()
	{
		RadioMenu* result = RadioMenu::create();
		result->set_toggle(true);

		result->add_row();
		auto btn = MenuButton::create(get_product_texture(0));
		result->add_radio_button(btn);

		result->add_row();
		btn = MenuButton::create(get_product_texture(0));
		result->add_radio_button(btn);

		result->add_row();
		btn = MenuButton::create("res/road.png");
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
				this->set_state(UIS_FACTORY);
				break;
			case 1:
				this->set_state(UIS_PRODUCT);
				break;
			case 2:
				this->set_state(UIS_ROAD);
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

		auto& industry = world.get_industries();
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
			_industry_view->set_industry(s);
			set_state(_state);
		});
		return result;
	}

	RadioMenu* WorldUI::create_products_browser()
	{
		RadioMenu* result = RadioMenu::create();
		auto& products = world.get_products();
		int i = 0;
		for (auto product : products)
		{
			if (i++ % 3 == 0) result->add_row();
			auto btn = MenuButton::create(product->icon_file);
			btn->setUserData(product);
			result->add_radio_button(btn);
		}

		result->set_selected_btn(0);
		result->set_on_changed([this](MenuButton* btn) {
			info.product = (Product*)btn->getUserData(); // world.get_products()[btn->getTag()];
			set_state(_state);
		});
		return result;
	}

	void WorldUI::create_color_layers_panel()
	{
		auto s = Size(20, 20);
		LinearLayoutParameter* p = LinearLayoutParameter::create();
		p->setGravity(LinearLayoutParameter::LinearGravity::TOP);
		p->setMargin(Margin(2, 2, 2, 2));
		LinearLayoutParameter* q = LinearLayoutParameter::create();
		q->setGravity(LinearLayoutParameter::LinearGravity::LEFT);

		_color_layers_panel = VBox::create();
		_color_layers_panel->setContentSize(Size(300, 80));
		_color_layers_panel->setBackGroundColor(def_bck_color3B);
		_color_layers_panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);

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
		_color_layers_panel->addChild(rb);
		_color_layers_panel->setAnchorPoint(Vec2(1, 0));

		_on_state_product = [=]() {
			rb->update();
		};

		this->addChild(_color_layers_panel);
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

		_on_state_factory = [=]() {
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
		_industry_browser->setPosition(Vec2(m + 64 + 10, h - m));
		_products_browser->setPosition(Vec2(m + 64 + 10, h - m));
		_industry_view->setPosition(Vec2(var.width, h));

		auto r = _industry_view->getBoundingBox();
		_factory_view->setPosition(Vec2(r.getMaxX(), r.getMinY()));
		_play_panel->setPosition(Vec2(m, m));

		_color_layers_panel->setPosition(Vec2(var.width, 0));
		_factory_layers_panel->setPosition(Vec2(var.width, 0));
	}

	void WorldUI::set_state(UIState state)
	{
		_state = state;
		bool factories = _state == UIS_FACTORY;
		bool products = _state == UIS_PRODUCT;
		_industry_browser->setVisible(factories);
		_industry_view->setVisible(factories);
		_products_browser->setVisible(products);
		_color_layers_panel->setVisible(products);
		_factory_layers_panel->setVisible(factories);
		_color_layer->setVisible(true);

		switch (state)
		{
		case simciv::UIS_FACTORY:
		{
			if (_popup) _popup->removeFromParent();
			_popup = new FactoryPopup();
			this->addChild(_popup);
			_on_state_factory();
		}
		break;
		case simciv::UIS_PRODUCT:
		{
			_on_state_product();
		}
		break;
		case simciv::UIS_ROAD:
		{

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
		if (!_popup && !_cursor->isVisible()) return;

		auto q = _map->convertToNodeSpace(wp);
		Area* a = _color_layer->get_area(q);

		if (is_inside_cell(q, a))
		{
			auto p = _factory_layer->get_point(a);
			_cursor->setPosition(p);
		}

		if (!_popup) return;

		auto n = find_child(this, wp);
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

	//void RoadView::draw(Renderer * renderer, const Mat4 & transform, uint32_t flags)
	//{
	//	_customCommand.init(_globalZOrder);
	//	_customCommand.func = CC_CALLBACK_0(RoadView::onDraw, this, transform, flags);
	//	renderer->addCommand(&_customCommand);
	//}

	void RoadView::onDraw(const Mat4 & transform, uint32_t flags)
	{
		Director* director = Director::getInstance();
		director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
		director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
		CHECK_GL_ERROR_DEBUG();

		//	0|		1/		2-		3\ 
		const int xx[4] = { 0, 1, 1, 1 };
		const int yy[4] = { 1, -1, 0, 1 };
		const double lw[5] = { 2, 3, 4, 5, 6 };
		const int a = 16;
		int x = xx[direction], y = yy[direction];

		DrawPrimitives::setDrawColor4F(0, 0, 0, 1);
		// DrawPrimitives::setPointSize(10.0f); // 
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(lw[level - 1]);
		Vec2 e(a - a*x, a - a*y);
		Vec2 f(a + a*x, a + a*y);
		// DrawPrimitives::drawLine(e, f);
		DrawPrimitives::drawCubicBezier(e, e + Vec2(0, a / 2), e + Vec2(a / 2, a), e + Vec2(a, a), 100);

		director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	}

	// std::vector<cocos2d::SpriteFrame*> RoadView::frames;
	cocos2d::SpriteFrame* RoadView::frames[5][9][9];

	//void RoadView::load(std::string file)
	//{
	//	int m = 1;
	//	int s = 1;
	//	int w = 50;
	//	for (int i = 0; i < 36; ++i)
	//	{
	//		int row = i / 6;
	//		int col = i % 6;
	//		Sprite* sprite = Sprite::create(file, Rect(m + col * (w + s), m + row * (w + s), w, w));
	//		sprites.push_back(sprite);
	//	}
	//}

	cocos2d::SpriteBatchNode * RoadView::create_batch_node(std::string file)
	{
		SpriteBatchNode* result = SpriteBatchNode::create(file);
		int m = 1;
		int s = 1;
		int w = 50;
		const int cols = 6;
		int k = 0;

		for (int level = 0; level < 5; ++level)
		{
			for (int i = 0; i < 9; ++i)
			{
				for (int j = i + 1; j < 9; ++j)
				{
					Road r;
					int row = k / cols;
					int col = k % cols;
					Rect rect(m + col * (w + s), m + row * (w + s), w, w);
					SpriteFrame* f = SpriteFrame::create(file, rect);
					f->retain();
					frames[level][i][j] = f;
					++k;
				}
			}
		}
		return result;
	}

	RoadView * RoadView::create(int level, Vec2 & a)
	{
		return create(level, a, Vec2(0, 0));
	}

	RoadView * RoadView::create(int level, const int & a)
	{
		return create(level, a, 8);
	}

#define dir(a, ad, i, j, d) if (a.x == i && a.y == - j) ad = d; else
#define dir2(a, ad) \
	dir(a, ad, -1, 0, 0) \
	dir(a, ad, -1, -1, 1) \
	dir(a, ad, 0, -1, 2) \
	dir(a, ad, 1, -1, 3) \
	dir(a, ad, 1, 0, 4) \
	dir(a, ad, 1, 1, 5) \
	dir(a, ad, 0, 1, 6) \
	dir(a, ad, -1, 1, 7) \
	dir(a, ad, 0, 0, 8);

	RoadView * RoadView::create(int level, Vec2 & a, Vec2 & b)
	{
		int ad, bd;
		dir2(a, ad)
		dir2(b, bd)
		return create(level, ad, bd);
	}

	RoadView * RoadView::create(int level, const int & ad, const int & bd)
	{
		SpriteFrame* f = ad < bd ? frames[level - 1][ad][bd] : frames[level - 1][bd][ad];
		RoadView* s = RoadView::create();
		s->level = level;
		s->setSpriteFrame(f);
		return s;
	}

	int RoadView::get_dir(Vec2 & a)
	{
		int d;
		dir2(a, d)
		return d;
	}

}