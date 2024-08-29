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

#include <iostream>
#include <array>
#include <Log.hpp>

#include "GeoCoordinate.hpp"
#include "MapProjection.hpp"

bool
CoordRefSystem::is_latitude_first() const
{
    bool latitude_first = false;
    if (m_value == EPSG_4326) {
        latitude_first = true;
    }
    return latitude_first;
}

CoordRefSystem
CoordRefSystem::parse(const Glib::ustring& ref)
{
    CoordRefSystem coordRefSystem;
    auto refUp = ref.uppercase();
    if (refUp == CRS_84_ID) {
        coordRefSystem = CRS_84;
    }
    else if (refUp == EPSG_4326_ID) {
        coordRefSystem = EPSG_4326;
    }
    //else if (refUp == EPSG_3857_ID) { // this is not yet well supported
    //    coordRefSystem = EPSG_3857;
    //}
    return coordRefSystem;
}

Glib::ustring
CoordRefSystem::identifier() const
{
    switch (m_value)     {
    case CRS_84:
        return CRS_84_ID;
    case EPSG_4326:
        return EPSG_4326_ID;
    case EPSG_3857:
        return EPSG_3857_ID;
    default:
        return NONE_ID;
    }
}

double
CoordRefSystem::toLinearLon(double lon) const
{
    switch (m_value) {
        case CRS_84:
        case EPSG_4326:
            return lon / 180.0;
        case EPSG_3857:
            return lon / EPSG3857_MAX;
        default:
            return lon;
    }
}

double
CoordRefSystem::toLinearLat(double lat) const
{
    MapProjectionMercator merc;
    switch (m_value) {
        case CRS_84:
        case EPSG_4326:
            return lat / 90.0;
        case EPSG_3857:
            return merc.toLinearLatitude(lat / EPSG3857_MAX);
        case None:
            return lat;
    }
    return lat;
}

double
CoordRefSystem::fromLinearLon(double relLon) const
{
    switch (m_value) {
        case CRS_84:
        case EPSG_4326:
            return relLon * 180.0;
        case EPSG_3857:
            return relLon * EPSG3857_MAX;
        case None:
            return relLon;
    }
    return relLon;
}

double
CoordRefSystem::fromLinearLat(double relLat) const
{
    MapProjectionMercator merc;
    switch (m_value) {
        case CRS_84:
        case EPSG_4326:
            return relLat * 90.0;
        case EPSG_3857:
            return merc.fromLinearLatitude(relLat) * EPSG3857_MAX;
        case None:
            return relLat;
    }
    return relLat;
}

GeoCoordinate::GeoCoordinate(double lon, double lat, CoordRefSystem coordRefSys)
: m_longitude{lon}
, m_latitude{lat}
, m_coordRef{coordRefSys}
{
}

double
GeoCoordinate::parseDouble(const Glib::ustring& sval)
{
    // glib function is more robust
    double value = Glib::Ascii::strtod(sval);
    return value;
}

Glib::ustring
GeoCoordinate::formatDouble(double val, std::chars_format fmt, int precision)
{
    std::array<char, 64> str;
    auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), val, fmt, precision);
    if (ec == std::errc()) {    // unlikely but check
        Glib::ustring ustr{str.data(), ptr};
        return ustr;
    }
    psc::log::Log::logAdd(Glib::ustring::sprintf("Formating %lf failed ", val));
    return "0";
}

double
GeoCoordinate::parseLatitude(const Glib::ustring& lat)
{
    m_latitude = parseDouble(lat);
    return m_latitude;
}

double
GeoCoordinate::parseLongitude(const Glib::ustring& lon)
{
    m_longitude = parseDouble(lon);
    return m_longitude;
}

Glib::ustring
GeoCoordinate::printValue(char separator) const
{
    double first;
    double second;
    if (m_coordRef.is_latitude_first()) {
        first = m_latitude;
        second = m_longitude;
    }
    else {
        first =  m_longitude;
        second = m_latitude;
    }
    return Glib::ustring::sprintf("%s%c%s"
            , formatDouble(first)
            , separator
            , formatDouble(second));
}

void
GeoCoordinate::setCoordRefSystem(CoordRefSystem coordRef)
{
    m_coordRef = coordRef;
}

CoordRefSystem
GeoCoordinate::getCoordRefSystem() const
{
    return m_coordRef;
}

double
GeoCoordinate::getLatitude() const
{
    return m_latitude;
}

double
GeoCoordinate::getLongitude() const
{
    return m_longitude;
}

void
GeoCoordinate::setLatitude(double lat)
{
    m_latitude = lat;
}

void
GeoCoordinate::setLongitude(double lon)
{
    m_longitude = lon;
}

GeoCoordinate
GeoCoordinate::convert(CoordRefSystem to) const
{
    double linLon = m_coordRef.toLinearLon(m_longitude);
    double linLat = m_coordRef.toLinearLat(m_latitude);
    double toLon = to.fromLinearLon(linLon);
    double toLat = to.fromLinearLat(linLat);
    GeoCoordinate result{toLon, toLat, to};
    return result;
}

double
GeoCoordinate::getLinearLatitude() const
{
    return getCoordRefSystem().toLinearLat(m_latitude);
}

double
GeoCoordinate::getLinearLongitude() const
{
    return getCoordRefSystem().toLinearLon(m_longitude);
}


GeoBounds::GeoBounds(double westLon, double southLat, double eastLon, double northLat, CoordRefSystem coordRefSys)
: m_westSouth{westLon, southLat, coordRefSys}
, m_eastNorth{eastLon, northLat, coordRefSys}
{
}

GeoBounds::GeoBounds(const GeoCoordinate& westSouth, const GeoCoordinate& eastNorth)
: m_westSouth{westSouth}
, m_eastNorth{eastNorth}
{
    if (westSouth.getCoordRefSystem() != eastNorth.getCoordRefSystem()) {
        std::cerr << "GeoBounds::GeoBounds using missmatching coordRefSystem"
                  << " westSouth"  << westSouth.getCoordRefSystem().identifier()
                  << " eastNorth " << eastNorth.getCoordRefSystem().identifier()
                  << std::endl;
    }
}


GeoCoordinate&
GeoBounds::getWestSouth()
{
    return m_westSouth;
}

GeoCoordinate&
GeoBounds::getEastNorth()
{
    return m_eastNorth;
}

Glib::ustring
GeoBounds::printValue(char separator) const
{
    return Glib::ustring::sprintf("%s%c%s"
                , m_westSouth.printValue(separator), separator, m_eastNorth.printValue(separator));
}

GeoBounds
GeoBounds::convert(CoordRefSystem to) const
{
    auto westSouth = m_westSouth.convert(to);
    auto eastNorth = m_eastNorth.convert(to);
    GeoBounds result{westSouth, eastNorth};
    return result;
}
