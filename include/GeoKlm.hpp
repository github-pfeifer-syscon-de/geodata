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

#include <glibmm.h>
#include <string>
#include <memory>
#include <vector>

#include "GeoCoordinate.hpp"
#include "GenericGeo.hpp"

namespace psc::geo {
class KlmGeometry
{
public:
    KlmGeometry() = default;
    explicit KlmGeometry(const KlmGeometry& other) = delete;
    virtual ~KlmGeometry() = default;

};

using PtrKlmGeometry = std::shared_ptr<KlmGeometry>;

class KlmSegment
: public Segment
, public KlmGeometry
{
public:
    KlmSegment() = default;
    explicit KlmSegment(const KlmSegment& other) = delete;
    virtual ~KlmSegment() = default;

    void moveCoords(const std::vector<GeoCoordinate>& coords){
        m_coords = std::move(coords);
    }
protected:
};

using PtrKlmSegment = std::shared_ptr<KlmSegment>;

class KlmPolygon
: public Polygon
, public KlmGeometry
{
public:
    KlmPolygon() = default;
    explicit KlmPolygon(const KlmPolygon& other) = delete;
    virtual ~KlmPolygon() = default;

    void addGeometry(const PtrKlmGeometry& geom) {
        auto segm = std::dynamic_pointer_cast<KlmSegment>(geom);
        if (segm) {
            m_segments.emplace_back(std::move(segm));
            return;
        }
        std::cout << "KlmPolygon::addGeometry unexpected type" << std::endl;
    }
protected:
};

using PtrKlmPolygon = std::shared_ptr<KlmPolygon>;

class KlmMultiPolygon
: public MultiPolygon
, public KlmGeometry
{
public:
    KlmMultiPolygon() = default;
    explicit KlmMultiPolygon(const KlmMultiPolygon& other) = delete;
    virtual ~KlmMultiPolygon() = default;

    void addGeometry(const PtrKlmGeometry& geom) {
        auto poly = std::dynamic_pointer_cast<KlmPolygon>(geom);
        if (poly) {
            m_polygons.push_back(std::move(poly));
            return;
        }
        if (m_polygons.empty()) {
            m_polygons.push_back(std::make_shared<KlmPolygon>());
        }
        auto lastPoly = std::dynamic_pointer_cast<KlmPolygon>(m_polygons[m_polygons.size() - 1]);
        lastPoly->addGeometry(geom);
    }
protected:
};

using PtrKlmMultiPolygon = std::shared_ptr<KlmMultiPolygon>;

class KlmProperties
: public Properties
{
public:
    KlmProperties() = default;
    explicit KlmProperties(const KlmProperties& other) = delete;
    virtual ~KlmProperties() = default;
    void addSimpleData(const Glib::ustring& name, const Glib::ustring& content){
        auto value = std::make_unique<PropValueString>(content);
        m_properties.emplace(name, std::move(value));
    }
protected:
};

using PtrKlmProperties = std::shared_ptr<KlmProperties>;

class KXMLParser;

class KlmPlacemark
: public Feature    // ~equivalent
{
public:
    KlmPlacemark()
    : m_properties(std::make_shared<KlmProperties>())
    {
    }
    explicit KlmPlacemark(const KlmPlacemark& other) = delete;
    virtual ~KlmPlacemark() = default;

    void addGeometry(const PtrKlmGeometry& geom) {
        auto multi = std::dynamic_pointer_cast<KlmMultiPolygon>(geom);
        if (multi) {
            m_multiPolygon = std::move(multi);
            return;
        }
        if (!m_multiPolygon) {
            m_multiPolygon = std::move(std::make_shared<KlmMultiPolygon>());
        }
        m_multiPolygon->addGeometry(geom);
    }
    void addSimpleData(const Glib::ustring& name, const Glib::ustring& content) {
        m_properties->addSimpleData(name, content);
    }
    PtrProperties getProperties() override {
        return m_properties;
    }
    PtrGeometry getGeometry() override {
        return m_multiPolygon;
    }
protected:
    PtrKlmProperties m_properties;
    PtrKlmMultiPolygon m_multiPolygon;
};

using PtrKlmPlacemark = std::shared_ptr<KlmPlacemark>;

class GeoKlm {
public:
    GeoKlm();
    explicit GeoKlm(const GeoKlm& other) = delete;
    virtual ~GeoKlm() = default;

    void read(const std::string& fileName);
    void addPlacemark(const PtrKlmPlacemark& placemark) {
        m_places.push_back(placemark);
    }
    std::vector<PtrFeature> getPlacemarks()
    {
        std::vector<PtrFeature> feats;
        for (auto place : m_places) {
            feats.push_back(place);
        }
        return feats;
    }
    PtrKlmSegment parseCoords(const Glib::ustring& text);

    static constexpr auto MIN_ADJUST_SIZE{16ul};            // don't adjust for smaller sizes as for these calculation will be inaccurate, and it doesn't matter that much
    static constexpr double PESSIMISTIC_SIZE_RATIO{0.1};    // guess smaller tend to overallocation
    static constexpr auto BUFFER_SIZE{32768ul};
protected:
    void readCompress(const Glib::RefPtr<Gio::FileInputStream>& fileStream
        , Glib::Markup::ParseContext& context);
    void readNativ(const Glib::RefPtr<Gio::FileInputStream>& fileStream
        , Glib::Markup::ParseContext& context);

    Glib::RefPtr<Gio::Cancellable> m_cancel;
    bool m_done{false};
    std::vector<PtrKlmPlacemark> m_places;
    int32_t m_average_coord_encode_length{};
    int64_t sum_text{};
    int64_t sum_coords{};
};


enum class TextContext
{
    None
    , Schema
    , Schemadata
    , Coordinates
    ,
};


class KXMLParser
: public Glib::Markup::Parser
{
public:
    KXMLParser(GeoKlm* geoKlm);
    virtual ~KXMLParser() = default;
protected:
    void on_start_element(Glib::Markup::ParseContext& context,
                const Glib::ustring& element_name,
                const Glib::Markup::Parser::AttributeMap& attributes) override;
    void on_end_element(Glib::Markup::ParseContext& context,
                const Glib::ustring& element_name ) override;
    void on_text(Glib::Markup::ParseContext& context,
                const Glib::ustring& text) override;
    void on_error(Glib::Markup::ParseContext& context,
                const Glib::MarkupError& error) override;
    bool isBlank(const Glib::ustring& text);

    GeoKlm* m_geoKlm;
    Glib::ustring m_simpleDataName;
    TextContext m_textContext{TextContext::None};
    PtrKlmPlacemark m_placemark;
    std::map<Glib::ustring, Glib::ustring> m_schemaMap;
};



} // namespace psc::geo
