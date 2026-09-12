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

#include <iostream>
#include <thread>
#include <atomic>

#include "../include/BoundsDisplay.hpp"
#include "WeatherConfigGrid.hpp"


BoundsDisplay::BoundsDisplay(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, BaseConfigListener* glSphereView)
: Gtk::DrawingArea(cobject)
{
    m_Dispatcher.connect(sigc::mem_fun(*this, &BoundsDisplay::notify));
    //std::thread t([=] {
        Glib::ustring file(glSphereView->findFile("2k_earth_daymap.jpg"));
        Glib::RefPtr<Gdk::Pixbuf> fullPix = Gdk::Pixbuf::create_from_file(file);
        m_width = fullPix->get_width() / 4;
        m_height = fullPix->get_height() / 4;
        m_pixbuf = fullPix->scale_simple(m_width, m_height,  Gdk::InterpType::INTERP_BILINEAR);
        m_pixbufGuard.store(m_pixbuf.operator bool());
        m_Dispatcher.emit();
    //});
    //t.detach();
}

void
BoundsDisplay::notify()
{
    if (m_width > 0 && m_height > 0) {
        set_size_request(m_width, m_height);
        queue_resize();
    }
    queue_draw();
}

void
BoundsDisplay::setBounds(const GeoBounds& bounds)
{
    m_bounds = bounds;
    queue_draw();
}


double
BoundsDisplay::lon2x(const GeoCoordinate& westSouth)
{
    // -180 ... 180 to 0 ... width
    double lin = westSouth.getCoordRefSystem().toLinearLon(westSouth.getLongitude());
    double full = (lin + 1.0) / 2.0;
    #ifdef BOUNDS_DEBUG
    std::cout << "BoundsDisplay::lon2x"
              << " lon " << westSouth.getLongitude()
              << " full " << full
              << " width " << m_width
              << std::endl;
    #endif
    return full * m_width;
}


double
BoundsDisplay::lat2y(const GeoCoordinate& westSouth)
{
    // -90 ... 90 to height ... 0
    double lin = westSouth.getCoordRefSystem().toLinearLat(westSouth.getLatitude());
    double full = (1.0 - lin) / 2.0;
    #ifdef BOUNDS_DEBUG
    std::cout << "BoundsDisplay::lat2y"
              << " lat " << westSouth.getLatitude()
              << " full " << full
              << " height " << m_height
              << std::endl;
    #endif
    return full * m_height;
}

bool
BoundsDisplay::on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx)
{
    if (m_pixbufGuard.load()
     && m_pixbuf) {
        Gdk::Cairo::set_source_pixbuf(cairoCtx, m_pixbuf, 0, 0);
        cairoCtx->rectangle(0, 0, m_pixbuf->get_width(), m_pixbuf->get_height());
        cairoCtx->fill();
        cairoCtx->set_line_width(1.0);
        GeoCoordinate westSouth = m_bounds.getWestSouth();
        GeoCoordinate eastNorth = m_bounds.getEastNorth();
        double x1 = lon2x(westSouth);
        double y1 = lat2y(westSouth);
        double x2 = lon2x(eastNorth);
        double y2 = lat2y(eastNorth);
        double xmin = std::min(x1, x2);
        double ymin = std::min(y1, y2);
        double xwidth = std::abs(x1 - x2);
        double yheight = std::abs(y1 - y2);
        cairoCtx->set_fill_rule(Cairo::FillRule::FILL_RULE_EVEN_ODD);
        cairoCtx->rectangle(0, 0, m_width, m_height);
        cairoCtx->rectangle(xmin, ymin, xwidth, yheight);
        cairoCtx->set_source_rgba(0.5, 0.5, 0.5, 0.5);
        cairoCtx->fill();
    }
    return true;
}

