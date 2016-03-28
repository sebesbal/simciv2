#include "stdafx.h"

#include "sprites.h"

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
}