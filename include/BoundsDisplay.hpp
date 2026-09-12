/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2023 RPf 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gtkmm.h>
#include <atomic>
#include <GeoCoordinate.hpp>

class BaseConfigListener;

#undef BOUNDS_DEBUG

class BoundsDisplay
: public Gtk::DrawingArea
{
public:
    BoundsDisplay(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, BaseConfigListener* glSphereView);
    explicit BoundsDisplay(const BoundsDisplay& orig) = delete;
    virtual ~BoundsDisplay() = default;

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx) override;
    void setBounds(const GeoBounds& bounds);
private:
    void notify();
    double lat2y(const GeoCoordinate& westSouth);
    double lon2x(const GeoCoordinate& westSouth);

    Glib::Dispatcher m_Dispatcher; // used for redraw notification
    GeoBounds m_bounds;
    volatile int m_width{0};
    volatile int m_height{0};
    std::atomic<bool> m_pixbufGuard;
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
};

