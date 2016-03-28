#include "stdafx.h"

#include "world_ui.h"

namespace simciv
{

USING_NS_CC;
using namespace std;


Rect MapView::get_rect(int x, int y)
{
	auto b = getBoundingBox();
	return Rect(b.getMinX() + cs * x, b.getMinY() + cs * y, cs, cs);
}

Vec2 MapView::get_point(Area * a)
{
	return get_point(a->x, a->y);
}

Vec2 MapView::get_point(int x, int y)
{
	auto b = getBoundingBox();
	return Vec2(b.getMinX() + cs * x + cs / 2, b.getMinY() + cs * y + cs / 2);
}

Area* MapView::get_area(Vec2 p)
{
	int x = p.x / cs;
	int y = p.y / cs;
	return world.get_area(x, y);
}

Vec2 MapView::dir(Area * a, Area * b)
{
	return Vec2(b->x - a->x, b->y - a->y);
}


void TileMapView::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	_customCommand.init(_globalZOrder);
	_customCommand.func = CC_CALLBACK_0(TileMapView::onDraw, this, transform, flags);
	renderer->addCommand(&_customCommand);
}


void TileMapView::draw_rect(int x, int y, float rate, float alpha)
{
	Rect r = get_rect(x, y);

	//DrawPrimitives::drawSolidRect( Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - rate, rate, 0, alpha));
	DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - rate, rate, 0, 0.5));
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(rate, rate, rate, 1));
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(rate, 1- rate, 1-rate, 1));
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1-rate, 1 - rate, rate, 1));
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(rate / 2, 0, rate / 2, 1));

	//alpha = 1 - alpha;
	//if (alpha > 0)
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(alpha * r.getMaxX() + (1-alpha) * r.getMinX(),
	//	alpha * r.getMaxY() + (1 - alpha) * r.getMinY()), Color4F(1 - rate, rate, 0, 1));
}

void TileMapView::draw_triangles(int x, int y, float a, float b)
{
	Rect r = get_rect(x, y);
	Vec2 v[3];
	v[0] = Vec2(r.getMinX(), r.getMinY());
	v[1] = Vec2(r.getMaxX(), r.getMinY());
	v[2] = Vec2(r.getMinX(), r.getMaxY());
	DrawPrimitives::drawSolidPoly(v, 3, Color4F(1 - a, a, 0, 0.8f));
	v[0] = Vec2(r.getMaxX(), r.getMaxY());
	//DrawPrimitives::drawSolidPoly(v, 3, Color4F(b, 1 - b, 0, 0.8));
	DrawPrimitives::drawSolidPoly(v, 3, Color4F(1 - b, b, 0, 0.8f));
}

void TileMapView::draw_circles(int x, int y, float a, float b)
{
	Rect r = get_rect(x, y);
	DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - a, a, 0, 1));
	//DrawPrimitives::setDrawColor4F(1 - b, b, 0, 1);
	DrawPrimitives::setDrawColor4F(b, 1 - b, 0, 1);
	DrawPrimitives::drawSolidCircle(Vec2(r.getMidX(), r.getMidY()), cell_size() / 3.0f, (float)(2 * M_PI), 20);
}

void TileMapView::draw_rect_green(int x, int y, float rate, float alpha)
{
	Rect r = get_rect(x, y);
	//DrawPrimitives::drawSolidRect( Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(0, rate, 0, alpha));
	DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(0, rate, 0, 1));
}

void TileMapView::onDraw(const Mat4 &transform, uint32_t flags)
{

}

}