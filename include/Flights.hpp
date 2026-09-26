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

#include <memory>
#include <chrono>
#include <vector>

#include "Flight.hpp"
#include "Spoon.hpp"

class Flights;

using PtrFlights = std::shared_ptr<Flights>;

class FlightsConsumer
{
public:
    virtual void update(std::vector<PtrFlight> flights) = 0;
    virtual void notifyError(const Glib::ustring& error, int status) = 0;
};

class Flights
{
public:
    Flights();
    explicit Flights(const Flights& other) = delete;
    virtual ~Flights() = default;

    virtual void query(GeoBounds& bounds) = 0;
    virtual std::string getServiceName() = 0;
    static std::vector<const char*> getServiceNames();
    // this should probably be followed by addListener
    static std::shared_ptr<Flights> getService(const std::string& service);
    static std::chrono::duration<gint64, std::micro> asDuration(Glib::TimeSpan& timeSpan);
    void setLastQuery(const Glib::DateTime& lastQuery);
    Glib::DateTime getLastQuery();
    void addListener(FlightsConsumer* flightsConsumer);
    void removeListener(FlightsConsumer* flightsConsumer);
protected:
    void notifyAll(std::vector<PtrFlight> flights);
    void notifyAll(const Glib::ustring& error, int status);
    std::shared_ptr<SpoonSession> getSpoonSession();

    std::shared_ptr<SpoonSession> spoonSession;
    Glib::DateTime m_lastQuery;
    std::list<FlightsConsumer*> m_flightConsumers;
};
