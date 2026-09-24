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

#pragma once

#include <glibmm.h>
#include <giomm.h>
#include <memory>

#include "Flights.hpp"

class FlightTest
: public Gio::Application, FlightsConsumer
{
public:
    FlightTest();
    explicit FlightTest(const FlightTest& other) = delete;
    virtual ~FlightTest() = default;

    void update(std::list<PtrFlight> flights) override;
    void notifyError(const Glib::ustring& error, int status) override;

    void on_activate() override;
    int getResult();
protected:
    void start();
private:
    Glib::ustring m_error;
    int m_status{};
    std::shared_ptr<Flights> m_opensky;
};
