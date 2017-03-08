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
}