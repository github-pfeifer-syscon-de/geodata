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

#include <json-glib/json-glib.h>


#include "Flights.hpp"

class OpenskyRequest
: public SpoonMessageStream
{
public:
    OpenskyRequest();
    explicit OpenskyRequest(const OpenskyRequest& other) = delete;
    virtual ~OpenskyRequest() = default;
};

// Ref https://openskynetwork.github.io/opensky-api/rest.html#all-state-vectors
// icao info infos https://globe.adsbexchange.com/?icao=3c4317
// callsign info https://www.flightaware.com/live/flight/FIN5YP/
class OpenskyFlight
: public Flight {
public:
    OpenskyFlight();
    explicit OpenskyFlight(const OpenskyFlight& other) = delete;
    virtual ~OpenskyFlight() = default;

    void parse(JsonArray* state);

    static constexpr auto IDX_ICAO24{0u};
    static constexpr auto IDX_CALLSIGN{1u};
    static constexpr auto IDX_COUNTRY{2u};
    static constexpr auto IDX_TIME_POS{3u};
    static constexpr auto IDX_CONTACT{4u};
    static constexpr auto IDX_LONGITUDE{5u};
    static constexpr auto IDX_LATITUDE{6u};
    static constexpr auto IDX_BARO_ALTITUDE{7u};
    static constexpr auto IDX_ON_GROUND{8u};
    static constexpr auto IDX_VELOCITY{9u};
    static constexpr auto IDX_TRACK{10u};
    static constexpr auto IDX_VERTICAL_RATE{11u};
    static constexpr auto IDX_GEO_ALTITUDE{13u};
    static constexpr auto IDX_SQUAKE{14u};
    static constexpr auto IDX_SPI{15u};
    static constexpr auto IDX_POSITION_SRC{16u};
    static constexpr auto IDX_CATEGORY{17u};
protected:
    bool isValue(JsonArray* state, guint idx, guint stateLen);
    double getDouble(JsonArray* state, guint idx, guint stateLen);
    std::string getString(JsonArray* state, guint idx, guint stateLen);
};

class OpenskyFlights
: public Flights
{
public:
    OpenskyFlights(FlightsConsumer* flightsConsumer);
    explicit OpenskyFlights(const OpenskyFlights& other) = delete;
    virtual ~OpenskyFlights() = default;

    void query(GeoBounds& bounds) override;
    void notify(const Glib::ustring& error, int status, SpoonMessageStream* message);


    static constexpr auto SERVICE_NAME = "Opensky";
};
