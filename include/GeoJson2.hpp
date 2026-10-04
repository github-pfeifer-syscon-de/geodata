/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2026 RPf
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

#include <JsonHelper.hpp>
#include <StringUtils.hpp>
#include <iostream>
#include <format>

#include "GenericGeo.hpp"

// this is intended to be a storing reader
//   for just conversions see GeoJson
namespace psc::geo
{

class Json2Exception
: public std::exception
{
public:
    Json2Exception(const std::string& msg)
    : swhat{msg}
    {
    }
    virtual ~Json2Exception() = default;
    virtual const char * what() const noexcept {
        return swhat.c_str();
    }
private:
    std::string swhat;
};

class JsonGeometry
{
public:
    JsonGeometry() = default;
    explicit JsonGeometry(const JsonGeometry& other) = delete;
    virtual ~JsonGeometry() = default;

    GeoCoordinate readCoord(JsonArray* coord, const std::string& ctx);

};

class JsonPoint
: public Point
, public JsonGeometry
{
public:
    JsonPoint(JsonArray* coord, const std::string& ctx);
    explicit JsonPoint(const JsonPoint& other) = delete;
    virtual ~JsonPoint() = default;

};

class JsonSegment
: public Segment
, public JsonGeometry
{
public:
    JsonSegment(JsonArray* segm, const std::string& ctx);
    explicit JsonSegment(const JsonSegment& other) = delete;
    virtual ~JsonSegment() = default;

};

class JsonPolygon
: public Polygon
, public JsonGeometry
{
public:
    JsonPolygon(JsonArray* segm, const std::string& ctx);
    explicit JsonPolygon(const JsonPolygon& other) = delete;
    virtual ~JsonPolygon() = default;


};
class JsonMultiPolygon
: public MultiPolygon
, public JsonGeometry
{
public:
    JsonMultiPolygon(JsonArray* multi, const std::string& ctx);
    explicit JsonMultiPolygon(const JsonMultiPolygon& other) = delete;
    virtual ~JsonMultiPolygon() = default;

    void read();

};

class JsonProperties
: public Properties
{
public:
    JsonProperties(JsonObject* propObj, const std::string& ctx);
};


class JsonFeature
: public Feature
{
public:
    JsonFeature(JsonObject* featObj, const std::string& ctx);
    explicit JsonFeature(const JsonFeature& other) = delete;
    virtual ~JsonFeature() = default;

    PtrProperties getProperties() override
    {
        return m_properties;
    }
    PtrGeometry getGeometry() override
    {
        return m_geometry;
    }
protected:
    PtrGeometry readGeometry(JsonObject* geometry, const std::string& ctx);

    PtrGeometry m_geometry;
    PtrProperties m_properties;
};

using PtrJsonFeature = std::shared_ptr<JsonFeature>;

class GeoJson2 {
public:
    GeoJson2() {
    }
    explicit GeoJson2(const GeoJson2& other) = delete;
    virtual ~GeoJson2() = default;

    std::vector<PtrFeature> read(JsonHelper& helper);
};

} /* namespace psc::geo */