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

#include "GeoCoordinate.hpp"
#include "LocaleContext.hpp"

GeoCoordinate::GeoCoordinate(double lon, double lat, CoordRefSystem coordRefSys)
: m_longitude{lon}
, m_latitude{lat}
, m_coordRef{coordRefSys}
{
}

double
GeoCoordinate::parseLatitude(const Glib::ustring& lat)
{
    LocaleContext localectx(LC_NUMERIC);
    m_latitude = localectx.parseDouble(LocaleContext::en_US, lat);
    return m_latitude;
}

double
GeoCoordinate::parseLongitude(const Glib::ustring& lon)
{
    LocaleContext localectx(LC_NUMERIC);
    m_longitude = localectx.parseDouble(LocaleContext::en_US, lon);
    return m_longitude;
}

Glib::ustring
GeoCoordinate::printValue(char separator)
{
    double first;
    double second;
    switch (m_coordRef) {
    case CoordRefSystem::EPSG_4326:
        first = m_latitude;
        second = m_longitude;
        break;
    case CoordRefSystem::CRS_84:
    case CoordRefSystem::None:
    default:
        first =  m_longitude;
        second = m_latitude;
        break;
    }
    LocaleContext localectx(LC_NUMERIC);
    if (localectx.set(LocaleContext::en_US)) {
        return Glib::ustring::sprintf("%.3f%c%.3f"
                , first, separator, second);
    }
    return Glib::ustring::sprintf("%d%c%d"
                , (int)first, separator, (int)second);
}

void
GeoCoordinate::setCoordRefSystem(CoordRefSystem coordRef)
{
    m_coordRef = coordRef;
}

CoordRefSystem
GeoCoordinate::getCoordRefSystem()
{
    return m_coordRef;
}

double
GeoCoordinate::getLatitude()
{
    return m_latitude;
}

double
GeoCoordinate::getLongitude()
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

CoordRefSystem
GeoCoordinate::parseRefSystem(const Glib::ustring& ref)
{
    auto refUp = ref.uppercase();
    if (refUp == CRS_84) {
        return CoordRefSystem::CRS_84;
    }
    if (refUp == EPSG_4326) {
        return CoordRefSystem::EPSG_4326;
    }
    return CoordRefSystem::None;
}

Glib::ustring
GeoCoordinate::identRefSystem(CoordRefSystem coordRefSys)
{
    switch (coordRefSys)     {
    case CoordRefSystem::CRS_84:
        return CRS_84;
    case CoordRefSystem::EPSG_4326:
        return EPSG_4326;
    default:
        return NONE;
    }
}