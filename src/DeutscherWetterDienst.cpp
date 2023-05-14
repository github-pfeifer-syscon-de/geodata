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

#include "DeutscherWetterDienst.hpp"
#include "LocaleContext.hpp"

DeutscherWetterDienst::DeutscherWetterDienst(WeatherConsumer* consumer)
: WebMapService(consumer)
{
}


Glib::ustring
DeutscherWetterDienst::get_base_url()
{
    return BASEURL;
}

Glib::ustring
DeutscherWetterDienst::get_path()
{
    return PATH;
}
