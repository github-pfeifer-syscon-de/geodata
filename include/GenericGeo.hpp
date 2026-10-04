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

#include <GeoCoordinate.hpp>
#include <memory>
#include <vector>


namespace psc::geo {


enum class ValueType {
    Null
   ,Integer
   ,Double
   ,Boolean
   ,String
   ,
};

class PropValueBase
{
public:
    PropValueBase(ValueType type)
    : m_type{type}
    {
    }
    explicit PropValueBase(const PropValueBase& value) = default;
    virtual ~PropValueBase() = default;
    ValueType getValueType() {
        return m_type;
    }
protected:
    ValueType m_type;
};

template <typename T>
class PropValue
: public PropValueBase
{
public:
    PropValue(const T& value, ValueType type)
    : PropValueBase{type}
    , m_value{value}
    {
    }
    explicit PropValue(const PropValue& value) = default;
    virtual ~PropValue() = default;
    T get() {
        return m_value;
    }
protected:
    T m_value;
};

class PropValueInt
: public PropValue<int64_t>
{
public:
    explicit PropValueInt(int64_t value)
    : PropValue{value, ValueType::Integer}
    {
    }
    explicit PropValueInt(const PropValueInt& value) = default;
    virtual ~PropValueInt() = default;
};

class PropValueDouble
: public PropValue<double>
{
public:
    PropValueDouble(double value)
    : PropValue{value, ValueType::Double}
    {
    }
    explicit PropValueDouble(const PropValueDouble& value) = default;
    virtual ~PropValueDouble() = default;
};

class PropValueBool
: public PropValue<bool>
{
public:
    PropValueBool(bool value)
    : PropValue{value, ValueType::Boolean}
    {
    }
    explicit PropValueBool(const PropValueBool& value) = default;
    virtual ~PropValueBool() = default;
};

class PropValueString
: public PropValue<Glib::ustring>
{
public:
    PropValueString(const Glib::ustring& value)
    : PropValue{value, ValueType::String}
    {
    }
    explicit PropValueString(const PropValueString& value) = default;
    virtual ~PropValueString() = default;
};

class Properties
{
public:
    Properties() = default;
    explicit Properties(const Properties& other) = delete;
    virtual ~Properties() = default;

    ValueType getType(const Glib::ustring& key);
    int64_t getInteger(const Glib::ustring& key);
    double getDouble(const Glib::ustring& key);
    bool getBool(const Glib::ustring& key);
    Glib::ustring getString(const Glib::ustring& key);

protected:
    std::map<Glib::ustring, std::unique_ptr<PropValueBase>> m_properties;
};

using PtrProperties = std::shared_ptr<Properties>;

class Geometry
{
public:
    Geometry() = default;
    explicit Geometry(const Geometry& other) = delete;
    virtual ~Geometry() = default;
protected:

};

using PtrGeometry = std::shared_ptr<Geometry>;


class Point
: public Geometry
{
public:
    Point() = default;
    explicit Point(const Point& other) = delete;
    virtual ~Point() = default;

    GeoCoordinate getCoordinate() const {
        return m_coord;
    }
protected:
    GeoCoordinate m_coord;
};

using PtrPoint = std::shared_ptr<Point>;

class Segment
: public Geometry
{
public:
    Segment() = default;
    explicit Segment(const Segment& other) = delete;
    virtual ~Segment() = default;

    std::vector<GeoCoordinate> getCoordinates(){
        return m_coords;
    }
protected:
    std::vector<GeoCoordinate> m_coords;
};

using PtrSegment = std::shared_ptr<Segment>;

class Polygon
: public Geometry
{
public:
    Polygon() = default;
    explicit Polygon(const Polygon& other) = delete;
    virtual ~Polygon() = default;

    std::vector<PtrSegment> getSegments() {
        return m_segments;
    }
protected:
    std::vector<PtrSegment> m_segments;
};

using PtrPolygon = std::shared_ptr<Polygon>;

class MultiPolygon
: public Geometry
{
public:
    MultiPolygon() = default;
    explicit MultiPolygon(const MultiPolygon& other) = delete;
    virtual ~MultiPolygon() = default;

    std::vector<PtrPolygon> getPolygons() {
        return m_polygons;
    }
protected:
    std::vector<PtrPolygon> m_polygons;
};

using PtrMultiPolygon = std::shared_ptr<MultiPolygon>;

class Feature
{
public:
    Feature() = default;
    explicit Feature(const Feature& other) = delete;
    virtual ~Feature() = default;

    virtual PtrProperties getProperties()  = 0;
    virtual PtrGeometry getGeometry() = 0;
};

using PtrFeature = std::shared_ptr<Feature>;

} /* namespace psc::geo */