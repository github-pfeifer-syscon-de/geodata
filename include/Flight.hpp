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

    std::string getIcao24();
    void setIcao24(std::string icao24);
    std::string getCallsign();
    void setCallsign(std::string callsign);
    std::string getOriginCountry();
    void setOriginCountry(std::string country);
    Glib::DateTime getTimePosition();
    void setTimePosition(const Glib::DateTime& timePosition);
    Glib::DateTime getLastContact();
    void setLastContact(const Glib::DateTime& lastContact);
    GeoCoordinate getPosition();
    void setPosition(const GeoCoordinate& position);
    bool isOnGround();
    void setOnGround(bool onGround);

    double getBaroAltitude();
    void setBaroAltitude(double baroAltitude);
    double getVelocity();
    void setVelocity(double velocity);
    double getTrack();
    void setTrack(double track);
    double getVerticalRate();
    void setVerticalRate(double verticalRate);
    double getGeoAltitude();
    void setGeoAltitude(double geoAltitude);
    std::string getSquake();
    void setSquake(const std::string& squake);
    bool isSpecialPurposeIndicator();
    void setSpecialPurposeIndicator(bool spi);
    int getPositonSource();
    void setPositonSource(int posSrc);
    std::string getPositonSourceName();
    int getCategory();
    void setCategory(int category);
    std::string getCategoryName();

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