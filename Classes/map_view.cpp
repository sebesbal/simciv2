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
	return world->get_area(x, y);
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

	if (rate < 0)
	{
		DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(0, 0, 0, alpha));
	}
	else
	{
		//DrawPrimitives::drawSolidRect( Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - rate, rate, 0, alpha));
		DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - rate, rate, 0, alpha));
		//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(rate, rate, rate, 1));
		//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(rate, 1- rate, 1-rate, 1));
		//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1-rate, 1 - rate, rate, 1));
		//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(rate / 2, 0, rate / 2, 1));
	}
}

void TileMapView::draw_rect(int x, int y, Color3B & col)
{
	Rect r = get_rect(x, y);
	DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(col));
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

void TileMapView::draw_triangle(int x, int y, float a, float alpha)
{
	Rect r = get_rect(x, y);
	Vec2 v[3];
	v[0] = Vec2(r.getMinX(), r.getMinY());
	v[1] = Vec2(r.getMaxX(), r.getMinY());
	v[2] = Vec2(r.getMinX(), r.getMaxY());
	DrawPrimitives::drawSolidPoly(v, 3, Color4F(1 - a, a, 0, alpha));
}

void TileMapView::draw_circles(int x, int y, float a, float b)
{
	Rect r = get_rect(x, y);
	DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - a, a, 0, 1));
	//DrawPrimitives::setDrawColor4F(1 - b, b, 0, 1);
	DrawPrimitives::setDrawColor4F(b, 1 - b, 0, 1);
	DrawPrimitives::drawSolidCircle(Vec2(r.getMidX(), r.getMidY()), cell_size() / 3.0f, (float)(2 * M_PI), 20);
}

void TileMapView::draw_circles2(int x, int y, float a, float alpha)
{
	Rect r = get_rect(x, y);
	a = std::max(0.02f, a);
	//DrawPrimitives::drawSolidRect(Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - a, a, 0, 1));
	//DrawPrimitives::setDrawColor4F(1 - b, b, 0, 1);
	DrawPrimitives::setDrawColor4F(1 - a, a, 0, alpha);
	//DrawPrimitives::drawSolidCircle(Vec2(r.getMidX(), r.getMidY()), cell_size() / 3.0f, (float)(2 * M_PI), 20);
	DrawPrimitives::drawSolidCircle(Vec2(r.getMidX(), r.getMidY()), a * cell_size() * 0.4, (float)(2 * M_PI), 20);
}

void drawPie(const Vec2& center, float radius, float startAngle, float endAngle, unsigned int segments, float scaleX, float scaleY, const Color4F &color)
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

	DrawPrimitives::drawSolidPoly(vertices, segments, color);

	CC_SAFE_DELETE_ARRAY(vertices);
}

#define lofusz(col, w) col.r *= w; col.g *= w; col.b *= w;
//
//void TileMapView::draw_circles(Area * a)
//{
//	Rect r = get_rect(a->x, a->y);
//	int n = a->color_in.size();
//	float dx = r.size.width / max(1, n);
//	Vec2 u = Vec2(r.getMinX(), r.getMinY());
//	Vec2 v = Vec2(r.getMinX() + dx, r.getMaxY());
//	float weight = (a->color_in.size() + a->color_out.size()) / 4.0f;
//
//	float rad_out = 0.8 * cell_size() / 2 * weight;
//
//	// DrawPrimitives::drawSolidRect(u, Vec2(r.getMaxX(), r.getMaxY()), Color4F(0, 0, 0, 1));
//
//	if (n == 0)
//	{
//		//DrawPrimitives::drawSolidRect(u, v, Color4F(0, 0, 0, 1));
//	}
//	else
//	{
//		//for (auto c : a->color_in)
//		//{
//		//	lofusz(c, 0.4);
//		//	//lofusz(c, weight);
//		//	DrawPrimitives::drawSolidRect(u, v, c);
//		//	u.x += dx;
//		//	v.x += dx;
//		//}
//
//		n = a->color_in.size();
//		float rad = rad_out + cell_size() * 0.15;
//		float f = M_PI / 2;
//		float df = 2 * M_PI / n;
//		for (auto c : a->color_in)
//		{
//			lofusz(c, weight);
//			//lofusz(c, 0.4);
//			// DrawPrimitives::setDrawColor4F(c.r, c.g, c.b, c.a);
//			drawPie(get_point(a), rad, f, f + df, 30, 1, 1, c);
//			f += df;
//			//DrawPrimitives::drawSolidCircle(get_point(a), cell_size() / 4, df, 20);
//		}
//	}
//	
//	{
//		n = a->color_out.size();
//		//float rad = 0.8 * cell_size() / 2 * weight;
//		float f = 0;
//		float df = 2 * M_PI / n;
//		for (auto c : a->color_out)
//		{
//			lofusz(c, weight);
//			//lofusz(c, 0.4);
//			// DrawPrimitives::setDrawColor4F(c.r, c.g, c.b, c.a);
//			drawPie(get_point(a), rad_out, f, f + df, 30, 1, 1, c);
//			f += df;
//			//DrawPrimitives::drawSolidCircle(get_point(a), cell_size() / 4, df, 20);
//		}
//	}
//
//	// 
//}

void TileMapView::draw_circles(Area * a)
{
	//Rect r = get_rect(a->x, a->y);
	//int n = a->color_in.size();

	//if (n > 0)
	//{
	//	float f = M_PI / 2;
	//	float df = 2 * M_PI / n;
	//	for (auto c : a->color_in)
	//	{
	//		drawPie(get_point(a), cell_size() * a->rad_2, f, f + df, 30, 1, 1, c);
	//		f += df;
	//	}
	//}

	//{
	//	n = a->color_out.size();
	//	float f = 0;
	//	float df = 2 * M_PI / n;
	//	for (auto c : a->color_out)
	//	{
	//		drawPie(get_point(a), cell_size() * a->rad_1, f, f + df, 30, 1, 1, c);
	//		f += df;
	//	}
	//}
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