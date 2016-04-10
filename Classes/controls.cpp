#include "stdafx.h"

#include "controls.h"
#include "world_ui.h"

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;

//const string def_font = "verdana";
//const int def_font_size = 12;

Layout* labelled_cb(std::string text, bool checked, CheckBox::ccCheckBoxCallback cb)
{
	auto l = HBox::create();
	auto p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	CheckBox* chb = CheckBox::create("ui/check_box_normal.png",
		"ui/check_box_normal_press.png",
		"ui/check_box_active.png",
		"ui/check_box_normal_disable.png",
		"ui/check_box_active_disable.png");
	chb->setSelected(checked);
	chb->addEventListener(cb);
	chb->setLayoutParameter(p);
	l->addChild(chb);

	auto label = Text::create();
	label->setString(text);
	label->setFontSize(18);
	label->setLayoutParameter(p);
	l->addChild(label);
	l->requestDoLayout();
	auto height = std::max(chb->getContentSize().height, label->getContentSize().height);
	l->setContentSize(Size(100, height));
	label->setTouchEnabled(true);
	label->addTouchEventListener([chb, cb](Ref*, Widget::TouchEventType type)
	{
		if (type == Widget::TouchEventType::ENDED)
		{
			chb->setSelected(!chb->isSelected());
			cb(NULL, chb->isSelected() ? CheckBox::EventType::SELECTED : CheckBox::EventType::UNSELECTED);
		}
	});

	return l;
}

std::string get_product_texture(int id)
{
	return world.get_products()[id]->icon_file;
}

Layout* combobox(const std::string* labels)
{
	return NULL;
}

RadioBox::RadioBox(std::vector<std::string> labels, int hh, int marginy) : hh(hh), marginy(marginy), selected(0)
{
	int k = 0;
	for (auto l: labels)
	{
		auto cb = [this, k](Ref* pSender, CheckBox::EventType type) { setSelected(k); };
		Widget* rb;
		if (l.substr(0, 1) == "_")
		{
			rb = image_radio(l.substr(1, l.length() - 1), cb);
		}
		else
		{
			rb = labelled_radio(l, cb);
		}
		++k;
		addChild(rb);
		items.push_back(rb);
	}
	setSelected(0);
}

void RadioBox::setSelected(int i)
{
	int l = 0;
	for (Widget* item: items)
	{
		((CheckBox*)item->getChildren().at(0))->setSelected(i == l++);
	}
	// *(this->data) = i;
	selected = i;
	if (changed)
	{
		changed(i);
	}
}

int RadioBox::getSelected()
{
	return selected;
}

void RadioBox::update()
{
	changed(selected);
}

RadioBox* RadioBox::create(std::vector<std::string> labels, int hh, int marginy)
{
	RadioBox* widget = new RadioBox(labels, hh, marginy);
	if (widget && widget->init())
	{
		widget->autorelease();
		widget->setContentSize(Size(100, hh + marginy));
		return widget;
	}
	else
	{
		CC_SAFE_DELETE(widget);
		return nullptr;
	}
}

#define RBON "ui/Radio_button_on.png"
#define RBOFF "ui/Radio_button_off.png"

Layout* RadioBox::labelled_radio(std::string text, CheckBox::ccCheckBoxCallback cb)
{
	auto l = HBox::create();
	auto p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	CheckBox* chb = CheckBox::create(RBOFF, RBOFF, RBON, RBOFF, RBON);
	chb->setSelected(false);
	chb->addEventListener(cb);
	chb->setLayoutParameter(p);
	l->addChild(chb);
	
	auto label = Text::create();
	label->setString(text);
	label->setFontSize(18);
	label->setLayoutParameter(p);
	label->setTouchEnabled(true);
	label->addTouchEventListener([chb, cb](Ref*,Widget::TouchEventType type)
	{ 
		if (type == Widget::TouchEventType::ENDED)
		{
			chb->setSelected(true);
			cb(NULL, CheckBox::EventType::SELECTED); 
		}
	});

	l->addChild(label);
	l->requestDoLayout();
	auto height = std::max(chb->getContentSize().height, label->getContentSize().height);
	l->setContentSize(Size(100, height));

	LinearLayoutParameter* pp = LinearLayoutParameter::create();
	pp->setGravity(LinearLayoutParameter::LinearGravity::TOP);
	pp->setMargin(Margin(2, marginy, 2, 2));

	l->setLayoutParameter(pp);

	return l;
}

Layout* RadioBox::image_radio(string img, CheckBox::ccCheckBoxCallback cb)
{
	auto l = HBox::create();
	auto p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	CheckBox* chb = CheckBox::create(RBOFF, RBOFF, RBON, RBOFF, RBON);
	chb->setSelected(false);
	chb->addEventListener(cb);
	chb->setLayoutParameter(p);
	l->addChild(chb);
	
	auto image = Widget::create();
	auto s = Sprite::create(img);
	s->setScale(hh / s->getContentSize().height);
	s->setPosition(hh/2, hh/2);
	image->addChild(s);
	image->setLayoutParameter(p);
	image->setContentSize(Size(hh, hh));
	image->setTouchEnabled(true);
	image->addTouchEventListener([chb, cb](Ref*,Widget::TouchEventType type)
	{ 
		if (type == Widget::TouchEventType::ENDED)
		{
			chb->setSelected(true);
			cb(NULL, CheckBox::EventType::SELECTED); 
		}
	});
	l->addChild(image);
	//l->requestDoLayout();
	// auto height = std::max(chb->getSize().height, label->getSize().height);
	l->setContentSize(Size(100, hh));

	LinearLayoutParameter* pp = LinearLayoutParameter::create();
	pp->setGravity(LinearLayoutParameter::LinearGravity::TOP);
	pp->setMargin(Margin(2, marginy, 2, 2));

	l->setLayoutParameter(pp);

	return l;
}

MenuButton::MenuButton() : _is_toggle(false)
{
	
}

MenuButton* MenuButton::create(std::string texture)
{
	//auto btn = MenuButton::create(Size(64, 64), texture, "Circle_Orange2.png", "Circle_Orange2_sel.png", "Circle_Blue2.png");
	//auto btn = MenuButton::create(Size(64, 64), texture, "circle_blue3.png", "circle_blue3.png", "circle_blue3.png");
	auto btn = MenuButton::create(Size(54, 54), texture, "circle_blue3.png", "circle_blue3.png", "circle_blue3.png");
	btn->setAnchorPoint(Vec2(0, 1));
	btn->ignoreContentAdaptWithSize(false);
	return btn;
}

MenuButton* MenuButton::create(Size size, std::string image, std::string bck_normal, std::string bck_selected, std::string bck_disabled)
{
	MenuButton* result = MenuButton::create();
	if (result)
	{
		result->setContentSize(size);
		result->_bck_normal = "img/" + bck_normal;
		result->_bck_selected = "img/" + bck_selected;
		result->_bck_disabled = "img/" + bck_disabled;
		
		//result->togg
		auto b = ImageView::create(result->_bck_normal);
		
		result->_bck = b;
		b->ignoreContentAdaptWithSize(false);
		b->setContentSize(size);
		b->setPosition(size / 2);
		result->addChild(b);

		result->_img = ui::ImageView::create();
		result->_img->ignoreContentAdaptWithSize(false);
		int m = 25;
		result->_img->setContentSize(Size(size.width - m, size.height - m));
		result->_img->loadTexture(image);
		result->_img->setPosition(size / 2);
		result->addChild(result->_img);
	}
	return result;
}

void MenuButton::onPressStateChangedToNormal()
{
	Button::onPressStateChangedToNormal();
	_bck->loadTexture(_bck_normal);
}

void MenuButton::onPressStateChangedToPressed()
{
	Button::onPressStateChangedToPressed();
	_bck->loadTexture(_bck_selected);
}

void MenuButton::onPressStateChangedToDisabled()
{
	Button::onPressStateChangedToDisabled();
	_bck->loadTexture(_bck_disabled);
}

void MenuButton::onTouchEnded(Touch *touch, Event *unusedEvent)
{
	if (_is_toggle)
	{
		// Skip change back to normal
	}
	else
	{
		Button::onTouchEnded(touch, unusedEvent);
	}
}

bool RadioMenu::init()
{
	if (!VBox::init()) return false;
	_selected = NULL;
	_space = 15;
	_toggle = false;
	_count = 0;
	return true;
}

void RadioMenu::set_selected_btn(MenuButton* btn)
{
	if (_selected)
	{
		_selected->set_selected(false);
	}
	_selected = btn;

	if (btn)
	{
		btn->set_selected(true);
		if (_on_changed)
		{
			_on_changed(btn);
		}
	}
	else
	{
		if (_on_changed)
		{
			_on_changed(NULL);
		}
	}
}

//void RadioMenu::set_selected_btn(int id)
//{
//	if (id > -1)
//	{
//		set_selected_btn((MenuButton*)getChildren().at(id));
//	}
//	else
//	{
//		set_selected_btn((MenuButton*)NULL);
//	}
//}

//int RadioMenu::find_btn(MenuButton* btn)
//{
//	int i = 0;
//	for (auto child : getChildren())
//	{
//		if (child == btn) return i;
//		++i;
//	}
//}

void RadioMenu::add_row()
{
	auto p = LinearLayoutParameter::create();
	int m = 3;
	p->setMargin(Margin(m, m, m, m));
	_row = HBox::create();
	_row->setLayoutParameter(p);
	addChild(_row);
}

void RadioMenu::add_radio_button(MenuButton* btn)
{
	btn->set_toggle(true);
	btn->addTouchEventListener(CC_CALLBACK_2(RadioMenu::on_btn_clicked, this));
	btn->setTag(_count++);
	auto p = LinearLayoutParameter::create();
	int m = 3;
	p->setMargin(Margin(m, m, m, m));
	btn->setLayoutParameter(p);
	_row->addChild(btn);
	auto s = _row->getContentSize();
	auto v = btn->getContentSize();
	_row->setContentSize(Size(s.width + v.width, v.height));
}

void RadioMenu::on_btn_clicked(Ref* btn, Widget::TouchEventType type)
{
	if (type == Widget::TouchEventType::BEGAN)
	{
		MenuButton* b = (MenuButton*)btn;
		if (_toggle && _selected == b)
		{
			set_selected_btn(NULL);
		}
		else
		{
			set_selected_btn(b);
		}
	}
}

bool Panel::init()
{
	if (!Layout::init()) return false;
	setBackGroundImageScale9Enabled(true);
	setBackGroundImage("img/button2.png");
	//setBackGroundImage("img/11949847621854578794pill-button-blue_benji_p_01.svg.med.png");
	return true;
}

Text* Panel::create_label(std::string text)
{
	auto label = ui::Text::create(text, "Arial", 12);
	label->ignoreContentAdaptWithSize(false);
	return label;
}

//bool FactoryPopup::init()
//{
//	if (!Popup::init()) return false;
//	setContentSize(Size(100, 100));
//	int w = 120;
//	int h = 100;
//	int rh = 30;
//	int m = 5;
//
//	VBox* v = VBox::create(Size(w - 2 * m, h - 2 * m));
//	v->setPosition(Vec2(m, m));
//	v->setAnchorPoint(Vec2(0, 0));
//	this->addChild(v);
//
//	auto f = [&](string str, double* d) {
//		HBox* hb = HBox::create(Size(w, rh));
//		v->addChild(hb);
//
//		auto text = ui::Text::create(str, def_font, def_font_size);
//		hb->addChild(text);
//		text->ignoreContentAdaptWithSize(false);
//		text->setContentSize(Size(w / 2 - m, rh));
//
//		Label* label = Label::create(d);
//		hb->addChild(label);
//		label->setContentSize(Size(w / 2 - m, rh));
//	};
//
//	f("profit: ", &_profit);
//	f("cost: ", &_cost);
//	return true;
//}

bool FactoryPopup::init()
{
	if (!Popup::init()) return false;
	auto row = table->create_row();
	row->addChild(Label::create("profit"));
	row->addChild(Label::create(&_profit));
	row = table->create_row();
	row->addChild(Label::create("cost"));
	row->addChild(Label::create(&_cost));
	return true;
}

bool IndustryView::init()
{
	if (!Panel::init()) return false;
	_production_view = VBox::create();
	_production_view->setAnchorPoint(Vec2(0, 1));
	addChild(_production_view);

	_icon = Node::create();
	addChild(_icon);

	_name_label = Label::create("anyad", LS_LARGE);
	_name_label->setAnchorPoint(Vec2(0, 1));
	//_name_label->setTextHorizontalAlignment(TextHAlignment::LEFT);
	addChild(_name_label);

	_build_cost = ProductStringView::create(30);
	addChild(_build_cost);

	_build_cost_label = Label::create("Build cost");
	_build_cost_label->setAnchorPoint(Vec2(0, 1));
	//_build_cost_label->setTextHorizontalAlignment(TextHAlignment::LEFT);
	addChild(_build_cost_label);
	
	setContentSize(Size(300, 500));
	return true;
}

void IndustryView::set_industry(Industry* industry)
{
	this->_industry = industry;
	if (industry)
	{
		string file = industry->icon_file;
		_icon->removeFromParent();
		_icon = Sprites::create(industry, Size(50, 50), true);
		_icon->setAnchorPoint(Vec2(0, 1));
		addChild(_icon);
		_name_label->setString(industry->display_name);
		_build_cost->set_map(industry->build_cost.total[0].input);


		_production_view->removeAllChildrenWithCleanup(true);

		for (auto& rule : industry->prod_rules)
		{
			Size ss = getContentSize();

			HBox* h = HBox::create();
			h->setContentSize(Size(ss.width, 30));
			auto left = ProductStringView::create(30);
			left->set_map(rule.input);
			left->setContentSize(Size(ss.width / 2, 30));
			auto right = ProductStringView::create(30);
			right->set_map(rule.output);

			h->addChild(left);
			h->addChild(right);

			_production_view->addChild(h);
		}
		auto s = getContentSize();
		setContentSize(s);
	}
}

void IndustryView::add_prod_row(Products& prod)
{

}

void IndustryView::setContentSize(const Size & var)
{
	if (var.width == 0) return;
	Panel::setContentSize(var);
	int m = 5;
	int y = var.height;
	_icon->setPosition(Vec2(m, var.height - m));
	_name_label->setPosition(Vec2(m + 50 + 10, var.height - m));

	y -= _icon->getContentSize().height + 30;

	_build_cost_label->setPosition(Vec2(20, y));
	
	//_build_cost_label->ignoreContentAdaptWithSize(false);
	//_build_cost_label->setContentSize(Size(200, 50));
	y -= 30;

	_build_cost->setPosition(Vec2(20, y));
	y -= 30;

	_production_view->setContentSize(Size(var.width, var.height - 100));
	_production_view->setPosition(Vec2(0, y));
}

bool FactoryView::init()
{
	if (Panel::init())
	{
		_producer_views = VBox::create();
		_producer_views->setAnchorPoint(Vec2(0, 1));
		_producer_views->setPosition(Vec2(0, 0));
		addChild(_producer_views);

		setContentSize(Size(300, 500));

		_money_txt = create_label("Money");
		addChild(_money_txt);

		_money_val = Label::create(NULL);
		_money_val->setSize(Size(100, 20));
		addChild(_money_val);

		scheduleUpdate();
		return true;
	}
	return false;
}

void FactoryView::set_factory(Factory* Factory)
{
	_factory = Factory;
	_producer_views->removeAllChildrenWithCleanup(true);
	
	if (!_factory) return;
	_money_val->data = &Factory->money;

	//auto h = ui::Layout::create();
	//auto size = Size(20, 20);
	//auto s = ui::ImageView::create("img/shapes/dollar.png");
	//s->ignoreContentAdaptWithSize(false);
	//s->setContentSize(size);
	//h->addChild(s);
	//s = ui::ImageView::create("img/shapes/storage.png");
	//s->ignoreContentAdaptWithSize(false);
	//s->setContentSize(size);
	//h->addChild(s);

	auto h = ui::HBox::create();
	_producer_views->addChild(h);
	int w = (getContentSize().width- 40) / 5;
	int hh = getContentSize().height;

	auto f = [w, h, hh](string s) {
		auto label = ui::Text::create(s, "Arial", 12);
		label->ignoreContentAdaptWithSize(false);
		label->setContentSize(Size(w, hh));
		//label->setLayoutParameter(po);
		h->addChild(label);
	};

	auto space = ui::Layout::create();
	space->ignoreContentAdaptWithSize(false);
	space->setContentSize(Size(40, 40));
	h->addChild(space);

	f("price");
	f("storage");
	f("vol");
	f("vol_out");
	f("free_vol");


	// po->setMargin(ui::Margin(5, 5, 5, 5));

	for (auto* p : Factory->buyers)
	{
		if (p->volume == 0 && p->storage() == 0) continue;
		LinearLayoutParameter* po = LinearLayoutParameter::create();
		po->setMargin(ui::Margin(5, 5, 5, 5));
		auto n = create_producer_view2(p);
		_producer_views->addChild(n);
		n->setLayoutParameter(po);
	}
	bool first = true;
	for (auto* p : Factory->sellers)
	{
		if (p->volume == 0 && p->storage() == 0) continue;
		LinearLayoutParameter* po = LinearLayoutParameter::create();
		po->setMargin(ui::Margin(5, first ? 20 : 5, 5, 5));
		first = false;
		auto n = create_producer_view2(p);
		_producer_views->addChild(n);
		n->setLayoutParameter(po);
	}

	this->forceDoLayout();
}

void FactoryView::update(float delta)
{
	static int k = 0;
	if (++k % 100 == 0 && _factory)
	{
		set_factory(_factory);
	}
}

void FactoryView::doLayout()
{
	Size var = getContentSize();

	_money_txt->setPosition(Vec2(30, var.height));
	_money_val->setPosition(Vec2(100, var.height));

	_producer_views->setPosition(Vec2(0, var.height - 20));
	_producer_views->setContentSize(Size(var.width, var.height - 20));

	if (!_factory) return;

	int hh = 20;
	int h2 = hh + 10;
	//Size s(hh, hh);
	auto titles = _producer_views->getChildren().at(0);
	titles->setPosition(30, 0);
	titles->setContentSize(Size(var.width - titles->getPositionX(), h2));
	
	//auto dollar = (ImageView*)h->getChildren().at(0);
	//auto storage = (ImageView*)h->getChildren().at(1);
	//dollar->setContentSize(s);
	//storage->setContentSize(s);
	//float w = var.width;

	//dollar->setPosition(Vec2(w / 4, h2 / 2));
	//storage->setPosition(Vec2(3 * w / 4, h2 / 2));
}

ui::HBox* FactoryView::create_producer_view(Trader* p)
{
	int w = getContentSize().width;
	int wd = (getContentSize().width - 50) / 3;
	int h = 50;
	ui::HBox* prodview = HBox::create();
	prodview->setContentSize(Size(w, h));

	LinearLayoutParameter* po = LinearLayoutParameter::create();
	po->setMargin(ui::Margin(5, 5, 5, 5));
	po->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	auto sprite = ProductSprite::create(p->product, 20);
	sprite->setLayoutParameter(po);
	prodview->addChild(sprite);

	Diagram* dia = Diagram::create(&p->history_price, 20, 0, 100);
	prodview->addChild(dia);
	dia->setContentSize(Size(wd, h));
	dia->setLayoutParameter(po);
	
	dia = Diagram::create(&p->history_storage, 20, 0, 100);
	prodview->addChild(dia);
	dia->setContentSize(Size(wd, h));
	dia->setLayoutParameter(po);

	dia = Diagram::create(&p->history_trade, 20, 0, 100);
	prodview->addChild(dia);
	dia->setContentSize(Size(wd, h));
	dia->setLayoutParameter(po);

	return prodview;
}

ui::HBox* FactoryView::create_producer_view2(Trader* p)
{
	int w = getContentSize().width;
	int wd = (getContentSize().width - 50) / 5;
	int h = 50;
	ui::HBox* prodview = HBox::create();
	prodview->setContentSize(Size(w, h));

	LinearLayoutParameter* po = LinearLayoutParameter::create();
	po->setMargin(ui::Margin(5, 5, 5, 5));
	po->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	auto sprite = ProductSprite::create(p->product, 20);
	sprite->setLayoutParameter(po);
	prodview->addChild(sprite);

	auto f = [prodview, wd, h, po](double* d) {
		Label* label = Label::create(d);
		label->setContentSize(Size(wd, h));
		label->setLayoutParameter(po);
		prodview->addChild(label);
	};

	f(&p->price);
	f(&p->storage());
	f(&p->volume);
	f(&p->vol_out);
	f(&p->free_volume);

	return prodview;
}

bool Diagram::init(history_t* data, int count, double min, double max)
{
	if (!Layout::init()) return false;
	_data = data;
	_count = count;
	_min = min;
	_max = max;
	_text = ui::Text::create();
	_text->setZOrder(getZOrder() + 10);
	addChild(_text);
	return true;
}

void Diagram::onDraw(const Mat4 &transform, uint32_t flags)
{
	if (!_data) return;

	Director* director = Director::getInstance();
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);

	//draw
	CHECK_GL_ERROR_DEBUG();

	auto s = getContentSize();
	
	// history_t &v = *_data;
	double text_w = 20;
	double w = s.width - text_w;
	double dx = w / _count;
	double a = s.height / (_max - _min);
	double b = -s.height * _min / (_max - _min);

	DrawPrimitives::drawSolidRect(Vec2(text_w, 0), Vec2(w, s.height), Color4F(0.4f, 0.1f, 0.6f, 1));
	double x = text_w;
	int i = 0;
	for (double d : *_data)
	{
		// double y = s.height * (d - _min) / (_max - _min);
		double y = a * d + b;
		y = std::max(0.0, y);
		y = std::min((double)s.height, y);
		DrawPrimitives::setDrawColor4F(1, 1, 1, 1);
		DrawPrimitives::setPointSize(3);
		DrawPrimitives::drawPoint(Vec2(x, y));
		x += dx;
	}

	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

void Diagram::draw(Renderer *renderer, const Mat4& transform, uint32_t flags)
// void Diagram::draw(Renderer *renderer, const kmMat4& transform, bool transformUpdated)
{
	if (_data && _data->size() > 0)
	{
		double d = _data->back();
		_text->setText(to_string((int)d));
	}
	_customCommand.init(100);
	_customCommand.func = CC_CALLBACK_0(Diagram::onDraw, this, transform, flags);
	renderer->addCommand(&_customCommand);
}

//ProductSprite* ProductSprite::create(Product* p, int size)
//{
//	ProductSprite* result = new ProductSprite();
//	if (result->init())
//	{
//		result->loadTexture(p->icon_file);
//		result->ignoreContentAdaptWithSize(false);
//		result->setAnchorPoint(Vec2(0.5, 0.5));
//		result->setContentSize(Size(size, size));
//		result->autorelease();
//		return result;
//	}
//	else
//	{
//		CC_SAFE_DELETE(result);
//		return nullptr;
//	}
//}

bool ProductSprite::init(Product * p, int size)
{
	if (!ImageView::init(p->icon_file)) return false;
	loadTexture(p->icon_file);
	ignoreContentAdaptWithSize(false);
	setAnchorPoint(Vec2(0.5, 0.5));
	setSize(Size(size, size));
	return true;
}

ProductStringView* ProductStringView::create(int size)
{
	ProductStringView* result = new ProductStringView();
	result->_size = size;
	if (result->init())
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

void ProductStringView::set_vector(const Products& v, int size)
{
	int i = 0;
	int x = size / 2;
	for (auto& d : v)
	{
		Product* p = world.get_products()[i];
		int m = d;
		if (m > 4)
		{
			int s2 = 1.3 * size;

			auto s = ProductSprite::create(p, s2);
			s->setPosition(Vec2(x, 0));
			addChild(s);
			
			auto t = Text::create(to_string(m), "Arial", 20);
			t->setColor(Color3B::WHITE);
			t->enableShadow();
			t->setPosition(Vec2(x, 0));
			t->setContentSize(Size(size, size));
			t->setTextHorizontalAlignment(TextHAlignment::CENTER);
			t->setTextVerticalAlignment(TextVAlignment::CENTER);
			t->ignoreContentAdaptWithSize(false);
			addChild(t);

			x += s2;
		}
		else
		{
			for (int k = 0; k < m; ++k)
			{
				auto s = ProductSprite::create(p, size);
				s->setPosition(Vec2(x, 0));
				x += size / 2;
				addChild(s);
			}
			if (m > 0)
			{
				x += size / 3;
			}
		}
		++i;
	}
	setContentSize(Size(x, size));
}

void ProductStringView::set_map(const ProductMap& map)
{
	removeAllChildren();
	setContentSize(Size(0, _size));
	for (auto& p : map)
	{
		add_item(world.get_products()[p.first], p.second);
	}
}

void ProductStringView::add_item(Product* p, double volume)
{
	float x = getContentSize().width + _size / 2;
	if (volume > 4)
	{
		int s2 = 1.3 * _size;

		auto s = ProductSprite::create(p, s2);
		s->setPosition(Vec2(x, 0));
		addChild(s);

		string text;
		if (volume > 999)
		{
			int i = volume;
			i = i - i % 100;
			text = to_string_with_precision(i / 1000.0, 1) + "K";
		}
		//else if ((int)volume == volume)
		//{
		//	text = to_string((int)volume);
		//}
		else
		{
			text = to_string_with_precision(volume, 0);
		}
		auto t = Text::create(text, "Arial", 20);
		t->setColor(Color3B::WHITE);
		t->enableShadow();
		t->setPosition(Vec2(x, 0));
		t->setContentSize(Size(_size, _size));
		t->setTextHorizontalAlignment(TextHAlignment::CENTER);
		t->setTextVerticalAlignment(TextVAlignment::CENTER);
		t->ignoreContentAdaptWithSize(false);
		addChild(t);

		x += s2;
	}
	else
	{
		for (int k = 0; k < volume; ++k)
		{
			auto s = ProductSprite::create(p, _size);
			s->setPosition(Vec2(x, 0));
			x += _size / 2;
			addChild(s);
		}
		if (volume > 0)
		{
			x += _size / 3;
		}
	}

	setContentSize(Size(x, _size));
}

bool EconomyView::init()
{
	if (!Panel::init()) return false;
	_table = Table::create();
	_table->set_cell_size(20, 20);
	return true;
}

void EconomyView::add(Product * p)
{
	auto row = _table->create_row();
	row->addChild(ProductSprite::create(p, 20));
	row->addChild(Label::create(&p->average_price));
}

bool Table::init()
{
	if (!Layout::init()) return false;
	left = top = right = bottom = MARGIN_NORMAL;
	pad_x = pad_y = PAD_NORMAL;
	return true;
}

void Table::set_sizes(float cell_height, std::vector<float> col_sizes)
{
	this->cell_height = cell_height;
	this->col_sizes = col_sizes;
}

void Table::set_cell_size(float width, float height)
{
	this->cell_width = width;
	this->cell_height = height;
}

void Table::set_margins(float pad_x, float pad_y, float left, float top, float right, float bottom)
{
	this->pad_x = pad_x;
	this->pad_y = pad_y;
	this->left = left;
	this->top = top;
	this->right = right;
	this->bottom = bottom;
}

void Table::doLayout()
{
	float x, y = top;
	for (Node* row : getChildren())
	{
		x = left;
		int col = 0;
		for (Node* cell : row->getChildren())
		{
			cell->setPosition(Vec2(x, y));
			cell->setAnchorPoint(Vec2(0, 0));
			float col_width = col < col_sizes.size() ? col_sizes[col] : cell_width;
			cell->setContentSize(Size(col_width, cell_height));
			x += pad_x + col_width;
		}
		y += cell_height + pad_y;
	}
	_contentSize = Size(x - pad_x + right, y - pad_y + bottom);
}

Node * Table::create_row()
{
	Node* row = Node::create();
	row->setAnchorPoint(Vec2(0, 0));
	this->addChild(row);
	return row;
}

bool Label::init(const std::string & text, const LabelSize & size)
{
	if (Text::init(text, "verdana", font_size(size)))
	{
		scheduleUpdate();
		ignoreContentAdaptWithSize(false);
		return true;
	}
	return false;
}

bool Label::init(double * data, const LabelSize& size)
{
	if (init("", size))
	{
		this->data = data;
		return true;
	}
	return false;
}

float Label::font_size(const LabelSize & size)
{
	switch (size)
	{
	case LS_SMALL:
		return 8.0f;
	case LS_LARGE:
		return 20.0f;
	case LS_NORMAL:
	default:
		return 12.0f;
	}
}

void Label::update(float delta)
{
	if (data) this->setText(to_string_with_precision(*data, 1));
}

bool Popup::init()
{
	if (!Panel::init()) return false;
	table = Table::create();
	vector<float> cells;
	cells.push_back(40);
	cells.push_back(100);
	table->set_sizes(Label::font_size(LS_NORMAL) + 5, cells);
	addChild(table);
	return true;
}

void Popup::doLayout()
{
	Panel::doLayout();
	table->setPosition(Vec2(0, 0));
	table->doLayout();
	Size s = table->getContentSize();
	setContentSize(s);
}

}