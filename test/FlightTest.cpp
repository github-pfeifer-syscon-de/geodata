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

#include "GeoCoordinate.hpp"
#include "FlightTest.hpp"
#include "OpenskyFlights.hpp"

FlightTest::FlightTest()
: Gio::Application("de.pfeifer_syscon.flightTest")
{
}


void
FlightTest::start()
{
    m_opensky = Flights::getService(OpenskyFlights::SERVICE_NAME, this);
    GeoBounds bounds{ 8, 50, 9, 52, CoordRefSystem(CoordRefSystem::Value::CRS_84)};
    m_opensky->query(bounds);
}

void
FlightTest::update(std::list<PtrFlight> flights)
{
    std::cout << "FlightTest::update size " << flights.size() << std::endl;
    for (PtrFlight flight : flights) {
        std::cout << "icao24 " << flight->getIcao24() << "\n"
                  << "  call " << flight->getCallsign() << "\n"
                  << "  ctry " << flight->getOriginCountry() << "\n"
                  << "  time " << (flight->getTimePosition() ? flight->getTimePosition().format_iso8601() : "-") << "\n"
                  << "  cont " << (flight->getLastContact() ? flight->getLastContact().format_iso8601() : "-") << "\n"
                  << "  ongr " << std::boolalpha << flight->isOnGround() << "\n"
                  << "   pos " << flight->getPosition().getLongitude() << " , " << flight->getPosition().getLatitude() << "\n"
                  << "  balt " << flight->getBaroAltitude() << "m\n"
                  << "   vel " << flight->getVelocity() << "m/s\n"
                  << "   trk " << flight->getTrack() << "°\n"
                  << "  vert " << flight->getVerticalRate() << "m/s\n"
                  << "  galt " << flight->getGeoAltitude() << "m\n"
                  << "  squa " << flight->getSquake() << "\n"
                  << "  spec " << std::boolalpha << flight->isSpecialPurposeIndicator() << "\n"
                  << "  posr " << flight->getPositonSourceName() << "\n"
                  << "   cat " << flight->getCategoryName() << std::endl;
    }
}

void
FlightTest::notifyError(const Glib::ustring& error, int status)
{
    std::cout << "FlightTest::notifyError"
              << " error " << error
              << " status " << status << std::endl;
    m_error = error;
    m_status = status;
}


void
FlightTest::on_activate()
{
    // create main loop so the dispatch works
    auto main = Glib::MainLoop::create(false);
    main->get_context()->signal_idle().connect_once(
            sigc::mem_fun(*this, &FlightTest::start));
    main->get_context()->signal_timeout().connect_seconds_once([&] {
            main->quit();
    }, 5);
    main->run();
}


int
FlightTest::getResult()
{
    if (!m_error.empty() || m_status != 0) {
        return 1;
    }
    return 0;
}


int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!
    Glib::init();
    Gio::init();

    auto app = FlightTest();
    app.run(argc, argv);
    return app.getResult();
}