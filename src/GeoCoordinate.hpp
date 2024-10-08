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
#include <cmath>
#include <charconv>
#include <GenericGlmCompat.hpp> // providers pi constants for windows

class CoordRefSystem {
public:
    enum Value {
    None,
    CRS_84,    // noted in degree lon,lat
    EPSG_4326, // noted in degree lat,lon
    EPSG_3857  // noted in m lon,lat (web-mercator)
    };
    CoordRefSystem() = default;
    constexpr CoordRefSystem(Value refSysValue)
    : m_value(refSysValue)
    {
    }
    // Allow switch and comparisons. (we can have this or operator==)
    //constexpr operator Value() const
    //{
    //    return m_value;
    //}

    // any valid coord reference system
    constexpr operator bool() const
    {
        return m_value != None;
    }
    constexpr bool operator==(CoordRefSystem a) const
    {
        return m_value == a.m_value;
    }
    constexpr bool operator==(CoordRefSystem::Value val) const
    {
        return m_value == val;
    }
    constexpr bool operator!=(CoordRefSystem a) const
    {
        return m_value != a.m_value;
    }
    constexpr bool operator!=(CoordRefSystem::Value val) const
    {
        return m_value != val;
    }
    double toLinearLon(double value) const;
    double toLinearLat(double value) const;
    double fromLinearLon(double value) const;
    double fromLinearLat(double value) const;

    Glib::ustring identifier() const;
    static CoordRefSystem parse(const Glib::ustring& ref);
    bool is_latitude_first() const;
    static constexpr auto EPSG3857_MIN{-M_PI * 6378137.0};
    static constexpr auto EPSG3857_MAX{M_PI * 6378137.0};
    friend std::ostream& operator<<(std::ostream& os, const CoordRefSystem& myRef) {
        os << myRef.identifier();
        return os;
    }

protected:
    static constexpr auto CRS_84_ID{"CRS:84"};
    static constexpr auto EPSG_4326_ID{"EPSG:4326"};
    static constexpr auto EPSG_3857_ID{"EPSG:3857"};
    static constexpr auto NONE_ID{"none"};

private:
    Value m_value{None};
};

class GeoCoordinate
{
public:
    GeoCoordinate() = default;
    GeoCoordinate(double lon, double lat, CoordRefSystem coordRefSys);
    GeoCoordinate(const GeoCoordinate& orig) = default;
    virtual ~GeoCoordinate() = default;

    double parseLatitude(const Glib::ustring& lat);
    double parseLongitude(const Glib::ustring& lon);
    double getLatitude() const;
    double getLongitude() const;
    void setLatitude(double lat);
    void setLongitude(double lon);
    void setCoordRefSystem(CoordRefSystem coordRef);
    CoordRefSystem getCoordRefSystem() const;
    Glib::ustring printValue(char separator = ',') const;
    GeoCoordinate convert(CoordRefSystem to) const;
    double getLinearLatitude() const;
    double getLinearLongitude() const;
    friend std::ostream& operator<<(std::ostream& os, const GeoCoordinate& myRef) {
        os << " lon "  << myRef.m_longitude << " lat " << myRef.m_latitude << myRef.m_coordRef;
        return os;
    }
    // parse by c-locale
    static double parseDouble(const Glib::ustring& sval);
    // format by c-locale
    static Glib::ustring formatDouble(double val, std::chars_format fmt = std::chars_format::fixed, int precision = 4);

private:
    double m_longitude{0.0};
    double m_latitude{0.0};
    CoordRefSystem m_coordRef;
};

class GeoBounds
{
public:
    GeoBounds() = default;
    GeoBounds(double westLon, double southLat, double eastLon, double northLat, CoordRefSystem coordRefSys);
    GeoBounds(const GeoCoordinate& westSouthLat, const GeoCoordinate& eastNorth);
    GeoBounds(const GeoBounds& orig) = default;
    virtual ~GeoBounds() = default;

    Glib::ustring printValue(char separator = ',') const;
    GeoBounds convert(CoordRefSystem to) const;
    GeoCoordinate& getWestSouth();
    GeoCoordinate& getEastNorth();

private:
    GeoCoordinate m_westSouth;
    GeoCoordinate m_eastNorth;
};

