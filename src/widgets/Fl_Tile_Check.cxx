// ----------------------------------------------------------------------------
// Fl_Tile_Check.cxx
//
// Copyright (C) 2007-2011
//		Stelios Bounanos, M0GLD
//		Dave Freese, W1HKJ
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.	If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <config.h>

#include <stdlib.h>
#include <stdio.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>

#include "Fl_Tile_Check.h"

// Drag the edges that were initially at oldx,oldy to newx,newy:
// pass -1 as oldx or oldy to disable drag in that direction:

void Tile_::position(int oix, int oiy, int newx, int newy) {
	Fl_Widget* const* a = array();
	short* p = (short *)sizes();
//int gX = x(), gW = w();
//printf("gX %d, gW %d\n", gX, gW);
//printf("oix %d, oiy %d, newx %d, newy %d\n", oix, oiy, newx, newy);
	p += 8; // skip group & resizable's saved size
	for (int i=children(); i--; p += 4) {
		Fl_Widget* o = *a++;
		if (o == resizable()) continue;
		int X = o->x();
		int Y = o->y();
		int W = o->w();
		int H = o->h();
		int R = X + W;
		int B = Y + H;
//printf("Child %d : %p : x %d, y %d, w %d, h %d",
//	i, o, X, Y, W, H);
//		if (o == resizable()) {
//printf(" ... resizable\n");
//			continue;
//} else
//printf("\n");
		if (oix > -1) {
			int t = p[0];
			if ((t == oix) || (t>oix && X<newx) || (t<oix && X>newx)) X = newx;
			t = p[1];
			if ((t == oix) || (t>oix && R<newx) || (t<oix && R>newx)) R = newx;
		}
		if (oiy > -1) {
			int t = p[2];
			if ((t == oiy) || (t>oiy && Y<newy) || (t<oiy && Y>newy)) Y = newy;
			t = p[3];
			if ((t == oiy) || (t>oiy && B<newy) || (t<oiy && B>newy)) B = newy;
		}
		o->damage_resize(X, Y, R-X, B-Y);
	}
}


void Tile_::newx( int oix, int newx )
{
//	Fl_Widget* const* a = array();
	short* p = (short *)sizes();
	int gX = x(), gW = w();
printf("gX %d, gW %d; newx %d\n", gX, gW, newx);
	if (newx > p[5]) {
printf("p[5] = %d\n", p[5]);
		newx = p[5] - 1;
	}
	position(oix, -1, newx, -1);
}

// move the lower-right corner (sort of):
void Tile_::resize(int X,int Y,int W,int H) {
	// remember how much to move the child widgets:
	short* p = (short *)sizes();
	int dx = X-x();
	int dy = Y-y();
	int dw = W-w();
	int dh = H-h();
//	int OGR = p[1];
//	int OGB = p[3];
//	int OW = w();
//	int OH = h();
	// resize this (skip the Fl_Group resize):
	Fl_Widget::resize(X,Y,W,H);
	// find bottom-right of resizable:
	int OR = p[5];
	int NR = X + W - (p[1]-OR);
	int OB = p[7];
	int NB = Y + H - (p[3]-OB);
	// move everything to be on correct side of new resizable:
	Fl_Widget*const* a = array();
	p += 8;
	for (int i=children(); i--;) {
		Fl_Widget* o = *a++;
		int xx = o->x()+dx;
		int R = xx+o->w();
		// left
		if (p[0] >= OR) xx += dw; else if (xx > NR) xx = NR;
		// right
		if (p[1] >= OR) R += dw; else if (R > NR) R = NR;
		int yy = o->y()+dy;
		int B = yy+o->h();
		// top
		if (p[2] >= OB) yy += dh; else if (yy > NB) yy = NB;
		// bottom
		if (p[3] >= OB) B += dh; else if (B > NB) B = NB;
		o->resize(xx,yy,R-xx,B-yy);
		p += 4; // next child sizes array
		// do *not* call o->redraw() here! If you do, and the tile is inside a
		// scroll, it'll set the damage areas wrong for all children!
	}
}

static void set_cursor(Tile_ *t, Fl_Cursor c) {
	static Fl_Cursor cursor;
	if (cursor == c || !t->window()) return;
	cursor = c;
	t->window()->cursor(c);
}

static Fl_Cursor cursors[4] = {
	FL_CURSOR_DEFAULT,
	FL_CURSOR_WE,
	FL_CURSOR_NS,
	FL_CURSOR_MOVE};

int Tile_::handle(int event) {
	static int sdrag;
	static int sdx, sdy;
	static int sx, sy;
#define DRAGH 1
#define DRAGV 2
#define GRABAREA 4

	int mx = Fl::event_x();
	int my = Fl::event_y();

	switch (event) {

	case FL_MOVE:
	case FL_ENTER:
	case FL_PUSH: {
		int mindx = 100;
		int mindy = 100;
		int oldx = 0;
		int oldy = 0;
		Fl_Widget*const* a = array();
			short* q = (short *)sizes();
		short* p = q+8;
		for (int i=children(); i--; p += 4) {
			Fl_Widget* o = *a++;
			if (o == resizable()) continue;
			if (p[1]<q[1] && o->y()<=my+GRABAREA && o->y()+o->h()>=my-GRABAREA) {
	int t = mx - (o->x()+o->w());
	if (abs(t) < mindx) {
		sdx = t;
		mindx = abs(t);
		oldx = p[1];
	}
			}
			if (p[3]<q[3] && o->x()<=mx+GRABAREA && o->x()+o->w()>=mx-GRABAREA) {
	int t = my - (o->y()+o->h());
	if (abs(t) < mindy) {
		sdy = t;
		mindy = abs(t);
		oldy = p[3];
	}
			}
		}
		sdrag = 0; sx = sy = -1;
		if (mindx <= GRABAREA) {sdrag = DRAGH; sx = oldx;}
		if (mindy <= GRABAREA) {sdrag |= DRAGV; sy = oldy;}
		set_cursor(this, cursors[sdrag]);
		if (sdrag) return 1;
		return Fl_Group::handle(event);
	}

	case FL_LEAVE:
		set_cursor(this, FL_CURSOR_DEFAULT);
		break;

	case FL_DRAG:
		// This is necessary if CONSOLIDATE_MOTION in Fl_x.cxx is turned off:
		// if (damage()) return 1; // don't fall behind
	case FL_RELEASE: {
		if (!sdrag) return 0; // should not happen
		Fl_Widget* r = resizable(); if (!r) r = this;
		int newx;
		if (sdrag&DRAGH) {
			newx = Fl::event_x()-sdx;
			if (newx < r->x()) newx = r->x();
			else if (newx >= r->x()+r->w()) newx = r->x()+r->w();
		} else
			newx = sx;
		int newy;
		if (sdrag&DRAGV) {
			newy = Fl::event_y()-sdy;
			if (newy < r->y()) newy = r->y();
			else if (newy >= r->y()+r->h()) newy = r->y()+r->h();
		} else
			newy = sy;
		position(sx,sy,newx,newy);
		if (event == FL_DRAG) set_changed();
		do_callback();
		return 1;}

	}

	return Fl_Group::handle(event);
}

Fl_Tile_Check::Fl_Tile_Check(int x, int y, int w, int h, const char* l)
	: Tile_(x, y, w, h, l)
{
	remove_checks();
}

int Fl_Tile_Check::handle(int event)
{
	switch (event) {
	case FL_DRAG:
		return 1;
	case FL_RELEASE:
		if (!do_checks(Fl::event_x() - xstart, Fl::event_y() - ystart))
			return 1;
		// fall through to reset [xy]start
	case FL_PUSH:
		xstart = Fl::event_x();
		ystart = Fl::event_y();
	}

	return Tile_::handle(event);
}

void Fl_Tile_Check::add_resize_check(resize_check_func f, void *a)
{
	for (size_t i = 0; i < sizeof(resize_checks) / sizeof(resize_checks[0]); i++) {
		if (resize_checks[i] == 0) {
			resize_checks[i] = f;
			resize_args[i] = a;
			break;
		}
	}
}
void Fl_Tile_Check::remove_resize_check(resize_check_func f, void *a)
{
	for (size_t i = 0; i < sizeof(resize_checks) / sizeof(resize_checks[0]); i++)
		if (resize_checks[i] == f && resize_args[i] == a)
			resize_checks[i] = 0;
}
void Fl_Tile_Check::remove_checks(void)
{
	for (size_t i = 0; i < sizeof(resize_checks) / sizeof(resize_checks[0]); i++) {
		resize_checks[i] = 0;
		resize_args[i] = 0;
	}
}
bool Fl_Tile_Check::do_checks(int xd, int yd)
{
	for (size_t i = 0; i < sizeof(resize_checks) / sizeof(resize_checks[0]); i++)
		if (resize_checks[i] && !resize_checks[i](resize_args[i], xd, yd))
			return false;
	return true;
}
