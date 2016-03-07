#include "world_ui.h"
#include "controls.h"

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;

const string def_font = "verdana";
const int def_font_size = 12;

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


std::string get_animal_texture(int id)
{
	const std::string files[46] = { "giraffe.png",
		"gorilla.png",
		"hippo.png",
		"horse.png",
		"insect.png",
		"lion.png",
		"monkey.png",
		"moose.png",
		"mouse.png",
		"owl.png",
		"panda.png",
		"penguin.png",
		"pig.png",
		"rabbit.png",
		"rhino.png",
		"rooster.png",
		"shark.png",
		"sheep.png",
		"snake.png",
		"tiger.png",
		"turkey.png",
		"turtle.png",
		"wolf.png",
		"alligator.png",
		"ant.png",
		"bat.png",
		"bear.png",
		"bee.png",
		"bird.png",
		"bull.png",
		"bulldog.png",
		"butterfly.png",
		"cat.png",
		"chicken.png",
		"cow.png",
		"crab.png",
		"crocodile.png",
		"deer.png",
		"dog.png",
		"donkey.png",
		"duck.png",
		"eagle.png",
		"elephant.png",
		"fish.png",
		"fox.png",
		"frog.png" };

	return "img/animals/" + files[id];
}

/*
std::string get_plant_texture(int id)
{
	const std::string files[6] = { "Tomato-icon.png",
		"Cabbage-icon.png",
		"Eggplant-icon.png",
		"Onion-icon.png",
		"Pepper-icon.png",
		"Pumpkin-icon.png" };

	return "img/plants/" + files[id];
}
*/
//std::string get_plant_texture(int id)
//{
//	int level = id / 3;
//	int color = id % 3;
//	return "img/shapes/shape_" + std::to_string(level) + "_" + std::to_string(color) + ".png";
//}

std::string get_plant_texture(int id)
{
	return _model.get_plants()[id]->icon_file;
}

Layout* combobox(const std::string* labels)
{
	return NULL;
}

RadioBox::RadioBox (int* data, std::vector<std::string> labels, int hh, int marginy): data(data), hh(hh), marginy(marginy)
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
	*(this->data) = i;
	if (changed)
	{
		changed(i);
	}
}

RadioBox* RadioBox::create(int* data, std::vector<std::string> labels, int hh, int marginy)
{
	RadioBox* widget = new RadioBox(data, labels, hh, marginy);
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
	auto btn = MenuButton::create(Size(64, 64), texture, "Circle_Orange2.png", "Circle_Orange2_sel.png", "Circle_Blue2.png");
	btn->setAnchorPoint(Vec2(0, 1));
	btn->ignoreContentAdaptWithSize(false);
	return btn;
}

MenuButton* MenuButton::create(Size size, std::string image, std::string bck_normal, std::string bck_selected, std::string bck_disabled)
{
	MenuButton* result = new MenuButton();
	if (result->init())
	{
		result->autorelease();
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

		return result;
	}
	else
	{
		CC_SAFE_DELETE(result);
		return nullptr;
	}
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

RadioMenu::RadioMenu() : _selected(NULL), _space(15), _toggle(false), _count(0)
{
	//add_row();
}

RadioMenu* RadioMenu::create()
{
	RadioMenu* result = new RadioMenu();
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
	_row = HBox::create();
	addChild(_row);
}

void RadioMenu::add_radio_button(MenuButton* btn)
{
	btn->set_toggle(true);
	btn->addTouchEventListener(CC_CALLBACK_2(RadioMenu::on_btn_clicked, this));
	btn->setTag(_count++);
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

MyPanel::MyPanel()
{
	_bck = LayerColor::create(def_bck_color4B);
	_bck->setZOrder(0);
	//_bck->setVisible(false);
	addChild(_bck);
}

Text* MyPanel::create_label(std::string text)
{
	auto label = ui::Text::create(text, "Arial", 12);
	label->ignoreContentAdaptWithSize(false);
	return label;
}

DebugLabel* MyPanel::create_data_label(double* data)
{
	//LinearLayoutParameter* po = LinearLayoutParameter::create();
	//po->setMargin(ui::Margin(5, 5, 5, 5));
	//po->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	DebugLabel* label = new DebugLabel();
	label->ignoreContentAdaptWithSize(false);
	//label->setContentSize(Size(wd, h));
	//label->setLayoutParameter(po);
	//prodview->addChild(label);
	label->data = data;
	return label;
}

void MyPanel::setContentSize(const Size & var)
{
	Layout::setContentSize(var);
	_bck->setContentSize(var);
}

SpeciesView::SpeciesView()
{
	_production_view = VBox::create();
	_production_view->setAnchorPoint(Vec2(0, 1));
	addChild(_production_view);

	_icon = ImageView::create();
	_icon->setAnchorPoint(Vec2(0, 1));
	_icon->ignoreContentAdaptWithSize(false);
	_icon->setContentSize(Size(50, 50));
	addChild(_icon);

	_name_label = Text::create("Giraffe", def_font, 20);
	_name_label->setAnchorPoint(Vec2(0, 1));
	_name_label->setTextHorizontalAlignment(TextHAlignment::LEFT);
	addChild(_name_label);

	_build_cost = MaterialStringView::create(30);
	addChild(_build_cost);

	_build_cost_label = Text::create("Build cost", def_font, def_font_size);
	_build_cost_label->setAnchorPoint(Vec2(0, 1));
	_build_cost_label->setTextHorizontalAlignment(TextHAlignment::LEFT);
	addChild(_build_cost_label);

	//LinearLayoutParameter* p = LinearLayoutParameter::create();
	//p->setGravity(LinearLayoutParameter::LinearGravity::TOP);
	//p->setMargin(Margin(2, 2, 2, 2));
	//_left->setLayoutParameter(p);

	
}

SpeciesView* SpeciesView::create()
{
	SpeciesView* result = new SpeciesView();
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

bool SpeciesView::init()
{
	if (Layout::init())
	{
		setContentSize(Size(300, 500));
		return true;
	}
	return false;
}

void SpeciesView::set_species(Species* species)
{
	this->_species = species;
	if (species)
	{
		string file = species->icon_file; //get_animal_texture(species->id);
		_icon->loadTexture(file);

		_name_label->setString(file.substr(12, file.length() - 4 - 12));

		//_icon->setTexture(get_animal_texture(species->id));
		//auto t = _icon->getTexture();
		//_icon->setScale(50.0 / std::max(t->getPixelsWide(), t->getPixelsHigh()));

		_build_cost->set_vector(species->build_cost, 30);


		_production_view->removeAllChildrenWithCleanup(true);

		for (auto rule : species->m2m_rules)
		{
			Size ss = getContentSize();

			HBox* h = HBox::create();
			h->setContentSize(Size(ss.width, 30));
			auto left = MaterialStringView::create(30);
			left->set_map(rule.input);
			left->setContentSize(Size(ss.width / 2, 30));
			auto right = MaterialStringView::create(30);
			right->set_map(rule.output);

			h->addChild(left);
			h->addChild(right);

			_production_view->addChild(h);
		}
		auto s = getContentSize();
		setContentSize(s);
	}
}

void SpeciesView::add_prod_row(MaterialVec& prod)
{

}

void SpeciesView::setContentSize(const Size & var)
{
	MyPanel::setContentSize(var);
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

AnimalView::AnimalView() : _animal(NULL)
{
	_producer_views = VBox::create();
	_producer_views->setAnchorPoint(Vec2(0, 1));
	_producer_views->setPosition(Vec2(0, 0));
	addChild(_producer_views);
}

AnimalView* AnimalView::create()
{
	AnimalView* result = new AnimalView();
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

bool AnimalView::init()
{
	if (Layout::init())
	{
		setContentSize(Size(300, 500));

		_money_txt = create_label("Money");
		addChild(_money_txt);

		_money_val = create_data_label(NULL);
		_money_val->setSize(Size(100, 20));
		addChild(_money_val);

		scheduleUpdate();
		return true;
	}
	return false;
}

void AnimalView::set_animal(Animal* animal)
{
	_animal = animal;
	_producer_views->removeAllChildrenWithCleanup(true);
	
	_money_val->data = &animal->money;

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

	for (auto* p : animal->buyers)
	{
		//if (p->prod_id % 3 != 0) continue;
		//if (p->volume == 0 && p->storage() == 0) continue;
		LinearLayoutParameter* po = LinearLayoutParameter::create();
		po->setMargin(ui::Margin(5, 5, 5, 5));
		auto n = create_producer_view2(p);
		_producer_views->addChild(n);
		n->setLayoutParameter(po);
	}
	bool first = true;
	for (auto* p : animal->sellers)
	{
		//if (p->prod_id % 3 != 0) continue;
		//if (p->volume == 0 && p->storage() == 0) continue;
		LinearLayoutParameter* po = LinearLayoutParameter::create();
		po->setMargin(ui::Margin(5, first ? 20 : 5, 5, 5));
		first = false;
		auto n = create_producer_view2(p);
		_producer_views->addChild(n);
		n->setLayoutParameter(po);
	}

	this->forceDoLayout();
}

void AnimalView::update(float delta)
{
	static int k = 0;
	if (++k % 100 == 0 && _animal)
	{
		set_animal(_animal);
	}
}

void AnimalView::setContentSize(const Size & var)
{
	MyPanel::setContentSize(var);
}

void AnimalView::doLayout()
{
	Size var = getContentSize();

	_money_txt->setPosition(Vec2(30, var.height));
	_money_val->setPosition(Vec2(100, var.height));

	_producer_views->setPosition(Vec2(0, var.height - 20));
	_producer_views->setContentSize(Size(var.width, var.height - 20));

	if (!_animal) return;

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

ui::HBox* AnimalView::create_producer_view(Producer* p)
{
	int w = getContentSize().width;
	int wd = (getContentSize().width - 50) / 3;
	int h = 50;
	ui::HBox* prodview = HBox::create();
	prodview->setContentSize(Size(w, h));

	LinearLayoutParameter* po = LinearLayoutParameter::create();
	po->setMargin(ui::Margin(5, 5, 5, 5));
	po->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	auto sprite = MaterialSprite::create(p->prod_id, 20);
	sprite->setLayoutParameter(po);
	prodview->addChild(sprite);

	Diagram* dia = Diagram::create();
	prodview->addChild(dia);
	dia->setContentSize(Size(wd, h));
	dia->set_range(20, 0, 100);
	dia->set_data(&p->history_price);
	dia->setLayoutParameter(po);
	
	dia = Diagram::create();
	prodview->addChild(dia);
	dia->setContentSize(Size(wd, h));
	dia->set_range(20, 0, 100);
	dia->set_data(&p->history_storage);
	dia->setLayoutParameter(po);

	dia = Diagram::create();
	prodview->addChild(dia);
	dia->setContentSize(Size(wd, h));
	dia->set_range(20, 0, 10);
	dia->set_data(&p->history_trade);
	dia->setLayoutParameter(po);

	return prodview;
}

ui::HBox* AnimalView::create_producer_view2(Producer* p)
{
	int w = getContentSize().width;
	int wd = (getContentSize().width - 50) / 5;
	int h = 50;
	ui::HBox* prodview = HBox::create();
	prodview->setContentSize(Size(w, h));

	LinearLayoutParameter* po = LinearLayoutParameter::create();
	po->setMargin(ui::Margin(5, 5, 5, 5));
	po->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	auto sprite = MaterialSprite::create(p->prod_id, 20);
	sprite->setLayoutParameter(po);
	prodview->addChild(sprite);

	auto f = [prodview, wd, h, po](double* d) {
		DebugLabel* label = new DebugLabel();
		label->ignoreContentAdaptWithSize(false);
		label->setContentSize(Size(wd, h));
		label->setLayoutParameter(po);
		prodview->addChild(label);
		label->data = d;
	};

	f(&p->price);
	f(&p->storage());
	f(&p->volume);
	f(&p->vol_out);
	f(&p->free_volume);

	return prodview;
}

Diagram* Diagram::create()
{
	Diagram* result = new Diagram();
	//result->setZOrder(99);
	if (result && result->init())
	{
		result->autorelease();
		result->_text = ui::Text::create();
		result->_text->setZOrder(result->getZOrder() + 10);
		result->addChild(result->_text);
		return result;
	}
	else
	{
		CC_SAFE_DELETE(result);
		return nullptr;
	}
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

	DrawPrimitives::drawSolidRect(Vec2(text_w, 0), Vec2(w, s.height), Color4F(0.4, 0.1, 0.6, 1));
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

MaterialSprite* MaterialSprite::create(int id, int size)
{
	MaterialSprite* result = new MaterialSprite();
	if (result->init())
	{
		result->loadTexture(get_plant_texture(id));
		result->ignoreContentAdaptWithSize(false);
		result->setAnchorPoint(Vec2(0.5, 0.5));
		result->setContentSize(Size(size, size));
		result->autorelease();
		return result;
	}
	else
	{
		CC_SAFE_DELETE(result);
		return nullptr;
	}
}

MaterialStringView* MaterialStringView::create(int size)
{
	MaterialStringView* result = new MaterialStringView();
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

void MaterialStringView::set_vector(const MaterialVec& v, int size)
{
	int i = 0;
	int x = size / 2;
	for (auto& d : v)
	{
		int m = d;
		if (m > 4)
		{
			int s2 = 1.3 * size;

			auto s = MaterialSprite::create(i, s2);
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
				auto s = MaterialSprite::create(i, size);
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

void MaterialStringView::set_map(const MaterialMap& map)
{
	for (auto& p : map)
	{
		add_item(p.first, p.second);
	}
}

void MaterialStringView::add_item(int prod_id, double volume)
{
	float x = getContentSize().width + _size / 2;
	if (volume > 4)
	{
		int s2 = 1.3 * _size;

		auto s = MaterialSprite::create(prod_id, s2);
		s->setPosition(Vec2(x, 0));
		addChild(s);

		auto t = Text::create(to_string(volume), "Arial", 20);
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
			auto s = MaterialSprite::create(prod_id, _size);
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

}