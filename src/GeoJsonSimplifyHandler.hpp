/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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
#include <glibmm.h>
#include <vector>
#include <memory>

#include "GeoJson.hpp"

// Helper class to simplify geo-json files
//   the file read with GeoJson will checked
//   for every coordinate and if the distance
//   from last to actual is less than MinDistance
//   it will be skipped on output.
// ?Additional check last three points and if the
//    middle point will be less than MinDistance
//    from a straight line it can be skipped.
// ?Missing function for GlibJson the double
//    output will always use all decimal places
//    which is really bad for output size :(

class GeoJsonSimplifyHandler
: public GeoJsonHandler
{
public:
    GeoJsonSimplifyHandler();
    virtual ~GeoJsonSimplifyHandler() = default;

    void addFeature(JsonObject* feat) override;
    void endFeature() override;
    void addGeometry(JsonObject* geo) override;
    void endGeometry() override;
    void addMultiPolygon(JsonArray* multipoly) override;
    void endMultiPolygon() override;
    void addPolygon(JsonArray* poly) override;
    void endPolygon() override;
    void addShape(JsonArray* shape) override;
    void endShape() override;
    void addCoord(JsonArray* coord, bool last) override;

    void setMinDistance(double minDist);
    double getMinDistance();
    void exportFile(const Glib::ustring& file);

private:
    static constexpr auto DEFAULT_MIN_DISTANCE{0.1};
    double m_min_distance{DEFAULT_MIN_DISTANCE};
    JsonBuilder* m_builder;
    static constexpr auto DEFAULT_START_COORD{-999.};
    double m_lastLon{DEFAULT_START_COORD};
    double m_lastLat{DEFAULT_START_COORD};
};
