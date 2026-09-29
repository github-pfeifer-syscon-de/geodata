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

#include "GeoJson2.hpp"

namespace psc::geo
{

Properties::Properties(JsonObject* propObj, const std::string& ctx)
{
    GList* values = json_object_get_members(propObj);
    for (GList* elem = values; elem; elem = elem->next) {
        auto* memberName = (const gchar*)elem->data;
        auto memNode = json_object_get_member(propObj, memberName);
        auto nodeType = json_node_get_node_type(memNode);
        auto smemberName = Glib::ustring(memberName);
        if (nodeType == JSON_NODE_VALUE) {
            auto valType = json_node_get_value_type(memNode);
            if (valType == G_TYPE_INT || valType == G_TYPE_INT64) {
                auto intVal = json_node_get_int(memNode);
                auto uptr{std::make_unique<PropValueInt>(intVal)};
                m_properties.emplace(smemberName, std::move(uptr));
            }
            else if (valType == G_TYPE_DOUBLE) {
                auto dbl = json_node_get_double(memNode);
                auto uptr{std::make_unique<PropValueDouble>(dbl)};
                m_properties.emplace(smemberName, std::move(uptr));
            }
            else if (valType == G_TYPE_BOOLEAN) {
                auto bVal = static_cast<bool>(json_node_get_boolean(memNode));
                auto uptr{std::make_unique<PropValueBool>(bVal)};
                m_properties.emplace(smemberName, std::move(uptr));
            }
            else if (g_type_is_a(valType, G_TYPE_STRING)) {
                auto cVal = json_node_get_string(memNode);
                auto sVal= Glib::ustring(cVal);
                auto uptr{std::make_unique<PropValueString>(sVal)};
                m_properties.emplace(smemberName, std::move(uptr));
            }
            else {
                auto msg = std::format( "GeoJson2 {} property {} unhandeld value-type {}",  ctx,  smemberName, static_cast<int>(valType));
                std::cout << msg << std::endl;  // there might be more types
            }
        }
        else if (nodeType == JSON_NODE_NULL) {
            //m_properties.insert(std::pair(smemberName, ""));
        }
        else {
            auto msg = std::format( "GeoJson2 {} property {} unhandeld node-type {}",  ctx,  smemberName, static_cast<int>(nodeType));
            throw Json2Exception(msg);
        }
    }
    g_list_free(values);
}

ValueType
Properties::getType(const Glib::ustring& key)
{
    auto iter = m_properties.find(key);
    if (iter != m_properties.end()) {
        auto& val = iter->second;
        return val->getValueType();
    }
    return ValueType::Null;
}

int64_t
Properties::getInteger(const Glib::ustring& key)
{
    auto iter = m_properties.find(key);
    if (iter != m_properties.end()) {
        auto& val = iter->second;
        auto iVal = dynamic_cast<PropValueInt*>(val.get()); // since uniqueptr allows no casting ... stealing should be ok
        if (iVal != nullptr) {
            return iVal->get();
        }
    }
    return 0l;
}

double
Properties::getDouble(const Glib::ustring& key)
{
    auto iter = m_properties.find(key);
    if (iter != m_properties.end()) {
        auto& val = iter->second;
        auto dVal = dynamic_cast<PropValueDouble*>(val.get()); // since uniqueptr allows no casting ... stealing should be ok
        if (dVal != nullptr) {
            return dVal->get();
        }
    }
    return 0.0;
}

bool
Properties::getBool(const Glib::ustring& key)
{
    auto iter = m_properties.find(key);
    if (iter != m_properties.end()) {
        auto& val = iter->second;
        auto bVal = dynamic_cast<PropValueBool*>(val.get()); // since uniqueptr allows no casting ... stealing should be ok
        if (bVal != nullptr) {
            return bVal->get();
        }
    }
    return false;
}

Glib::ustring
Properties::getString(const Glib::ustring& key)
{
    auto iter = m_properties.find(key);
    if (iter != m_properties.end()) {
        auto& val = iter->second;
        auto sVal = dynamic_cast<PropValueString*>(val.get()); // since uniqueptr allows no casting ... stealing should be ok
        if (sVal != nullptr) {
            return sVal->get();
        }
    }
    return "";
}

GeoCoordinate
Geometry::readCoord(JsonArray* coord, const std::string& ctx)
{
    auto cnt = json_array_get_length(coord);
    if (cnt == 2) {
        double lon = json_array_get_double_element(coord, 0);
        double lat = json_array_get_double_element(coord, 1);
        return GeoCoordinate(lon, lat, CoordRefSystem::CRS_84);
    }
    auto msg = std::format( "GeoJson2 {} geometry count {} unexpected",  ctx,  cnt);
    throw Json2Exception(msg);
}

Segment::Segment(JsonArray* segm, const std::string& ctx) {
    auto cntIn = json_array_get_length(segm);
    m_coords.reserve(cntIn);
    for (uint32_t j = 0; j < cntIn; ++j) {
        JsonArray* polyIn = json_array_get_array_element(segm, j);
        m_coords.push_back(readCoord(polyIn, ctx));
    }
}

Polygon::Polygon(JsonArray* poly, const std::string& ctx) {
    auto pctx = ctx + " poly ";
    auto cntOut = json_array_get_length(poly);
    m_segments.reserve(cntOut);
    for (uint32_t i = 0; i < cntOut; ++i) {
        auto poctx = pctx + std::to_string(i);
        JsonArray* polyOut = json_array_get_array_element(poly, i);
        auto segm = std::make_shared<Segment>(polyOut, poctx);
        m_segments.emplace_back(std::move(segm));
    }
}

MultiPolygon::MultiPolygon(JsonArray* coord, const std::string& ctx)
{
    auto pctx = ctx + " multi poly ";
    auto cntOut = json_array_get_length(coord);
    m_polygons.reserve(cntOut);
    for (uint32_t i = 0; i < cntOut; ++i) {
        auto poctx = pctx + std::to_string(i);
        JsonArray* polyOut = json_array_get_array_element(coord, i);
        auto poly = std::make_shared<Polygon>(polyOut, poctx);
        m_polygons.emplace_back(std::move(poly));
    }
}

Feature::Feature(JsonObject* featObj, const std::string& ctx)
{
    if (static_cast<bool>(json_object_has_member(featObj, "type"))) {
        Glib::ustring type = json_object_get_string_member(featObj, "type");
        if (type == "Feature") {
            if (static_cast<bool>(json_object_has_member(featObj, "geometry"))) {
                auto geometry = json_object_get_object_member(featObj, "geometry");
                m_geometry = readGeometry(geometry, ctx);
            }
            else {
                auto msg = std::format( "GeoJson2 {} expected geometry not found",  ctx);
                throw Json2Exception(msg);
            }
            if (static_cast<bool>(json_object_has_member(featObj, "properties"))) {
                auto properties = json_object_get_object_member(featObj, "properties");
                m_properties = std::make_shared<Properties>(properties, ctx);
            }
            else {  // this should  probably not be an error
                std::cout << "GeoJson2 " << ctx << " expected properties not found" << std::endl;
            }
        }
        else {
            auto msg = std::format( "GeoJson2 {} expected type Feature got {}",  ctx, type);
            throw Json2Exception(msg);
        }
    }
    else {
        auto msg = std::format( "GeoJson2 {} expected type for feature not found",  ctx);
        throw Json2Exception(msg);
    }
}

PtrGeometry
Feature::readGeometry(JsonObject* geometry, const std::string& ctx)
{
    if (static_cast<bool>(json_object_has_member(geometry, "type"))) {
        if (static_cast<bool>(json_object_has_member(geometry, "coordinates"))) {
            auto coords = json_object_get_array_member(geometry, "coordinates");
            Glib::ustring geomType = json_object_get_string_member(geometry, "type");
            if (geomType == "Point") {
                return std::make_shared<Point>(coords, ctx);
            }
            if (geomType == "Polygon") {
                return std::make_shared<Polygon>(coords, ctx);
            }
            if (geomType == "MultiPolygon") {
                return std::make_shared<MultiPolygon>(coords, ctx);
            }
            auto msg = std::format( "GeoJson2 {} geometry type {} unknown",  ctx,  geomType);
            throw Json2Exception(msg);
        }
        auto msg = std::format( "GeoJson2 {} geometry expected coordinates not found",  ctx);
        throw Json2Exception(msg);
    }
    auto msg = std::format( "GeoJson2 {} geometry expected type not found",  ctx);
    throw Json2Exception(msg);
}

std::vector<PtrFeature>
GeoJson2::read(JsonHelper& helper)
{
    std::vector<PtrFeature> features;
    auto root = helper.get_root_object();
    if (static_cast<bool>(json_object_has_member(root, "type"))) {
        auto type = Glib::ustring(json_object_get_string_member(root, "type"));
        if (type == "FeatureCollection") {
            if (static_cast<bool>(json_object_has_member(root, "features"))) {
                auto featArray = json_object_get_array_member(root, "features");
                auto featCnt = json_array_get_length(featArray);
                for (uint32_t i = 0; i < featCnt; i++) {
                    auto featElem = json_array_get_object_element(featArray, i);
                    auto ctx = std::format("FeatureCollection[{}]", i);
                    auto feat = std::make_shared<Feature>(featElem, ctx);
                    features.emplace_back(std::move(feat));
                }
            }
            else {
                throw Json2Exception("GeoJson2 root expected member features not found");
            }
        }
        else {
            throw Json2Exception("GeoJson2 root expected type FeatureCollection got");
        }
    }
    else {
        throw Json2Exception("GeoJson2 root expected type not found");
    }
    return features;
}

}