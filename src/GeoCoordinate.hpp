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

enum class CoordRefSystem {
    None,
    CRS_84,
    EPSG_4326
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
    double getLatitude();
    double getLongitude();
    void setLatitude(double lat);
    void setLongitude(double lon);
    void setCoordRefSystem(CoordRefSystem coordRef);
    CoordRefSystem getCoordRefSystem();
    Glib::ustring printValue(char separator = ',');
    static CoordRefSystem parseRefSystem(const Glib::ustring& ref);
    static Glib::ustring identRefSystem(CoordRefSystem coordRefSys);
protected:
    static constexpr auto CRS_84{"CRS:84"};
    static constexpr auto EPSG_4326{"EPSG:4326"};
    static constexpr auto NONE{"none"};
private:
    double m_longitude{0.0};
    double m_latitude{0.0};
    CoordRefSystem m_coordRef{CoordRefSystem::None};
};



