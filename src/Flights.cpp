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

#include "Flights.hpp"
#include "OpenskyFlights.hpp"
#include "Spoon.hpp"

Flights::Flights()
{
}

std::shared_ptr<SpoonSession>
Flights::getSpoonSession()
{
    if (!spoonSession) {
        spoonSession = std::make_shared<SpoonSession>("flight private use ");// last ws will add libsoup3
    }
    return spoonSession;
}

std::vector<const char*>
Flights::getServiceNames()
{
    std::vector<const char*> serviceNames;
    serviceNames.push_back(OpenskyFlights::SERVICE_NAME);
    return serviceNames;
}

PtrFlights
Flights::getService(const std::string& service)
{
    if (service == OpenskyFlights::SERVICE_NAME) {
        return std::make_shared<OpenskyFlights>();
    }
    return PtrFlights{};
}

std::chrono::duration<gint64, std::micro>
Flights::asDuration(Glib::TimeSpan& timeSpan)
{
    return std::chrono::duration<gint64, std::micro>(timeSpan);
}

void
Flights::setLastQuery(const Glib::DateTime& lastQuery)
{
    m_lastQuery = lastQuery;
}

Glib::DateTime
Flights::getLastQuery()
{
    return m_lastQuery;
}

void
Flights::addListener(FlightsConsumer* flightsConsumer)
{
    for (auto* consumer : m_flightConsumers) {
        if (consumer == flightsConsumer) {
            return;
        }
    }
    m_flightConsumers.push_back(flightsConsumer);
}
void
Flights::removeListener(FlightsConsumer* flightsConsumer)
{
    for (auto iter = m_flightConsumers.begin(); iter != m_flightConsumers.end(); ) {
        auto consumer = *iter;
        if (consumer == flightsConsumer) {
            iter = m_flightConsumers.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

void
Flights::notifyAll(std::vector<PtrFlight> flights)
{
    if (m_flightConsumers.empty()) {
        std::cout << "Flights::notifyAll"
                  << " flights " << flights.size()
                  << " no one listened!" << std::endl;
        return;
    }
    for (auto* consumer : m_flightConsumers) {
        consumer->update(flights);
    }
}
void
Flights::notifyAll(const Glib::ustring& error, int status)
{
    if (m_flightConsumers.empty()) {
        std::cout << "Flights::notifyAll"
                  << " errror " << error
                  << " status " << status
                  << " no one listened!" << std::endl;
        return;
    }
    for (auto* consumer : m_flightConsumers) {
        consumer->notifyError(error, status);
    }
}