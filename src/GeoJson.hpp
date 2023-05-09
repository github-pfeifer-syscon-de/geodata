/*
 * Copyright (C) 2023 RPf <gpl3@pfeifer-syscon.de>
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

#include <glibmm.h>

#include "JsonHelper.hpp"

class Geometry;

#undef GEO_DEBUG

class GeoJson
{
public:
    GeoJson();
    GeoJson(const GeoJson& orig) = delete;
    virtual ~GeoJson();

    void read(const Glib::ustring& file, Geometry* geom);
    void set_radius(double geomRadius);

    static const goffset GEO_FILE_SIZE_LIMIT;
    static constexpr int JSON_POINT_LIMIT = 65535;  // as we fixed index for geometry to ushort
protected:
    void read_multi_polygon(JsonHelper& parser, JsonArray* coord);
    void read_polygon(JsonHelper& parser, JsonArray* coord);

private:
    Geometry* m_geometry;
    int m_count{0};
    double m_geomRadius{30.0 + 0.01};

};

