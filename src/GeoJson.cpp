/*
 * Copyright (C) 2023 RPf <gpl3@pfeifer-syscon.de>
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
#include <cmath>

#include "GeoJson.hpp"

int GeoJsonVectorHandler::getPointsLimit()
{
    return m_pointsLimit;
}

void
GeoJsonVectorHandler::setPointsLimit(int points)
{
    m_pointsLimit = points;
}

void
GeoJsonVectorHandler::addShape(JsonArray* shape)
{
    m_first = true;
}

void
GeoJsonVectorHandler::endShape()
{
    if (m_segment) {
        m_path.push_back(std::move(m_segment));
        m_segment.reset();
    }
}

void
GeoJsonVectorHandler::addCoord(JsonArray* coord, bool last)
{
    int coordLen = json_array_get_length(coord);
    if (coordLen >= 2) {
        if (m_first) {
            if (m_segment) {
                m_path.push_back(std::move(m_segment));
            }
            m_segment = std::make_shared<GeoSegment>();
            // the size is varying so reserve will not help
        }
        double lon = json_array_get_double_element(coord, 0);
        double lat = json_array_get_double_element(coord, 1);
        m_segment->push_back(lon);
        m_segment->push_back(lat);
        ++m_count;
        m_first = false;
    }
    else {
        // keep as warning ?
        std::cout << "Expected coords 2 got " << coordLen << std::endl;
    }

}

GeoPath&
GeoJsonVectorHandler::getPath()
{
    if (m_count > m_pointsLimit) {
        throw JsonException(Glib::ustring::sprintf("The file contains more points %d (allowed %d) as we can handle", m_count, m_pointsLimit));
    }
    return m_path;
}


void
GeoJson::read_polygon(JsonHelper& parser, JsonArray* poly, GeoJsonHandler* handler)
{
    handler->addPolygon(poly);
    int polyLen = json_array_get_length(poly);
    #ifdef GEO_DEBUG
    std::cout << "Found polys " << polyLen << std::endl;
    #endif
    for (int iPoly = 0; iPoly < polyLen; ++iPoly) {
        JsonArray* shape = parser.get_array_array(poly, iPoly);
        handler->addShape(shape);
        int shapeLen = json_array_get_length(shape);
        #ifdef GEO_DEBUG
        std::cout << "Found shapes " << shapeLen << std::endl;
        #endif
        for (int iShape = 0; iShape < shapeLen; ++iShape) {
            JsonArray* coord = parser.get_array_array(shape, iShape);
            handler->addCoord(coord, iShape == shapeLen-1);
        }
        handler->endShape();
    }
    handler->endPolygon();
}

void
GeoJson::read_multi_polygon(JsonHelper& parser, JsonArray* multiPoly, GeoJsonHandler* handler)
{
    handler->addMultiPolygon(multiPoly);
    int len = json_array_get_length(multiPoly);
    #ifdef GEO_DEBUG
    std::cout << "Found multi poly " << len << std::endl;
    #endif
    for (int iMultPoly = 0; iMultPoly < len; ++iMultPoly) {
        JsonArray* poly = parser.get_array_array(multiPoly, iMultPoly);
        read_polygon(parser, poly, handler);
    }
    handler->endMultiPolygon();
}

void
GeoJson::read(const Glib::ustring& file, GeoJsonHandler* handler)
{
    JsonHelper parser;
    parser.load_from_file(file);
    JsonObject* rootObj = parser.get_root_object();
    JsonArray* features = parser.get_array(rootObj, "features");
    int featLen = json_array_get_length(features);
    #ifdef GEO_DEBUG
    std::cout << "Found features " << len << std::endl;
    #endif
    for (int iFeat = 0; iFeat < featLen; ++iFeat) {
        JsonObject* feat = parser.get_array_object(features, iFeat);
        handler->addFeature(feat);
        JsonObject* geo = parser.get_object(feat, "geometry");
        handler->addGeometry(geo);
        const gchar* type = json_object_get_string_member(geo, "type");
        JsonArray* coord = parser.get_array(geo, "coordinates");
        if (strcmp("MultiPolygon", type) == 0) {
            read_multi_polygon(parser, coord, handler);
        }
        else if (strcmp("Polygon", type) == 0) {
            read_polygon(parser, coord, handler);
        }
        else {
            std::cout << "The file " << file << " contains an unexpected type " << type << std::endl;
        }
        #ifdef GEO_DEBUG
        std::cout << "Coords created " << m_count << std::endl;
        #endif
        handler->endGeometry();
        handler->endFeature();
    }
}
