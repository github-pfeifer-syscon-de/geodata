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


#include <StringUtils.hpp>
#include <cmath>

#include "GeoKlm.hpp"



namespace psc::geo {

GeoKlm::GeoKlm()
: m_cancel{Gio::Cancellable::create()}
{
}


void
GeoKlm::readCompress(const Glib::RefPtr<Gio::FileInputStream>& fileStream
    , Glib::Markup::ParseContext& context)
{
    // this is the "wrong" zip, since the intention of these is to pack media files the use is limited ...
    auto decompress = Gio::ZlibDecompressor::create(Gio::ZlibCompressorFormat::ZLIB_COMPRESSOR_FORMAT_ZLIB);
    auto bufOut = Glib::ByteArray::create();
    bufOut->set_size(BUFFER_SIZE);
    auto bufIn = Glib::ByteArray::create();
    bufIn->set_size(BUFFER_SIZE);
    while (true) {
        gssize read = fileStream->read(bufIn->get_data(), bufIn->size());
        while (true) {
            gsize bytes_read, bytes_written;
            auto res = decompress->convert(bufIn->get_data(), read
                         , bufOut->get_data(), bufOut->size()
                         , (read <= 0)
                            ? Gio::ConverterFlags::CONVERTER_FLUSH
                            : Gio::ConverterFlags::CONVERTER_NO_FLAGS
                         , bytes_read, bytes_written);
            const char* start = reinterpret_cast<const char*>(bufOut->get_data());
            const char* end = start + bytes_written;
            if (end > start) {
                context.parse(start, end);
            }
            if (bytes_written < bufOut->size() || res == Gio::ConverterResult::CONVERTER_FINISHED) {
                break;
            }
        }
        if (read <= 0) {
            break;
        }
    }
    context.end_parse();
}

void
GeoKlm::readNativ(const Glib::RefPtr<Gio::FileInputStream>& fileStream
    , Glib::Markup::ParseContext& context)
{
    auto bufIn = Glib::ByteArray::create();
    bufIn->set_size(BUFFER_SIZE);
    while (true) {
        //std::string line;       // some lines are huge +200k
        //auto dataStream = Gio::DataInputStream::create(fileStream);
        //if (!dataStream->read_line(line, m_cancel)) {
        //    break;
        //}
        //line += '\n';           // keep as lines
        //context.parse(line);    // better avoid ustring conversion
        // this makes the processing "chunky", but overall faster
        gssize read = fileStream->read(bufIn->get_data(), bufIn->size());
        if (read <= 0) {
            break;
        }
        const char* start = reinterpret_cast<const char*>(bufIn->get_data());
        auto end = start;
        std::advance(end, read);
        context.parse(start, end);
    }
    context.end_parse();
}

void
GeoKlm::read(const std::string& filePath)
{
    KXMLParser parser(this);
    Glib::Markup::ParseContext context(parser);	// , Glib::Markup::ParseFlags::TREAT_CDATA_AS_TEXT
    try {
        auto file = Gio::File::create_for_path(filePath);
        auto fileStream = file->read();
        if (StringUtils::endsWith(filePath, ".kmz")) {
            //readCompress(fileStream, context);
            throw std::runtime_error("Reading for .kmz is not implemented...");
        }
        else {
            readNativ(fileStream, context);
        }
    }
    catch (const std::exception& ex) {
        //logMsg(psc::log::Level::Error, Glib::ustring::sprintf("Error %s", ex.what()));
        std::cout << "Error " <<  ex.what() << std::endl;
    }
}


PtrKlmGeometry
GeoKlm::parseCoords(const Glib::ustring& text)
{
    std::vector<GeoCoordinate> coords;
    if (m_average_coord_encode_length == 0) {   // adapt to file effective number encoding
        size_t coordCount{};
        StringUtils::splitNotify(
            text
            ,Glib::Unicode::isspace
            ,[&](size_t pos, size_t len) {
            ++coordCount;
            return true;
        });
        int32_t effectiveRatio = static_cast<int32_t>(text.size() / coordCount);
        m_average_coord_encode_length = static_cast<int32_t>(static_cast<double>(effectiveRatio) * (1.0 - PESSIMISTIC_SIZE_RATIO));
    }
    coords.reserve(text.size() / static_cast<uint64_t>(m_average_coord_encode_length));
    StringUtils::splitNotify(
        text
        ,Glib::Unicode::isspace
        ,[&](size_t pos, size_t len) {
        auto part = text.substr(pos, len);
        auto coordsComp = StringUtils::splitEach(part, static_cast<gunichar>(','));
        if (coordsComp.size() >= 2) {   // ignore height
             double lon = GeoCoordinate::parseDouble(coordsComp[0]);
             double lat = GeoCoordinate::parseDouble(coordsComp[1]);
             coords.emplace_back(std::move(GeoCoordinate(lon, lat, CoordRefSystem::CRS_84)));
        }
        return true;
    });
    sum_text += text.size();
    sum_coords += coords.size();
    int32_t effectiveRatio = static_cast<int32_t>(text.size() / coords.size());
    //std::cout << "   " << coords.size() << " coordinates"
    //          << " guessed " << (text.size() / m_average_coord_encode_length)
    //          << " ratio "  << effectiveRatio << std::endl;
    int32_t diff = effectiveRatio - m_average_coord_encode_length;
    if (coords.size() > MIN_ADJUST_SIZE
     && (diff > static_cast<int32_t>(static_cast<double>(m_average_coord_encode_length) * PESSIMISTIC_SIZE_RATIO * 2.0)   // accept overallocation upto ~20%
     || diff < 0)) {                         // in case of underallocation adjust
        //std::cout << "   changed " << (effectiveRatio - PESSIMISTIC_SIZE_GUESS)
        //          << " from " << m_average_coord_encode_length << std::endl;
        m_average_coord_encode_length = static_cast<int32_t>(static_cast<double>(effectiveRatio) * (1.0 - PESSIMISTIC_SIZE_RATIO));
    }
    if (coords.size() == 1) {
        auto klmPnt = std::make_shared<KlmPoint>();
        std::cout << "Adding point" << std::endl;
        klmPnt->moveCoords(coords);
        return klmPnt;
    }
    else {
        auto klmSeg = std::make_shared<KlmSegment>();
        std::cout << "Adding coords" << coords.size() << std::endl;
        klmSeg->moveCoords(coords);
        return klmSeg;
    }
}


KXMLParser::KXMLParser(GeoKlm *geoKlm)
: m_geoKlm{geoKlm}
{
}

void
KXMLParser::on_start_element(Glib::Markup::ParseContext& context,
        const Glib::ustring& element_name,
        const Glib::Markup::Parser::AttributeMap& attributes)
{
    //std::cout << "start " << element_name << std::endl;
    if (element_name == "Placemark") {
        m_placemark = std::make_shared<KlmPlacemark>();
    }
    else if (element_name == "Schema") {
        m_textContext = TextContext::Schema;
    }
    else if (element_name == "SchemaData") {
        m_textContext = TextContext::Schemadata;
    }
    else if (element_name == "SimpleData") {
        auto nameIter = attributes.find("name");
        if (nameIter != attributes.end()) {
            m_simpleDataName = nameIter->second;
        }
        if (m_textContext == TextContext::Schema) {
            auto typeIter = attributes.find("type");
            if (typeIter != attributes.end()) {
                Glib::ustring simpleDataType = typeIter->second;
                m_schemaMap.insert(std::pair(m_simpleDataName, simpleDataType));
            }
        }
    }
    else if (element_name == "MultiGeometry") {
        auto multiGeom = std::make_shared<KlmMultiPolygon>();
        m_placemark->addGeometry(multiGeom);
    }
    else if (element_name == "Polygon") {
        auto polyGeom = std::make_shared<KlmPolygon>();
        m_placemark->addGeometry(polyGeom);
    }
    else if (element_name == "Point") {
        auto polyGeom = std::make_shared<KlmPoint>();
        m_placemark->addGeometry(polyGeom);
    }
    else if (element_name == "coordinates") {
        m_textContext = TextContext::Coordinates;
    }
}

void
KXMLParser::on_end_element(Glib::Markup::ParseContext& context,
                const Glib::ustring& element_name)
{
    //std::cout << "end " << element_name << std::endl;
    if (element_name == "Placemark") {
        m_geoKlm->addPlacemark(m_placemark);
        m_placemark.reset();
    }
    else if (element_name == "Schema") {
        m_simpleDataName.clear();
        m_textContext = TextContext::None;
    }
    else if (element_name == "SchemaData") {
        m_simpleDataName.clear();
        m_textContext = TextContext::None;
    }
    else if (element_name == "coordinates") {
        m_textContext = TextContext::None;
    }
}

bool
KXMLParser::isBlank(const Glib::ustring& text)
{
    auto iter = text.begin();
    while (iter != text.end()) {
        if (!Glib::Unicode::isspace(*iter)) {
            return false;
        }
        ++iter;
    }
    return true;
}

void
KXMLParser::on_text(Glib::Markup::ParseContext& context,
                const Glib::ustring& text)
{
    if (isBlank(text)) {
        return;     // we get any linefeed ...  ignore these
    }
    switch (m_textContext) {
    case TextContext::Schema:
        if (!text.empty()) {
            std::cout << "Unexpected text " <<  text << "for schema." << std::endl;
        }
        break;      // this was handeled with startElement
    case TextContext::Schemadata: {
            if (m_placemark) {
                if (m_simpleDataName == "shapeName") {
                    std::cout << "placemark " << text << std::endl;
                }
                m_placemark->addSimpleData(m_simpleDataName, text);
            }
        }
        break;
    case TextContext::Coordinates: {
            auto geom = m_geoKlm->parseCoords(text);
            m_placemark->addGeometry(geom);
        }
        break;
    default:
        //std::cout << "KXMLParser text unhandeled context " << static_cast<int>(m_textContext) << std::endl;
        break;
    }
}


void
KXMLParser::on_error(Glib::Markup::ParseContext& context,
                const Glib::MarkupError& error)
{
    std::cout << "error " << error.what() << std::endl;
    //throw error;
}


} // namespace psc::geo