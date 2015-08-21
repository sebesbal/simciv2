#include "world_ui.h"
#include "controls.h"

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;

const string def_font = "verdana";
const int def_font_size = 12;
const int spec_count = 6;
const int mat_count = 6;

Layout* labelled_cb(std::string text, bool checked, CheckBox::ccCheckBoxCallback cb)
{
	auto l = HBox::create();
	auto p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	CheckBox* chb = CheckBox::create("cocosui/check_box_normal.png",
		"cocosui/check_box_normal_press.png",
		"cocosui/check_box_active.png",
		"cocosui/check_box_normal_disable.png",
		"cocosui/check_box_active_disable.png");
	chb->setSelectedState(checked);
	chb->addEventListener(cb);
	chb->setLayoutParameter(p);
	l->addChild(chb);

	auto label = Text::create();
	label->setString(text);
	label->setFontSize(18);
	label->setLayoutParameter(p);
	l->addChild(label);
	l->requestDoLayout();
	auto height = std::max(chb->getSize().height, label->getSize().height);
	l->setSize(Size(100, height));
	label->setTouchEnabled(true);
	label->addTouchEventListener([chb, cb](Ref*, Widget::TouchEventType type)
	{
		if (type == Widget::TouchEventType::ENDED)
		{
			chb->setSelectedState(!chb->getSelectedState());
			cb(NULL, chb->getSelectedState() ? CheckBox::EventType::SELECTED : CheckBox::EventType::UNSELECTED);
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
		((CheckBox*)item->getChildren().at(0))->setSelectedState(i == l++);
	}
	*(this->data) = i;
}

RadioBox* RadioBox::create(int* data, std::vector<std::string> labels, int hh, int marginy)
{
	RadioBox* widget = new RadioBox(data, labels, hh, marginy);
	if (widget && widget->init())
	{
		widget->autorelease();
		widget->setSize(Size(100, hh + marginy));
		return widget;
	}
	else
	{
		CC_SAFE_DELETE(widget);
		return nullptr;
	}
}

#define RBON "cocosui/Radio_button_on.png"
#define RBOFF "cocosui/Radio_button_off.png"

Layout* RadioBox::labelled_radio(std::string text, CheckBox::ccCheckBoxCallback cb)
{
	auto l = HBox::create();
	auto p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	CheckBox* chb = CheckBox::create(RBOFF, RBOFF, RBON, RBOFF, RBON);
	chb->setSelectedState(false);
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
			chb->setSelectedState(true);
			cb(NULL, CheckBox::EventType::SELECTED); 
		}
	});

	l->addChild(label);
	l->requestDoLayout();
	auto height = std::max(chb->getSize().height, label->getSize().height);
	l->setSize(Size(100, height));

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
	chb->setSelectedState(false);
	chb->addEventListener(cb);
	chb->setLayoutParameter(p);
	l->addChild(chb);
	
	auto image = Widget::create();
	auto s = Sprite::create(img);
	s->setScale(hh / s->getContentSize().height);
	s->setPosition(hh/2, hh/2);
	image->addChild(s);
	image->setLayoutParameter(p);
	image->setSize(Size(hh, hh));
	image->setTouchEnabled(true);
	image->addTouchEventListener([chb, cb](Ref*,Widget::TouchEventType type)
	{ 
		if (type == Widget::TouchEventType::ENDED)
		{
			chb->setSelectedState(true);
			cb(NULL, CheckBox::EventType::SELECTED); 
		}
	});
	l->addChild(image);
	//l->requestDoLayout();
	// auto height = std::max(chb->getSize().height, label->getSize().height);
	l->setSize(Size(100, hh));

	LinearLayoutParameter* pp = LinearLayoutParameter::create();
	pp->setGravity(LinearLayoutParameter::LinearGravity::TOP);
	pp->setMargin(Margin(2, marginy, 2, 2));

	l->setLayoutParameter(pp);

	return l;
}

MenuButton::MenuButton() : _is_toggle(false)
{
	
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

RadioMenu::RadioMenu() : _selected(NULL)
{
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
	btn->set_selected(true);
	_selected = btn;
}
 
int RadioMenu::find_btn(MenuButton* btn)
{
	int i = 0;
	for (auto child : getChildren())
	{
		if (child == btn) return i;
		++i;
	}
}

void RadioMenu::add_radio_button(MenuButton* btn)
{
	btn->set_toggle(true);
	btn->addTouchEventListener([this, btn](Ref*, Widget::TouchEventType type)
	{
		this->set_selected_btn(btn);
	});
	addChild(btn);
}

SpeciesBrowser::SpeciesBrowser()
{
	int space = 15;
	int h = getContentSize().height;
	for (int i = 0; i < spec_count; ++i)
	{
		auto btn = MenuButton::create(Size(64, 64), get_animal_texture(i), "Circle_Orange2.png", "Circle_Orange2_sel.png", "Circle_Blue2.png");
		btn->setAnchorPoint(Vec2(0, 1));
		btn->ignoreContentAdaptWithSize(false);
		btn->setPosition(Vec2(0, -i * (64 + space)));

		// addChild(btn);
		add_radio_button(btn);
	}
}

SpeciesBrowser* SpeciesBrowser::create()
{
	SpeciesBrowser* result = new SpeciesBrowser();
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

SpeciesView::SpeciesView()
{
	//_bck = CCLayerColor::create(Color4B(0, 0, 255, 255));
	_bck = CCLayerColor::create(Color4B::BLACK);
	this->addChild(_bck);

	_production_view = VBox::create();
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

	_build_cost = MaterialStringView::create();
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
	if (Layer::init())
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
		_icon->loadTexture(get_animal_texture(species->id));

		//_icon->setTexture(get_animal_texture(species->id));
		//auto t = _icon->getTexture();
		//_icon->setScale(50.0 / std::max(t->getPixelsWide(), t->getPixelsHigh()));

		_build_cost->set_vector(species->build_cost, 30);

		auto s = getContentSize();
		setContentSize(s);
	}
}

void SpeciesView::add_prod_row(MaterialVec& prod)
{

}

void SpeciesView::setContentSize(const Size & var)
{
	CCLayer::setContentSize(var);

	_bck->setContentSize(var);

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

	_production_view->setContentSize(Size(var.width / 2, var.height - 100));
	_production_view->setPosition(Vec2(var.width / 2, 0));
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

MaterialStringView* MaterialStringView::create()
{
	MaterialStringView* result = new MaterialStringView();
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
	int x = 0;
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

}