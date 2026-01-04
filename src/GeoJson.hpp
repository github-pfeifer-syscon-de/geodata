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

#include "JsonHelper.hpp"

class Geometry;

#undef GEO_DEBUG

// a list of alternating x,y values
typedef std::vector<double> GeoSegment;
typedef std::vector<std::shared_ptr<GeoSegment>> GeoPath;

class GeoJsonHandler
{
public:
    GeoJsonHandler() = default;
    virtual ~GeoJsonHandler() = default;

    virtual void addFeature(JsonObject* feat) = 0;
    virtual void endFeature() = 0;
    virtual void addGeometry(JsonObject* geo) = 0;
    virtual void endGeometry() = 0;
    virtual void addMultiPolygon(JsonArray* multipoly) = 0;
    virtual void endMultiPolygon() = 0;
    virtual void addPolygon(JsonArray* poly) = 0;
    virtual void endPolygon() = 0;
    virtual void addShape(JsonArray* shape) = 0;
    virtual void endShape() = 0;
    virtual void addCoord(JsonArray* coord, bool last) = 0;
};

class GeoJsonVectorHandler
: public GeoJsonHandler
{
public:
    GeoJsonVectorHandler() = default;
    virtual ~GeoJsonVectorHandler() = default;

    void addFeature(JsonObject* feat) override {};
    void endFeature() override {};
    void addGeometry(JsonObject* geo) override {};
    void endGeometry() override {};
    void addMultiPolygon(JsonArray* multipoly) override {};
    void endMultiPolygon() override {};
    void addPolygon(JsonArray* poly) override {};
    void endPolygon() override {};
    void addShape(JsonArray* shape) override;
    void endShape() override;
    void addCoord(JsonArray* coord, bool last) override;
    GeoPath& getPath();
    int getPointsLimit();
    void setPointsLimit(int points);
private:
    bool m_first{true};
    int m_count{0};
    static constexpr auto DEFAULT_JSON_POINT_LIMIT{65535};  // as we fixed index for geometry to ushort
    int m_pointsLimit{DEFAULT_JSON_POINT_LIMIT};

    std::shared_ptr<GeoSegment> m_segment;
    GeoPath m_path;
};


class GeoJson
{
public:
    GeoJson() = default;
    explicit GeoJson(const GeoJson& orig) = delete;
    virtual ~GeoJson() = default;

    void read(const Glib::ustring& file, GeoJsonHandler* handler);
protected:
    void read_multi_polygon(JsonHelper& parser, JsonArray* coord, GeoJsonHandler* handler);
    void read_polygon(JsonHelper& parser, JsonArray* coord, GeoJsonHandler* handler);

private:
};

