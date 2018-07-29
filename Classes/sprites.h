#pragma once
#include "world.h"
#include "controls.h"

namespace simciv
{
	const int cs = 32; // cell size
	class MapView;

	class Sprites
	{
	public:
		static cocos2d::Node* create(Industry* f, const Size& s, bool use_bck = false);
		static cocos2d::Node* create(Product* f, const Size& s, bool use_bck = false);
		static cocos2d::Node* create(std::string file_name, const Size& s, bool use_bck = false);
		static void set_scale(cocos2d::Node* sprite, const Size& size);
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
		void update_route(MapView* map);
		void stop();
		void start();
		int time;
	private:
		Transport* transport;
		cocos2d::Node* sprite;
	};

	/// Factory icon + progressbar.
	/// It's not a Node, because it doesn't want to be the parent of the children (because of sprite-drawing performance causes)
	struct FactorySprite
	{
		FactorySprite();
		~FactorySprite();
		virtual void update(float dt);
		void set_show_progressbar(bool show);
		void setPosition(const cocos2d::Vec2& p);
		Factory* _factory;
		cocos2d::DrawNode* _progress_bar;
		//cocos2d::Sprite* _sprite;
		std::vector<cocos2d::Node*> _nodes;
		cocos2d::Vec2 _position;
		cocos2d::Node* _layer;
		Area* area;
	};

	class CircleShape : public DrawNode
	{
	public:
		static CircleShape* create(Color4F& color, Size& size)
		CREATE_FUNC_BODY(CircleShape, color, size)
		bool init(Color4F& color, Size& size);
	};

	class Circle : public cocos2d::ui::ImageView
	{
	public:
		static Circle* create(int prod_id, bool filled, int size)
		CREATE_FUNC_BODY(Circle, prod_id, filled, size)
		bool init(int prod_id, bool filled, int size);
		// virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
	protected:
		bool filled;
		int prod_id;
	};

	class CircleNumber : public Circle
	{
	public:
		static CircleNumber* create(double val, int prod_id, bool filled, int size)
		CREATE_FUNC_BODY(CircleNumber, val, prod_id, filled, size)
		bool init(double val, int prod_id, bool filled, int size);
	protected:
		ui::Text* text;
	};

	class CircleFactory : public DrawNode
	{
	public:
		static CircleFactory* create(Industry* ind, Size& size)
		CREATE_FUNC_BODY(CircleFactory, ind, size)
		bool init(Industry* ind, Size& size);
		void update_colors();
		bool has_factory;
	protected:
		vector<Color4F> color_in, color_out;
		Industry* industry;
		float rad_1, rad_2;
	};
}