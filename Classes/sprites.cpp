#include "stdafx.h"

#include "sprites.h"
#include "map_view.h"

namespace simciv
{
	Sprite * Sprites::create(Industry * i, const Size& s, bool use_bck)
	{
		return create(i->icon_file, s, use_bck);
	}

	Sprite * Sprites::create(Product * f, const Size & s, bool use_bck)
	{
		return create(f->icon_file, s, use_bck);
	}

	Sprite * Sprites::create(std::string file_name, const Size & s, bool use_bck)
	{
		Sprite* sprite = Sprite::create(file_name);
		if (!sprite) throw "File not found: " + file_name;

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

	void Sprites::set_scale(Sprite * sprite, const Size & size)
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

	void FactorySprite::update(float dt)
	{
		if (_factory->state == FS_UNDER_CONTRUCTION)
		{
			if (!_progress_bar)
			{
				_progress_bar = DrawNode::create();
				_progress_bar->setScale(1 / _sprite->getScale());
				_sprite->addChild(_progress_bar);
			}

			int m = g_factory_layer->cell_size();
			int w = 5;

			_progress_bar->clear();
			_progress_bar->setLineWidth(10);
			_progress_bar->drawSolidRect(Vec2(0, 0), Vec2(m, w), Color4F(0, 0, 0, 1));
			_progress_bar->setLineWidth(3);
			_progress_bar->drawSolidRect(Vec2(1, 1), Vec2(m * 0.5, w - 1), Color4F(1, 0, 0, 1));
		}
	}
}