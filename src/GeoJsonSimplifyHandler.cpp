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
#include <StringUtils.hpp>

#include "GeoJsonSimplifyHandler.hpp"


void
GeoJsonSimplifyHandler::addFeature(JsonObject* feat)
{
    json_builder_begin_object(m_builder);
    json_builder_set_member_name(m_builder, "type");
    json_builder_add_string_value(m_builder, "Feature");
}

void
GeoJsonSimplifyHandler::endFeature()
{
    json_builder_end_object(m_builder); // feature
}

void
GeoJsonSimplifyHandler::addGeometry(JsonObject* geo)
{
    json_builder_set_member_name(m_builder, "geometry");
    json_builder_begin_object(m_builder);
    const gchar* type = json_object_get_string_member(geo, "type");
    json_builder_set_member_name(m_builder, "type");
    json_builder_add_string_value(m_builder, type);
    json_builder_set_member_name(m_builder, "coordinates");
}

void
GeoJsonSimplifyHandler::endGeometry()
{
    json_builder_end_object(m_builder); // geometry
}

void
GeoJsonSimplifyHandler::addMultiPolygon(JsonArray* multipoly)
{
    json_builder_begin_array(m_builder);
}

void
GeoJsonSimplifyHandler::endMultiPolygon()
{
    json_builder_end_array(m_builder); // coordinates
}

void
GeoJsonSimplifyHandler::addPolygon(JsonArray* poly)
{
    json_builder_begin_array(m_builder);
}

void
GeoJsonSimplifyHandler::endPolygon()
{
    json_builder_end_array(m_builder); // coordinates
}

void
GeoJsonSimplifyHandler::addShape(JsonArray* shape)
{
    json_builder_begin_array(m_builder);
}

void
GeoJsonSimplifyHandler::endShape()
{
    json_builder_end_array(m_builder);
}

void
GeoJsonSimplifyHandler::addCoord(JsonArray* coord, bool last)
{
    int coordLen = json_array_get_length(coord);
    if (coordLen >= 2) {
        double lon = json_array_get_double_element(coord, 0);
        double lat = json_array_get_double_element(coord, 1);
        bool use = true;
        if (!last
         && m_lastLon != DEFAULT_START_COORD
         && m_lastLat != DEFAULT_START_COORD) {
            auto dist = std::hypot(m_lastLon - lon, m_lastLat - lat);
            use = dist > m_min_distance;
        }
        if (use) {
            json_builder_begin_array(m_builder);
            json_builder_add_double_value(m_builder, lon);
            json_builder_add_double_value(m_builder, lat);
            m_lastLon = lon;
            m_lastLat = lat;
            json_builder_end_array(m_builder);
        }
    }
    else {
        // keep as warning ?
        std::cout << "Expected coords 2 got " << coordLen << std::endl;
    }
}

GeoJsonSimplifyHandler::GeoJsonSimplifyHandler()
: GeoJsonHandler()
{
    m_builder = json_builder_new();
    json_builder_begin_object(m_builder);
    json_builder_set_member_name(m_builder, "type");
    json_builder_add_string_value(m_builder, "FeatureCollection");
    json_builder_set_member_name(m_builder, "features");
    json_builder_begin_array(m_builder);
}

void
GeoJsonSimplifyHandler::setMinDistance(double minDist)
{
    m_min_distance = minDist;
}

double
GeoJsonSimplifyHandler::getMinDistance()
{
    return m_min_distance;
}

void
GeoJsonSimplifyHandler::exportFile(const Glib::ustring& file)
{
    json_builder_end_array(m_builder);
    json_builder_end_object(m_builder);
    JsonGenerator *generator = json_generator_new();
    json_generator_set_root(generator, json_builder_get_root(m_builder));
    //json_generator_set_indent(generator, 2);
    //json_generator_set_pretty(generator, true);
    GError* error{nullptr};
    json_generator_to_file(generator, file.c_str(), &error);
    if (error) {
        Glib::ustring msg = std::format("Error {0} export json file {1}", error->message, file);
        g_error_free(error);
        throw JsonException(msg);
    }
}
