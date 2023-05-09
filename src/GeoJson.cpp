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

#include <glm/trigonometric.hpp>
#include <iostream>

#include "GeoJson.hpp"
#include "Geometry.hpp"

// as we have no Gis limit the complexity of usable files
const goffset GeoJson::GEO_FILE_SIZE_LIMIT{204800};

GeoJson::GeoJson()
{
}


GeoJson::~GeoJson()
{
}

void GeoJson::set_radius(double geomRadius)
{
    m_geomRadius = geomRadius + 0.01;   // increase the radius as we want to draw the shape slightly over the surface (reduce problem of lines that are cut by surface)
}

void
GeoJson::read_polygon(JsonHelper& parser,JsonArray* poly)
{
    Color color(0.6f, 0.6f, 0.6f);
    int polyLen = json_array_get_length(poly);
    #ifdef GEO_DEBUG
    std::cout << "Found polys " << polyLen << std::endl;
    #endif
    for (int iPoly = 0; iPoly < polyLen; ++iPoly) {
        JsonArray* shape = parser.get_array_array(poly, iPoly);
        int shapeLen = json_array_get_length(shape);
        #ifdef GEO_DEBUG
        std::cout << "Found shapes " << shapeLen << std::endl;
        #endif
        bool first = true;
        for (int iShape = 0; iShape < shapeLen; ++iShape) {
            JsonArray* coord = parser.get_array_array(shape, iShape);
            int coordLen = json_array_get_length(coord);
            if (coordLen >= 2) {
                gdouble lon = M_PI + glm::radians(json_array_get_double_element(coord, 0));
                gdouble lat = glm::radians(json_array_get_double_element(coord, 1));
                double const cosLat = std::cos(lat);
                double const sinLat = std::sin(lat);
                double const cosLon = std::cos(lon);
                double const sinLon = std::sin(lon);
                Position pnt;
                pnt.x = (cosLat * sinLon) * m_geomRadius;    // match definition of texture at -180° from greenwich and wrap (date border)
                pnt.y = sinLat * m_geomRadius;
                pnt.z = (cosLat * cosLon)* m_geomRadius;
                m_geometry->addPoint(&pnt, &color, nullptr, nullptr, nullptr, nullptr);
                if (!first) {   // start with each poly
                    m_geometry->addIndex(m_count-1, m_count);    // create connected line
                }
                ++m_count;
                first = false;
            }
            else {
                // keep as warning ?
                std::cout << "Expected coords 2 got " << coordLen << std::endl;
            }
        }
    }

}

void
GeoJson::read_multi_polygon(JsonHelper& parser,JsonArray* multiPoly)
{
    int len = json_array_get_length(multiPoly);
    #ifdef GEO_DEBUG
    std::cout << "Found multi poly " << len << std::endl;
    #endif
    for (int iMultPoly = 0; iMultPoly < len; ++iMultPoly) {
        JsonArray* poly = parser.get_array_array(multiPoly, iMultPoly);
        read_polygon(parser, poly);
    }
}

void
GeoJson::read(const Glib::ustring& file, Geometry* geom)
{
    m_geometry = geom;
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
        JsonObject* geo = parser.get_object(feat, "geometry");
        const gchar* type = json_object_get_string_member(geo, "type");
        JsonArray* coord = parser.get_array(geo, "coordinates");
        if (strcmp("MultiPolygon", type) == 0) {
            read_multi_polygon(parser, coord);
        }
        else if (strcmp("Polygon", type) == 0) {
            read_polygon(parser, coord);
        }
        else {
            std::cout << "The file " << file << " contains an unexpected type " << type << std::endl;
        }
        #ifdef GEO_DEBUG
        std::cout << "Coords created " << m_count << std::endl;
        #endif
    }
    if (m_count > JSON_POINT_LIMIT) {
        throw JsonException(Glib::ustring::sprintf("The file %s contains more points %d (allowed %d) as we can handle", file, m_count, 65535));
    }

}
