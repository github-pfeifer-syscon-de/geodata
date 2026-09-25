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

#include "Flights.hpp"
#include "OpenskyFlights.hpp"
#include "Spoon.hpp"

Flights::Flights(FlightsConsumer* flightsConsumer)
: m_flightsConsumer{flightsConsumer}
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
Flights::getService(const std::string& service, FlightsConsumer* flightsConsumer)
{
    if (service == OpenskyFlights::SERVICE_NAME) {
        return std::make_shared<OpenskyFlights>(flightsConsumer);
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
