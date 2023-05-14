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

#include "WebMapService.hpp"

// https://maps.dwd.de/geoserver/ows?service=wms&version=1.3.0&request=GetCapabilities


class DeutscherWetterDienst
: public WebMapService
{
public:
    DeutscherWetterDienst(WeatherConsumer* consumer);
    virtual ~DeutscherWetterDienst() = default;

    Glib::ustring get_base_url() override;
    Glib::ustring get_path() override;

    static constexpr auto PATH{"geoserver/ows"};
    static constexpr auto BASEURL{"https://maps.dwd.de"};
    static constexpr auto NAME{"DeutscherWetterDienst"};
private:

};
