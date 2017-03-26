#include "stdafx.h"

#include "sprites.h"
#include "map_view.h"

namespace simciv
{
	cocos2d::Node * Sprites::create(Industry * i, const Size& s, bool use_bck)
	{
		return create(i->icon_file, s, use_bck);
	}

	cocos2d::Node * Sprites::create(Product * f, const Size & s, bool use_bck)
	{
		return create(f->icon_file, s, use_bck);
	}

	cocos2d::Node * Sprites::create(std::string file_name, const Size & s, bool use_bck)
	{
		Sprite* sprite = Sprite::create(file_name);
		//if (!sprite) throw "File not found: " + file_name;
		if (!sprite) return cocos2d::Node::create();

		if (use_bck)
		{
			Sprite* bck = Sprite::create("res/img/shapes/white_circle.png");
			set_scale(bck, s);
			bck->addChild(sprite);
			sprite->setPosition(bck->getContentSize() / 2);
			set_scale(sprite, s * (0.8f / bck->getScale()));
			return bck;
		}
		else
		{
			set_scale(sprite, s);
			return sprite;
		}
	}

	void Sprites::set_scale(cocos2d::Node * sprite, const Size & size)
	{
		auto s = sprite->getContentSize();
		if (s.width / s.height > size.width / size.height)
		{
			// sprite is wider than size
			sprite->setScale(size.width / s.width);
		}
		else
		{
			// sprite is taller than size
			sprite->setScale(size.height / s.height);
		}
	}

	FactorySprite::FactorySprite(): _progress_bar(NULL), _factory(NULL)
	{
	}

	FactorySprite::~FactorySprite()
	{
		for (auto n : _nodes)
		{
			n->removeFromParent();
		}
	}

	void FactorySprite::update(float dt)
	{
		switch (_factory->state)
		{
		case FS_UNDER_CONTRUCTION:
		case FS_UPGRADE:
			set_show_progressbar(true);
			break;
		case FS_RUN:
			set_show_progressbar(_factory->health < 1);
			break;
		case FS_DEAD:
			set_show_progressbar(true);
			break;
		default:
			break;
		}
	}
	void FactorySprite::set_show_progressbar(bool show)
	{
		if (show)
		{
			if (!_progress_bar)
			{
				_progress_bar = DrawNode::create();
				//_progress_bar->setScale(1 / _sprite->getScale());
				_layer->addChild(_progress_bar);
				//_nodes[0]->getParent()->addChild(_progress_bar);
				_nodes.push_back(_progress_bar);
			}

			int m = g_factory_layer->cell_size();
			int w = 5;

			Vec2 off(-m / 2, m / 2 - w);
			double h = _factory->health;
			_progress_bar->clear();
			_progress_bar->setLineWidth(10);
			_progress_bar->drawSolidRect(Vec2(0, 0) + off, Vec2(m, w) + off, Color4F(0, 0, 0, 1));
			_progress_bar->setLineWidth(3);
			_progress_bar->drawSolidRect(Vec2(1, 1) + off, Vec2(m * h, w - 1) + off, Color4F(1 - h, h, 0, 1));
			_progress_bar->setPosition(_position);
		}
		else
		{
			if (_progress_bar)
			{
				_nodes.erase(find(_nodes.begin(), _nodes.end(), _progress_bar));
				_progress_bar->removeFromParent();
				_progress_bar = NULL;
			}
		}
	}
	void FactorySprite::setPosition(const cocos2d::Vec2& p)
	{
		_position = p;
		for (auto n : _nodes)
		{
			n->setPosition(p);
		}
	}

	bool Circle::init(int prod_id, bool filled, int size)
	{	
		auto file = world.get_products()[prod_id]->icon_file;
		if (!ImageView::init(file)) return false;

		setContentSize(Size(size, size));
		setAnchorPoint(Vec2(0.5, 0.5));
		this->filled = filled;
		this->prod_id = prod_id; 
		//auto c = world.colors[prod_id];
		//float r = size / 2.0;
		//drawCircle(Vec2(r, r), r, 2 * M_PI, 40, false, c);
		
		
		//auto iv = ImageView::create(file);
		//iv->setContentSize(Size(size, size));
		ignoreContentAdaptWithSize(false);
		//addChild(iv);

		if (!filled)
		{ 
			auto hole = ImageView::create("img/shapes/circle_black.png");
			auto s2 = size / 2.0;
			hole->ignoreContentAdaptWithSize(false);
			hole->setContentSize(Size(s2, s2));
			hole->setAnchorPoint(Vec2(0.5, 0.5));
			hole->setPosition(Vec2(size/2, size/2));
			hole->setZOrder(1);
			addChild(hole);
		}

		return true;
	}
	bool CircleNumber::init(double val, int prod_id, bool filled, int size)
	{
		if (!Circle::init(prod_id, filled, size)) return false;
		
		// text->setText(to_string(val));
		text = ui::Text::create(to_string((int)val), "Arial", 11);
		text->ignoreContentAdaptWithSize(true);
		text->setContentSize(this->getContentSize());
		text->setAnchorPoint(Vec2(0.5, 0.5));
		text->setPosition(this->getContentSize() / 2);
		text->setZOrder(2);
		addChild(text);
		return true;
	}


	void drawPie(DrawNode* node, const Vec2& center, float radius, float startAngle, float endAngle, unsigned int segments, float scaleX, float scaleY, const Color4F &color)
	{
		segments++;
		// auto draw = DrawNode::create();
		const float coef = (endAngle - startAngle) / (segments - 2);

		Vec2 *vertices = new (std::nothrow) Vec2[segments];

		for (unsigned int i = 0; i < segments - 1; i++)
		{
			float rads = i*coef;
			GLfloat j = radius * cosf(rads + startAngle) * scaleX + center.x;
			GLfloat k = radius * sinf(rads + startAngle) * scaleY + center.y;

			vertices[i].x = j;
			vertices[i].y = k;
		}

		vertices[segments - 1].x = center.x;
		vertices[segments - 1].y = center.y;

		node->drawSolidPoly(vertices, segments, color);

		CC_SAFE_DELETE_ARRAY(vertices);
	}

	bool CircleFactory::init(Industry * ind, Size& size)
	{
		if (!DrawNode::init()) return false;
		setContentSize(size);
		this->industry = ind;
		has_factory = false;
		update_colors();
		return true;
	}

	#define lofusz(col, w) col.r *= w; col.g *= w; col.b *= w;
	void CircleFactory::update_colors()
	{
		color_in.clear();
		color_out.clear();
		clear();

		if (!industry)
		{
			return;
		}

		//const cocos2d::Color4F colsou[4] = { Color4F(1, 0, 0, 1), Color4F(0, 1, 0, 1), Color4F(0, 0, 1, 1), Color4F(0.7, 0.7, 0.7, 1) };
		ProductionRule& r = industry->prod_rules[0];
		float in = 0, out = 0;
		for (auto& p : r.input)
		{
			color_in.push_back(world.colors[p.first]);
			in += p.second;
		}
		for (auto& p : r.output)
		{
			color_out.push_back(world.colors[p.first]);
			out += p.second;
		}

		//const float fact_w = 5;
		//float r_weight = (color_in.size() + color_out.size()) / 4.0f;
		//rad_1 = 0.8 / 2 * r_weight;
		//rad_2 = rad_1 + 0.15;

		float rad_2 = max(0.15f, min(0.4f, sqrt(in + out) / 6));
		float rad_1 = rad_2 * sqrt(out) / sqrt(in + out);

		float weight = has_factory ? 1 : 0.4;
		for (auto& c : color_in)
		{
			lofusz(c, weight);
		}

		for (auto& c : color_out)
		{
			lofusz(c, weight);
		}


		// draw
		float size = getContentSize().width;
		int n = color_in.size();
		Vec2 center(0, 0);

		if (n > 0)
		{
			float f = M_PI / 2;
			float df = 2 * M_PI / n;
			for (auto c : color_in)
			{
				drawPie(this, center, size * rad_2, f, f + df, 30, 1, 1, c);
				f += df;
			}
		}

		{
			n = color_out.size();
			float f = 0;
			float df = 2 * M_PI / n;
			for (auto c : color_out)
			{
				drawPie(this, center, size * rad_1, f, f + df, 30, 1, 1, c);
				f += df;
			}
		}
	}

	//void Circle::draw(Renderer * renderer, const Mat4 & transform, uint32_t flags)
	//{
	//	auto c = world.colors[prod_id];
	//	drawCircle(getPosition(), getContentSize().width / 2, 2 * M_PI, 40, false, c);
	//}
}