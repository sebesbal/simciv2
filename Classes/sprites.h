#pragma once
#include "world.h"

USING_NS_CC;

namespace simciv
{
	class MapView;

	class Sprites
	{
	public:
		static Sprite* create(Industry* f, const Size& s, bool use_bck = false);
		static Sprite* create(Product* f, const Size& s, bool use_bck = false);
		static Sprite* create(std::string file_name, const Size& s, bool use_bck = false);
		static void set_scale(Sprite* sprite, const Size& size);
	};

	class RoadView : public cocos2d::Sprite
	{
	public:
		RoadView() : level(1), direction(0) {  }
		CREATE_FUNC(RoadView)
			int level;
		int direction;
		Area* area;
		static cocos2d::SpriteBatchNode* create_batch_node(std::string file);

		static RoadView* create(Area* a, const int& dir1, const int& dir2, int level = -1);
		static RoadView* create(Area* a, Area* b, Area* c, int level = -1);
		static int road_level(Area* a, Area* b, Area* c);

	protected:
		static cocos2d::SpriteFrame* frames[5][9][9];
	};

	/// Animation for draw transports
	class TransportAnimation
	{
	public:
		TransportAnimation();
		void set_route(Product* prod, Transport* transport, MapView* map);
		void stop();
		void start();
	private:
		Transport* transport;
		Sprite* sprite;
	};
}