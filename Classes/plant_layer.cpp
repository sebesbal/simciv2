#include "world_ui.h"
#include "economy.h"
#include "controls.h"
#include <map>

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;

PlantMapLayer* PlantMapLayer::create(WorldModel* model, UIStateData& info)
{
	PlantMapLayer* result = new PlantMapLayer(info);
	result->_model = model;
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

bool is_map_point(cocos2d::Vec2& p)
{
	//return p.y < 557 || p.x > 366;
	return p.x > 370 || p.y > 222;
}

#define DRAW_AREAS(area, exp) \
{ \
	double min = std::numeric_limits<double>::max(); \
	double max = std::numeric_limits<double>::min(); \
	vector<double> u(_model->areas().size()); \
	int i = 0; \
	for (Area* area : _model->areas()) \
	{ \
		double v = exp; \
		min = std::min(min, v); \
		max = std::max(max, v); \
		u[i++] = v; \
	} \
	double d = max - min; \
	i = 0; \
	if (d == 0) \
	{ \
		for (Area* a : _model->areas()) \
		{ \
			draw_rect(a->x, a->y, min, 1); \
		} \
	} \
	else \
	{ \
		for (Area* a : _model->areas()) \
		{ \
			draw_rect(a->x, a->y, (u[i++] - min) / d, 1); \
		} \
	} \
} \

void PlantMapLayer::onDraw(const Mat4 &transform, uint32_t flags)
{
	// calculate roads
	_model->products()[info.plant->id]->routes_to_areas(info.plant->id);

    Director* director = Director::getInstance();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
    
    //draw
    CHECK_GL_ERROR_DEBUG();
	auto b = _map->getBoundingBox();

	if (info.show_grid)
	{
		glLineWidth(1);
		float x = b.getMinX();
		for (int i = 0; i <= _model->width(); ++i, x += cs)
		{
			DrawPrimitives::drawLine( Vec2(x, b.getMinY()), Vec2(x, b.getMaxY()));
		}
		float y = b.getMinY();
		for (int i = 0; i <= _model->height(); ++i, y += cs)
		{
			DrawPrimitives::drawLine( Vec2(b.getMinX(), y), Vec2(b.getMaxX(), y));
		}
	}

	double min_v = 1000;
	double max_v = 0;
	double min_vol = 1000;
	double max_vol = 0;

	switch (info.mode)
	{
	case MM_PRICE_SELL:
		DRAW_AREAS(area, _model->get_prod(area, info.plant->id).p_sell);
		break;
	case MM_PRICE_BUY:
		DRAW_AREAS(area, _model->get_prod(area, info.plant->id).p_buy);
		break;
	case MM_RESOURCES:
		DRAW_AREAS(area, _model->get_prod(area, info.plant->id).resource);
		break;
	case MM_PROFIT:
		if (info.species) DRAW_AREAS(area, ((AnimalWorld*)_model)->get_profit(info.species, area));
		break;
	default:
		break;
	}

	//if (info.price_vol_mode == 0)
	//{
	//	if (info.produce_consume_mode == 2 && info.species)
	//	{
	//		// profit
	//		
	//	}
	//	else if (info.produce_consume_mode == 0)
	//	{
	//		for (Area* a: _model->areas())
	//		{
	//			auto& p = _model->get_prod(a, info.plant->id);
	//			double v = p.p_sell;
	//			min_v = std::min(min_v, v);
	//			max_v = std::max(max_v, v);
	//			double vol = p.p_buy;
	//			min_vol = std::min(min_vol, vol);
	//			max_vol = std::max(max_vol, vol);
	//		}
	//		double d = max_v - min_v;
	//		double d_vol = max_vol - min_vol;

	//		for (Area* a: _model->areas())
	//		{
	//			auto& p = _model->get_prod(a, info.plant->id);
	//			double v = p.p_sell;
	//			double r = d == 0 ? 0.5 : (v - min_v) / d;
	//			double vol = p.p_buy;
	//			draw_rect(a->x, a->y, r, vol / d_vol);
	//			//draw_triangles(a->x, a->y, r, vol / d_vol);
	//			//draw_circles(a->x, a->y, r, vol / d_vol);
	//		}
	//	}
	//	else if (info.produce_consume_mode == 1)
	//	{
	//		for (Area* a: _model->areas())
	//		{
	//			auto& p = _model->get_prod(a, info.plant->id);
	//			double v = p.p_buy;
	//			min_v = std::min(min_v, v);
	//			max_v = std::max(max_v, v);
	//			double vol = p.v_buy;
	//			min_vol = std::min(min_vol, vol);
	//			max_vol = std::max(max_vol, vol);
	//		}
	//		double d = max_v - min_v;
	//		double d_vol = max_vol - min_vol;

	//		for (Area* a: _model->areas())
	//		{
	//			auto& p = _model->get_prod(a, info.plant->id);
	//			double v = p.p_buy;
	//			double r = d == 0 ? 0.5 : (v - min_v) / d;
	//			double vol = p.v_buy;
	//			draw_rect(a->x, a->y, r, vol / d_vol);
	//		}
	//	}
	//}
	//else if (info.price_vol_mode == 2)
	//{
	//	for (Area* a: _model->areas())
	//	{
	//		auto& p = _model->get_prod(a, info.plant->id);
	//		draw_rect_green(a->x, a->y, p.resource, 1);
	//	}
	//}

	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

void PlantMapLayer::update(float delta)
{
	if (info.show_transport)
	{
		for (int prod_id = 0; prod_id < material_count; ++prod_id)
		{
			ProductMap* prod = _model->products()[prod_id];
			auto& v = prod->transports();
			for (auto transport : v)
			{
				auto it = transports.find(transport);
				if (it == transports.end())
				{
					if (transport->route->roads.size() > 0)
					{
						RouteAnimation* ani = new RouteAnimation();
						ani->set_route(prod_id, transport, this);
						transports[transport] = ani;
					}
				}
				else
				{
					Transport* t = it->first;
					if (t->volume == 0 && t->active_time + 50 < _model->time)
					{
						RouteAnimation* ani = it->second;
						ani->stop();
						transports.erase(it);
						delete ani;
					}
					else
					{
						//RouteAnimation* ani = it->second;
						//ani->start();

					}
				}
			}
		}
	}
}

RouteAnimation::RouteAnimation() : sprite(NULL), transport(NULL)
{
}

void RouteAnimation::set_route(int prod_id, Transport* transport, MapView* map)
{
	if (sprite) return;

	// auto it = route->roads.begin();
	Area* a = transport->seller->area;
	//Sprite* sprite = Sprite::create(get_animal_texture(ani->species.id));
	sprite = Sprite::create(get_plant_texture(prod_id));
	Rect r = map->get_rect(a->x, a->y);
	//sprite->setPosition(r.getMidX(), r.getMidY());
	sprite->setScale(0.04f);
	map->addChild(sprite);

	int cs = map->cell_size();
	this->transport = transport;
	float time = 0;

	
	Vector<FiniteTimeAction*> v;

	CCFiniteTimeAction* actionMove = CCMoveTo::create(0, ccp(a->x * cs + cs / 2, a->y * cs + cs / 2));
	v.pushBack(actionMove);

	for (auto r : transport->route->roads)
	{
		Area* b = r->other(a);
		//Area* a = r->a;
		//Area* b = r->b;

		int i = abs(b->x - a->x) + abs(b->y - a->y);

		CCFiniteTimeAction* actionMove = CCMoveTo::create(i == 2 ? sqrt(2) : 1, ccp(b->x * cs + cs / 2, b->y * cs + cs / 2));
		v.pushBack(actionMove);
		
		a = b;

		// time += 0.05;
		//a = b;
	}
	//CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create(this, callfuncN_selector(RouteAnimation::spriteMoveFinished));
	CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create([](CCNode* sender){});
	v.pushBack(actionMoveDone);
	// v.pushBack(NULL);
	
	sprite->runAction(CCRepeatForever::create(CCSequence::create(v)));
}

void RouteAnimation::stop()
{
	if (sprite)
	{
		sprite->removeFromParentAndCleanup(true);
		sprite = NULL;
	}
}

void RouteAnimation::start()
{
	if (sprite)
	{
		sprite->resume();
		sprite->setVisible(true);
		//sprite->getParent()->removeChild(sprite, true);
		//sprite = NULL;
	}
}

//
//void RouteAnimation::spriteMoveFinished(CCNode* sender)
//{
//	//CCSprite *sprite = (CCSprite *)sender;
//	//this->removeChild(sprite, true);
//}

}