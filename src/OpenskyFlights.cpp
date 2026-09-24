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

#include <iostream>
#include <JsonHelper.hpp>

#include "Spoon.hpp"
#include "OpenskyFlights.hpp"


OpenskyRequest::OpenskyRequest()
: SpoonMessageStream("https://opensky-network.org", "api/states/all")
{
}

OpenskyFlight::OpenskyFlight()
: Flight()
{
}

bool
OpenskyFlight::isValue(JsonArray* state, guint idx, guint stateLen)
{
    return idx < stateLen && !static_cast<bool>(json_array_get_null_element(state, idx));
}

double
OpenskyFlight::getDouble(JsonArray* state, guint idx, guint stateLen)
{
    if (isValue(state, idx, stateLen)) {
        const auto alt = json_array_get_double_element(state, idx);
        return alt;
    }
    return 0.0;
}

std::string
OpenskyFlight::getString(JsonArray* state, guint idx, guint stateLen)
{
    if (isValue(state, idx, stateLen)) {
        auto str = json_array_get_string_element(state, idx);
        if (str != nullptr) {
            return str;
        }
    }
    return "";
}


void
OpenskyFlight::parse(JsonArray* state)
{
    const auto stateLen = json_array_get_length(state);
    setIcao24( json_array_get_string_element(state, IDX_ICAO24));
    setCallsign(getString(state, IDX_CALLSIGN, stateLen));
    setOriginCountry(getString(state, IDX_COUNTRY, stateLen));
    if (isValue(state, IDX_TIME_POS, stateLen)) {
        const auto time = json_array_get_int_element(state, IDX_TIME_POS);
        setTimePosition(Glib::DateTime::create_now_utc(time));
    }
    if (isValue(state, IDX_CONTACT, stateLen)) {
        auto contact= json_array_get_int_element(state, IDX_CONTACT);
        setLastContact(Glib::DateTime::create_now_utc(contact));
    }
    if (isValue(state, IDX_LONGITUDE, stateLen)
     && isValue(state, IDX_LATITUDE, stateLen)) {
        const auto lon = json_array_get_double_element(state, IDX_LONGITUDE);
        const auto lat = json_array_get_double_element(state, IDX_LATITUDE);
        GeoCoordinate coord = GeoCoordinate(lon, lat, CoordRefSystem(CoordRefSystem::Value::CRS_84));
        setPosition(coord);
    }
    setBaroAltitude(getDouble(state, IDX_BARO_ALTITUDE, stateLen));
    const auto ground = isValue(state, IDX_ON_GROUND, stateLen);
    setOnGround(ground);
    setVelocity(getDouble(state, IDX_VELOCITY, stateLen));
    setTrack(getDouble(state, IDX_TRACK, stateLen));
    setVerticalRate(getDouble(state, IDX_VERTICAL_RATE, stateLen));
    setGeoAltitude(getDouble(state, IDX_GEO_ALTITUDE, stateLen));
    setSquake(getString(state, IDX_SQUAKE, stateLen));
    if (isValue(state, IDX_SPI, stateLen)) {
        setSpecialPurposeIndicator(json_array_get_boolean_element(state, IDX_SPI));
    }
    if (isValue(state, IDX_POSITION_SRC, stateLen)) {
        setPositonSource(static_cast<int>(json_array_get_int_element(state, IDX_POSITION_SRC)));
    }
    if (isValue(state, IDX_CATEGORY, stateLen)) {
        setCategory(static_cast<int>(json_array_get_int_element(state, IDX_CATEGORY)));
    }

}


OpenskyFlights::OpenskyFlights(FlightsConsumer* flightsConsumer)
: Flights(flightsConsumer)
{
}

void
OpenskyFlights::query(GeoBounds& bounds)
{
    auto now =Glib::DateTime::create_now_local();
    setLastQuery(now);
    const auto& westSouth = bounds.getWestSouth();
    const auto& eastNorth = bounds.getEastNorth();
    auto req = std::make_shared<OpenskyRequest>();
    req->addQuery("lamin", GeoCoordinate::formatDouble(westSouth.getLatitude()));
    req->addQuery("lomin", GeoCoordinate::formatDouble(westSouth.getLongitude()));
    req->addQuery("lamax", GeoCoordinate::formatDouble(eastNorth.getLatitude()));
    req->addQuery("lomax", GeoCoordinate::formatDouble(eastNorth.getLongitude()));
    req->signal_receive().connect(sigc::mem_fun(*this, &OpenskyFlights::notify));
    getSpoonSession()->send(req);
}

void
OpenskyFlights::notify(const Glib::ustring& error, int status, SpoonMessageStream* message)
{
    if (m_flightsConsumer != nullptr) {
        if (error != "" || status != SpoonMessage::OK) {
            m_flightsConsumer->notifyError(error, status);
        }
        else {
            auto openskyRequest = dynamic_cast<OpenskyRequest*>(message);
            if (openskyRequest != nullptr) {
                std::list<PtrFlight> flights;
                try {
                    JsonHelper jsonHelper;
                    auto strm = openskyRequest->get_stream();
                    jsonHelper.load_data(strm);
                    //auto parser = json_parser_new();
                    //GError *error{};
                    //GCancellable cancel;  will not be cancelable
                    //json_parser_load_from_stream(parser, strm, nullptr, &error);
                    //if (error != nullptr) {
                    //JsonNode* rootNode = json_parser_get_root(parser);
                    //JsonObject* rootObj = json_node_get_object(rootNode);
                    JsonObject* rootObj = jsonHelper.get_root_object();
                    auto states = json_object_get_array_member(rootObj, "states");
                    int statesLen = json_array_get_length(states);
                    for (int iState = 0; iState < statesLen; ++iState) {
                        JsonArray* state = json_array_get_array_element(states, iState);
                        auto flight = std::make_shared<OpenskyFlight>();
                        flight->parse(state);
                        flights.emplace_back(std::move(flight));
                    }
                    m_flightsConsumer->update(flights);
                }
                catch (const JsonException& ex) {
                    auto msg = Glib::ustring::sprintf("Unable to parse flight data %s", ex.what());
                    m_flightsConsumer->notifyError(msg, 0);
                }
            }
            else {
                std::cout << "OpenskyFlights::notify"
                          << " errror " << error
                          << " status " << status
                          << " no opensky req!" << std::endl;
            }
        }
    }
    else {
        std::cout << "OpenskyFlights::notify"
                  << " errror " << error
                  << " status " << status
                  << " no consumer!" << std::endl;
    }

}


