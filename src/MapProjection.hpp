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

class MapProjection
{
public:
    MapProjection() = default;
    virtual ~MapProjection() = default;

    // expect some normalized value -1...1 and returns -1...1
    //   for the respective projection
    virtual double fromLinearLatitude(double rel) = 0;
    virtual double toLinearLatitude(double rel) = 0;
    double fromLinearLongitude(double input);
protected:
    double normToRadians(double norm);
    double radiansToNorm(double rel);

};

// Simplified (web) mercator
class MapProjectionMercator
: MapProjection
{
public:
    MapProjectionMercator() = default;
    virtual ~MapProjectionMercator() = default;

    double fromLinearLatitude(double rel) override;
    double toLinearLatitude(double rel) override;
};