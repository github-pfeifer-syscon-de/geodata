/*
 * Copyright (C) 2024 RPf <gpl3@pfeifer-syscon.de>
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

#include <glibmm.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <iterator>

#include "GeoJson2.hpp"
#include "GeoKlm.hpp"

static std::string
indent_n(uint32_t indent)
{
    std::string ret;
    ret.resize(indent * 3, ' ');
    return ret;
}

static void
printGeo(std::shared_ptr<psc::geo::Geometry> geom, uint32_t indent)
{

    auto mpolyGeo = std::dynamic_pointer_cast<psc::geo::MultiPolygon>(geom);
    if (mpolyGeo != nullptr) {
        auto polys = mpolyGeo->getPolygons();
        std::cout << indent_n(indent) << "MultiPolyGeo " << polys.size() << std::endl;
        for (auto& poly : polys) {
            printGeo(poly, indent + 1);
        }
    }
    auto polyGeo = std::dynamic_pointer_cast<psc::geo::Polygon>(geom);
    if (polyGeo != nullptr) {
        auto segs =polyGeo->getSegments();
        std::cout << indent_n(indent) << "PolyGeo " << segs.size() << std::endl;
        for (auto& seg : segs) {
            printGeo(seg, indent + 1);
        }
    }
    auto segGeo  = std::dynamic_pointer_cast<psc::geo::Segment>(geom);
    if (segGeo != nullptr) {
        auto cords = segGeo->getCoordinates();
        std::cout << indent_n(indent) << "SegGeo " << cords.size() << std::endl;
    }
    auto pntGeo = std::dynamic_pointer_cast<psc::geo::Point>(geom);
    if (pntGeo != nullptr) {
        std::cout << indent_n(indent) << "PntGeo" << std::endl;
    }
}

static bool
jsonReadTest(const std::string& geoJsonFile)
{
    JsonHelper jsonHelper;
    jsonHelper.load_from_file(geoJsonFile);
    psc::geo::GeoJson2 geoJson;
    auto feats = geoJson.read(jsonHelper);
    std::cout << "Geo features " << feats.size() << std::endl;
    for (auto& feat : feats) {
        auto props = feat->getProperties();
        auto type = props->getType("name");
        if (type == psc::geo::ValueType::String) {
            std::cout << "Geo feature " << props->getString("name") << std::endl;
        }
        auto geo = feat->getGeometry();
        printGeo(geo, 1);

    }

    return true;
}


static bool
kmlReadTest(const std::string& kmlFile)
{
    psc::geo::GeoKlm geoKlm;
    geoKlm.read(kmlFile);

    return true;
}


int
main(int argc, char** argv) {
    std::setlocale(LC_ALL, "");      // use locale formating
    Glib::init();
    Gio::init();
    if (argc >= 2) {
        for (int32_t i = 1; i < argc; i++) {
            std::string name{argv[i]};
            if (name.find(".json") != name.npos) {
                if (!jsonReadTest(name)) {
                    return 1;
                }
            }
            else {
                if (!kmlReadTest(name)) {
                    return 1;
                }
            }
        }
    }
    else {
        std::cout << "Please provide a geo.json test file." << std::endl;
    }

    return 0;
}

