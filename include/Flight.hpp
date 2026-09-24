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

#include "GeoCoordinate.hpp"

#include <glibmm.h>

class Flight {
public:
    Flight() = default;
    explicit Flight(const Flight& other) = delete;
    virtual ~Flight() = default;

    std::string getIcao24()
    {
        return m_icao24;
    }
    void setIcao24(std::string icao24)
    {
        m_icao24 = icao24;
    }
    std::string getCallsign()
    {
        return m_callsign;
    }
    void setCallsign(std::string callsign)
    {
        m_callsign = callsign;
    }
    std::string getOriginCountry()
    {
        return m_originCountry;
    }
    void setOriginCountry(std::string country)
    {
        m_originCountry = country;
    }
    Glib::DateTime getTimePosition()
    {
        return m_timePosition;
    }
    void setTimePosition(const Glib::DateTime& timePosition)
    {
        m_timePosition = timePosition;
    }
    Glib::DateTime getLastContact()
    {
        return m_lastContact;
    }
    void setLastContact(const Glib::DateTime& lastContact)
    {
        m_lastContact = lastContact;
    }
    GeoCoordinate getPosition()
    {
        return m_position;
    }
    void setPosition(const GeoCoordinate& position)
    {
        m_position = position;
    }
    bool isOnGround()
    {
        return m_onGround;
    }
    void setOnGround(bool onGround)
    {
        m_onGround = onGround;
    }

    double getBaroAltitude()
    {
        return m_baroAltitude;
    }
    void setBaroAltitude(double baroAltitude)
    {
        m_baroAltitude = baroAltitude;
    }
    double getVelocity()
    {
        return m_velocity;
    }
    void setVelocity(double velocity)
    {
        m_velocity = velocity;
    }
    double getTrack()
    {
        return m_track;
    }
    void setTrack(double track)
    {
        m_track = track;
    }
    double getVerticalRate()
    {
        return m_verticalRate;
    }
    void setVerticalRate(double verticalRate)
    {
        m_verticalRate = verticalRate;
    }
    double getGeoAltitude()
    {
        return m_geoAltitude;
    }
    void setGeoAltitude(double geoAltitude)
    {
        m_geoAltitude = geoAltitude;
    }
    std::string getSquake()
    {
        return m_squake;
    }
    void setSquake(const std::string& squake)
    {
        m_squake = squake;
    }
    bool isSpecialPurposeIndicator()
    {
        return m_specialPurposeIndicator;
    }
    void setSpecialPurposeIndicator(bool spi)
    {
        m_specialPurposeIndicator = spi;
    }
    int getPositonSource()
    {
        return m_positonSource;
    }
    void setPositonSource(int posSrc)
    {
        m_positonSource = posSrc;
    }
    std::string getPositonSourceName()
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
    int getCategory()
    {
        return m_category;
    }
    void setCategory(int category)
    {
        m_category = category;
    }
    std::string getCategoryName()
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

private:
    std::string m_icao24; // Unique ICAO 24-bit address of the transponder in hex string representation.
    std::string m_callsign;
    std::string m_originCountry;
    Glib::DateTime m_timePosition;
    Glib::DateTime m_lastContact;
    GeoCoordinate m_position;
    bool m_onGround{};
    double m_baroAltitude{};
    double m_velocity{};
    double m_track{};
    double m_verticalRate{};
    double m_geoAltitude{};
    std::string m_squake;
    bool m_specialPurposeIndicator{};
    int m_positonSource{};
    int m_category{};
};



using PtrFlight = std::shared_ptr<Flight> ;