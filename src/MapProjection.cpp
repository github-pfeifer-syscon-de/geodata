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

#include <glibmm.h>
#include <GenericGlmCompat.hpp> // providers pi constants for windows
#include <cmath>

#include "MapProjection.hpp"

double
MapProjection::normToRadians(double rel)
{
	return rel * M_PI_2;
}


double
MapProjection::radiansToNorm(double rel)
{
	return rel / M_PI_2;
}

// not much to project in this case
double
MapProjection::fromLinearLongitude(double input)
{
    return input;
}

double
MapProjectionMercator::fromLinearLatitude(double input)
{
    auto absinput = std::abs(input);
    double ym = std::log(std::tan(M_PI_4 + normToRadians(absinput) / 2.0));
    if (input < 0.0) {
        ym = -ym;
    }
    return ym / M_PI;   // keep range -1...1
}

double
MapProjectionMercator::toLinearLatitude(double input)
{
    auto absinput = std::abs(input);
    double yr = 2.0 * (std::atan(std::exp(absinput * M_PI)) - M_PI_4);
    if (input < 0.0) {
        yr = -yr;
    }
    return radiansToNorm(yr);   // keep range -1...1
}

