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

#include "Flight.hpp"


std::string
Flight::getIcao24()
{
    return m_icao24;
}

void
Flight::setIcao24(std::string icao24)
{
    m_icao24 = icao24;
}

std::string
Flight::getCallsign()
{
    return m_callsign;
}

void
Flight::setCallsign(std::string callsign)
{
    m_callsign = callsign;
}

std::string
Flight::getOriginCountry()
{
    return m_originCountry;
}

void
Flight::setOriginCountry(std::string country)
{
    m_originCountry = country;
}

Glib::DateTime
Flight::getTimePosition()
{
    return m_timePosition;
}

void
Flight::setTimePosition(const Glib::DateTime& timePosition)
{
    m_timePosition = timePosition;
}

Glib::DateTime
Flight::getLastContact()
{
    return m_lastContact;
}

void
Flight::setLastContact(const Glib::DateTime& lastContact)
{
    m_lastContact = lastContact;
}

GeoCoordinate
Flight::getPosition()
{
    return m_position;
}

void
Flight::setPosition(const GeoCoordinate& position)
{
    m_position = position;
}

bool
Flight::isOnGround()
{
    return m_onGround;
}

void
Flight::setOnGround(bool onGround)
{
    m_onGround = onGround;
}

double
Flight::getBaroAltitude()
{
    return m_baroAltitude;
}

void
Flight::setBaroAltitude(double baroAltitude)
{
    m_baroAltitude = baroAltitude;
}

double
Flight::getVelocity()
{
    return m_velocity;
}

void
Flight::setVelocity(double velocity)
{
    m_velocity = velocity;
}

double
Flight::getTrack()
{
    return m_track;
}

void
Flight::setTrack(double track)
{
    m_track = track;
}

double
Flight::getVerticalRate()
{
    return m_verticalRate;
}

void
Flight::setVerticalRate(double verticalRate)
{
    m_verticalRate = verticalRate;
}

double
Flight::getGeoAltitude()
{
    return m_geoAltitude;
}

void
Flight::setGeoAltitude(double geoAltitude)
{
    m_geoAltitude = geoAltitude;
}

std::string
Flight::getSquake()
{
    return m_squake;
}

void
Flight::setSquake(const std::string& squake)
{
    m_squake = squake;
}

bool
Flight::isSpecialPurposeIndicator()
{
    return m_specialPurposeIndicator;
}

void
Flight::setSpecialPurposeIndicator(bool spi)
{
    m_specialPurposeIndicator = spi;
}

int
Flight::getPositonSource()
{
    return m_positonSource;
}

void
Flight::setPositonSource(int posSrc)
{
    m_positonSource = posSrc;
}

std::string
Flight::getPositonSourceName()
{
    switch (m_positonSource) {
    case 0:
        return "ADS-B";
    case 1:
        return "ASTERIX";
    case 2:
        return "MLAT";
    case 3:
        return "FLARM";
    default:
        return "position source " + std::to_string(m_positonSource);
    }
}

int
Flight::getCategory()
{
    return m_category;
}

void
Flight::setCategory(int category)
{
    m_category = category;
}

std::string
Flight::getCategoryName()
{
    switch (m_category) {
    case 0:
        return "No information at all";
    case 1:
        return "No ADS-B Emitter Category Information";
    case 2:
        return "Light (< 15500 lbs)";
    case 3:
        return "Small (15500 to 75000 lbs)";
    case 4:
        return "Large (75000 to 300000 lbs)";
    case 5:
        return "High Vortex Large (aircraft such as B-757)";
    case 6:
        return "Heavy (> 300000 lbs)";
    case 7:
        return "High Performance (> 5g acceleration and 400 kts)";
    case 8:
        return "Rotorcraft";
    case 9:
        return "Glider / sailplane";
    case 10:
        return "Lighter-than-air";
    case 11:
        return "Parachutist / Skydiver";
    case 12:
        return "Ultralight / hang-glider / paraglider";
    case 13:
        return "Reserved";
    case 14:
        return "Unmanned Aerial Vehicle";
    case 15:
        return "Space / Trans-atmospheric vehicle";
    case 16:
        return "Surface Vehicle – Emergency Vehicle";
    case 17:
        return "Surface Vehicle – Service Vehicle";
    case 18:
        return "Point Obstacle (includes tethered balloons)";
    case 19:
        return "Cluster Obstacle";
    case 20:
        return "Line Obstacle";
    default:
        return "category " + std::to_string(m_category);
    }
}
