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
#include <string>
#include <StringUtils.hpp>
#include <limits>

#include "WebMapService.hpp"
#include "LocaleContext.hpp"
#include "MapProjection.hpp"

// docs (even if the examples are outdated):
// https://sos.noaa.gov/support/sos/how-to/wms-tutorial/all/
// https://sos.noaa.gov/media/downloads/wms_tutorial.pdf
// the full docs 1.3.0
// http://portal.opengeospatial.org/files/?artifact_id=14416

WebMapImageRequest::WebMapImageRequest(WebMapService* webMapService
        , const GeoBounds& bounds
        , int pixX, int pixY, int pixWidth, int pixHeight
        , std::shared_ptr<WebMapProduct>& product)
: WeatherImageRequest(webMapService->getAddress(), "")
, m_webMapService{webMapService}
, m_bounds{bounds}
, m_pixX{pixX}
, m_pixY{pixY}
, m_pixWidth{pixWidth}
, m_pixHeight{pixHeight}
{
    addQuery("service", "WMS");
    addQuery("version", "1.3.0");
    addQuery("REQUEST", "GetMap");
    addQuery("LAYERS", product->get_id());
    addQuery("CRS", product->getCoordRefSystem().identifier());
    addQuery("FORMAT", "image/png");
    addQuery("HEIGHT", Glib::ustring::sprintf("%d", m_pixHeight));
    addQuery("WIDTH", Glib::ustring::sprintf("%d", m_pixWidth));
    addQuery("TRANSPARENT", "TRUE");    // prefer transparent
    if (!product->get_latest().empty()) {
        #ifdef WEATHER_DEBUG
        std::cout << "using time " << product->get_latest() << std::endl;
        #endif
        addQuery("TIME", product->get_latest());
    }
    Glib::ustring bound = m_bounds.printValue(',');
    #ifdef WEATHER_DEBUG
    std::cout << "m_westSouth " << m_bounds.getWestSouth().getLongitude()
              << ", " << m_bounds.getWestSouth().getLatitude()
              << " " << m_bounds.getWestSouth().getCoordRefSystem().identifier() << std::endl;
    std::cout << "m_eastNorth " << m_bounds.getEastNorth().getLongitude()
              << ", " << m_bounds.getEastNorth().getLatitude()
              << " " << m_bounds.getEastNorth().getCoordRefSystem().identifier() << std::endl;
    std::cout << "WebMapImageRequest::WebMapImageRequest bound " << bound << std::endl;
    #endif
    addQuery("BBOX", bound);
    signal_receive().connect(
        sigc::mem_fun(*webMapService, &WebMapService::inst_on_image_callback));
}

void
WebMapImageRequest::mapping(Glib::RefPtr<Gdk::Pixbuf> pix, Glib::RefPtr<Gdk::Pixbuf>& weather_pix)
{
    Glib::RefPtr<Gdk::Pixbuf> clearPix = Gdk::Pixbuf::create(pix->get_colorspace(), pix->get_has_alpha(), pix->get_bits_per_sample(), pix->get_width(), 1);
    clearPix->fill(0x0);   // transp. black
	bool isnorth = m_bounds.getEastNorth().getLatitude() > 0.0;
    double pix_height = pix->get_height();
	double relOrigin = (isnorth
                        ? m_bounds.getEastNorth().getCoordRefSystem().toLinearLat(m_bounds.getEastNorth().getLatitude())
                        : std::abs(m_bounds.getWestSouth().getCoordRefSystem().toLinearLat(m_bounds.getWestSouth().getLatitude())));
	for (int linY = 0; linY < pix_height; ++linY) {
	    double relLat = isnorth
		            ? ((double)(pix_height - linY) / pix_height)
		            : ((double)linY / pix_height);
	    // still have to adapt, target will always be 0...90 but source 0...max
	    if (relLat < relOrigin) {
            double relMap = isnorth
                    ? 1.0 - (relLat / relOrigin)
                    : (relLat / relOrigin);
            int linYsrc = (int)(relMap * pix_height);
            if (linYsrc >= 0 && linYsrc < pix_height) {     // just to be safe (better than to crash)
                pix->copy_area(0, linYsrc, pix->get_width(), 1, weather_pix, m_pixX, m_pixY+linY);
            }
            else {
                std::cout << "Generated lin y " << linYsrc << " while mapping exceeded size " << pix_height << std::endl;
            }
    	}
        else {
            clearPix->copy_area(0, 0, clearPix->get_width(), 1, weather_pix, m_pixX, m_pixY+linY);
        }
    }
}

WebMapProduct::WebMapProduct(WebMapService* webMapService)
: WeatherProduct()
, m_webMapService{webMapService}
{
    m_parseLevel.push(ParseContext::None);  // keep this so we always return something on pop
}

void
WebMapProduct::start_element(Glib::Markup::ParseContext& context,
    const Glib::ustring& element_name,
    const Glib::Markup::Parser::AttributeMap& attributes)
{
    if (element_name == "Name") {
        m_context = ParseContext::Name;
    }
    else if (element_name == "Title") {
        m_context = ParseContext::Title;
    }
    else if (element_name == "Abstract") {
        m_context = ParseContext::Abstract;
    }
    else if (element_name == "KeywordList") {
        m_context = ParseContext::KeywordList;
    }
    else if (element_name == "Keyword") {
        m_context = ParseContext::Keyword;
    }
    else if (element_name == "CRS") {
        m_context = ParseContext::CRS;
    }
    else if (element_name == "EX_GeographicBoundingBox") {
        m_context = ParseContext::EX_GeographicBoundingBox;
    }
    else if (element_name == "westBoundLongitude") {
        m_context = ParseContext::westBoundLongitude;
    }
    else if (element_name == "eastBoundLongitude") {
        m_context = ParseContext::eastBoundLongitude;
    }
    else if (m_context == ParseContext::EX_GeographicBoundingBox
          && element_name == "southBoundLatitude") {
        m_context = ParseContext::southBoundLatitude;
    }
    else if (element_name == "northBoundLatitude") {
        m_context = ParseContext::northBoundLatitude;
    }
    else if (element_name == "BoundingBox") {
        auto crs = attributes.find("CRS");
        if (crs != attributes.end()
         && getCoordRefSystem() == CoordRefSystem::None) {  // if we did not find a useable crs up to now
            m_crs = CoordRefSystem::parse(crs->second);     // servers seem not to mind being asked one advertised with bounds
        }
        auto minx = attributes.find("minx");
        auto maxx = attributes.find("maxx");
        auto miny = attributes.find("miny");
        auto maxy = attributes.find("maxy");
        if (crs != attributes.end()
         && crs->second == getCoordRefSystem().identifier()
         && minx != attributes.end()
         && maxx != attributes.end()
         && miny != attributes.end()
         && maxy != attributes.end()) {
            bool latFirst = getCoordRefSystem().is_latitude_first();
            m_bounds.getWestSouth().parseLongitude(latFirst ? miny->second : minx->second);
            m_bounds.getWestSouth().parseLatitude(latFirst ? minx->second : miny->second);
            m_bounds.getWestSouth().setCoordRefSystem(getCoordRefSystem());
            m_bounds.getEastNorth().parseLongitude(latFirst ? maxy->second : maxx->second);
            m_bounds.getEastNorth().parseLatitude(latFirst ? maxx->second : maxy->second);
            m_bounds.getEastNorth().setCoordRefSystem(getCoordRefSystem());
        }
        m_context = ParseContext::BoundingBox;
    }
    else if (element_name == "Dimension") {
        auto name = attributes.find("name");
        if (name != attributes.end()
         && name->second == "time") {
            auto def = attributes.find("default");
            if (def != attributes.end()
             && def->second != "current") {
                // prefere text
            }
        }
        m_context = ParseContext::Dimension;
    }
    else if (element_name == "Style") {
        m_context = ParseContext::Style;
    }
    else if (element_name == "LegendURL") {
        auto width = attributes.find("width");
        if (width != attributes.end()) {
            m_LastLegendWidth = width->second;
        }
        m_context = ParseContext::LegendURL;

    }
    else if (element_name == "Format") {
        m_context = ParseContext::Format;
    }
    else if (element_name == "OnlineResource") {
        auto type = attributes.find("xlink:type");
        if (type != attributes.end()
         && type->second == "simple") {
            auto link = attributes.find("xlink:href");
            if (link != attributes.end()) {
                auto url = link->second;
                // this is probably a EumetSat quirk ...
                if (!m_LastLegendWidth.empty()) {   // the default url doesn't work
                    url += "&WIDTH=" + m_LastLegendWidth;
                    m_LastLegendWidth.clear();
                }
                m_legends.push_back(url);
            }
        }
        m_context = ParseContext::OnlineResource;
    }
    else if (element_name == "Attribution") {
        m_context = ParseContext::Attribution;
    }
    else if (element_name == "MinScaleDenominator") {
        m_context = ParseContext::MinScaleDenominator;
    }
    else if (element_name == "MaxScaleDenominator") {
        m_context = ParseContext::MaxScaleDenominator;
    }
    else {
        #ifdef WEATHER_DEBUG
        std::cout << "Unhandled element start " << element_name << std::endl;
        #endif
    }
    m_parseLevel.push(m_context);
}

void
WebMapProduct::end_element(Glib::Markup::ParseContext& context,
    const Glib::ustring& element_name)
{
    m_parseLevel.pop();
    m_context = m_parseLevel.top();
}

Glib::ustring
WebMapProduct::get_legend_url()
{
    if (!m_legends.empty()) {
        return m_legends[0];    // unsure there are 2 legends (but as we use the default style the first seems to be right)
    }
    return "";
}

CoordRefSystem
WebMapProduct::getCoordRefSystem()
{
    return m_crs;
}

Glib::ustring
WebMapProduct::get_description()
{
    return m_abstract;
}

void
WebMapProduct::text(Glib::Markup::ParseContext& context, const Glib::ustring& text)
{
    switch (m_context) {
    case ParseContext::None:
        break;
    case ParseContext::Name:
        if (m_parseLevel.size() == 2) {
            m_id = text;
        }
        break;
    case ParseContext::Title:
        if (m_parseLevel.size() == 2) {
            m_name = text;
        }
        break;
    case ParseContext::Abstract:
        m_abstract = text;
        break;
    case ParseContext::KeywordList:
        break;
    case ParseContext::Keyword:
        m_keywords = text;
        break;
    case ParseContext::CRS:
        if (!m_crs) {    // keep the first usable
            m_crs = CoordRefSystem::parse(text);
            #ifdef WEATHER_DEBUG
            if (m_crs == CoordRefSystem::None) {
                std::cout << "WebMapProduct::text " << get_id()
                      << " unusable crs from \"" << text << "\"" << std::endl;
            }
            #endif
        }
        break;
    case ParseContext::EX_GeographicBoundingBox:
        break;
    case ParseContext::westBoundLongitude:
        if (m_parseLevel.size() == 3    // could check parent element
         && getCoordRefSystem()) {      // only useful with crs defined
            m_bounds.getWestSouth().parseLongitude(text);
            m_bounds.getWestSouth().setCoordRefSystem(getCoordRefSystem());
            #ifdef WEATHER_DEBUG
            std::cout << "ParseContext::westBoundLongitude "
                      << get_id()
                      << " " << text
                      << " " << m_bounds.getWestSouth().getLongitude()
                      << " " << getCoordRefSystem().identifier() << std::endl;
            #endif
        }
        break;
    case ParseContext::eastBoundLongitude:
        if (m_parseLevel.size() == 3
         && getCoordRefSystem()) {      // only useful with crs defined)
            m_bounds.getEastNorth().parseLongitude(text);
            m_bounds.getEastNorth().setCoordRefSystem(getCoordRefSystem());
            #ifdef WEATHER_DEBUG
            std::cout << "ParseContext::eastBoundLongitude "
                      << get_id()
                      << " " << text
                      << " " << m_bounds.getEastNorth().getLongitude()
                      << " " << getCoordRefSystem().identifier() << std::endl;
            #endif
        }
        break;
    case ParseContext::southBoundLatitude:
        if (m_parseLevel.size() == 3
         && getCoordRefSystem()) {      // only useful with crs defined
            m_bounds.getWestSouth().parseLatitude(text);
            m_bounds.getWestSouth().setCoordRefSystem(getCoordRefSystem());
            #ifdef WEATHER_DEBUG
            std::cout << "ParseContext::southBoundLatitude "
                      << get_id()
                      << " " << text
                      << " " << m_bounds.getWestSouth().getLatitude()
                      << " " << getCoordRefSystem().identifier() << std::endl;
            #endif
        }
        break;
    case ParseContext::northBoundLatitude:
        if (m_parseLevel.size() == 3
         && getCoordRefSystem()) {      // only useful with crs defined
            m_bounds.getEastNorth().parseLatitude(text);
            m_bounds.getEastNorth().setCoordRefSystem(getCoordRefSystem());
            #ifdef WEATHER_DEBUG
            std::cout << "ParseContext::northBoundLatitude "
                      << get_id()
                      << " " << text
                      << " " << m_bounds.getEastNorth().getLatitude()
                      << " " << getCoordRefSystem().identifier() << std::endl;
            #endif
        }
        break;
    case ParseContext::Style:
        break;
    case ParseContext::LegendURL:
        break;
    case ParseContext::Format:
        break;
    case ParseContext::OnlineResource:
        break;
    case ParseContext::BoundingBox:
        break;
    case ParseContext::Dimension:
        parseDimension(text);
        break;
    case ParseContext::Attribution:
        m_attribution = text;
        break;
    case ParseContext::MinScaleDenominator:
        break;
    case ParseContext::MaxScaleDenominator:
        break;
    }
}

void
WebMapProduct::parseDimension(const Glib::ustring& text)
{
    std::vector<std::string> parts;
    StringUtils::split(text, '/', parts);
    if (parts.size() == 3) {
        m_timeDimStart = parts[0];
        m_timeDimEnd = parts[1];
        m_timeDimPeriod = parts[2];
        m_timePeriodSec = periodSeconds(m_timeDimPeriod);
        #ifdef WEATHER_DEBUG
        std::cout << "WebMapProduct::parseDimension "
                  << get_id()
                  << " got " << text
                  << " end " << m_timeDimEnd
                  << " dim " << m_timeDimPeriod
                  << " period " << m_timePeriodSec << "s"
                  << std::endl;
        #endif
    }
}


/*
D.3 periods
An ISO 8601 Period is used to indicate the time resolution of the available data. The ISO 8601 format for
representing a period of time is used to represent the resolution: Designator P (for Period), number of years Y,
months M, days D, time designator T, number of hours H, minutes M, seconds S. Unneeded elements may be
omitted.
EXAMPLE 1 P1Y — 1 year
EXAMPLE 2 P1M10D — 1 month plus 10 days
EXAMPLE 3 PT2H — 2 hours
EXAMPLE 4 PT1.5S — 1.5 seconds
 * but no one elaborates on just P ...(when we guess from end that is ~30min behind now, what just may be delay, no live images :( )
*/
uint64_t
WebMapProduct::periodSeconds(const Glib::ustring& timeDimPeriod)
{
    uint64_t timePeriodSec{0};
    int val = 0;
    bool time = false;
    for (uint32_t i = 0; i < timeDimPeriod.size(); ++i) {
        gunichar c = timeDimPeriod.at(i);
        if (i == 0 && c != 'P') {
            break;
        }
        else {
            if (c == 'T') {
                time = true;
            }
            else {
                if (g_unichar_isdigit(c)) {    // wont recognize decimal
                    val = val * 10 + g_unichar_xdigit_value(c);
                }
                else if (!time) {
                    if (c == 'D') {
                        timePeriodSec += val * 24 * 60 * 60;
                        val = 0;
                    }
                    else if (c == 'M') {  // ~ estimate
                        timePeriodSec += val * 30 * 24 * 60 * 60;
                        val = 0;
                    }
                    else if (c == 'Y') {  // ~ estimate
                        timePeriodSec += val  * 364 * 24 * 60 * 60;
                        val = 0;
                    }
                }
                else {
                    if (c == 'H') {
                        timePeriodSec += val * 60 * 60;
                        val = 0;
                    }
                    else if (c == 'M') {
                        timePeriodSec += val * 60;
                        val = 0;
                    }
                    else if (c == 'S') {
                        timePeriodSec += val;
                        val = 0;
                    }
                }
            }
        }
    }
    if (timePeriodSec < m_webMapService->getMinPeriodSec()) {
        timePeriodSec = m_webMapService->getMinPeriodSec();
    }
    return timePeriodSec;
}

bool
WebMapProduct::is_displayable()
{
    bool displayable = m_crs;   // works when crs is defined
    return displayable;
}

bool
WebMapProduct::is_latest()
{
    if (!m_timeDimEnd.empty()) {
        auto utcLatest = Glib::DateTime::create_from_iso8601(m_timeDimEnd, Glib::TimeZone::create_utc());
        if (utcLatest) {
            utcLatest = utcLatest.add_seconds(m_timePeriodSec);
            auto now = Glib::DateTime::create_now_utc();
            now = now.add_seconds(-m_webMapService->getDelaySec()); // compare with past as service introduce delay
            #ifdef WEATHER_DEBUG
            std::cout << "WebMapProduct::is_latest "
                      << get_id()
                      << " period " << m_timePeriodSec
                      << " dimEnd " << m_timeDimEnd
                      << " latest " << utcLatest.format_iso8601()
                      << " now " << now.format_iso8601()
                      << " cmp " << now.compare(utcLatest)
                      << std::endl;
            #endif
            if (now.compare(utcLatest) >= 0) {   // we passed the expected time
                m_timeDimEnd = utcLatest.format_iso8601();
                return false;
            }
        }
    }
    return true;    // cant tell or already latest
}

Glib::RefPtr<Gdk::Pixbuf>
WebMapProduct::get_legend()
{
    return m_legendImage;
}

void
WebMapProduct::set_legend(Glib::RefPtr<Gdk::Pixbuf>& legendImage)
{
    m_legendImage = legendImage;
    m_signal_legend.emit(m_legendImage);
}

bool
WebMapProduct::latest(Glib::DateTime& dateTime, bool local)
{
   if (!m_timeDimEnd.empty()) {
        auto iso8601 = m_timeDimEnd;
        if (local) {    // as the string tz has precedence remove it
            auto pos = iso8601.find("Z");
            if (pos != Glib::ustring::npos) {
                iso8601.erase(pos, 1);
            }
        }
        auto tz = local ? Glib::TimeZone::create_local() : Glib::TimeZone::create_utc();
        auto utc = Glib::DateTime::create_from_iso8601(iso8601, tz);
        if (utc) {
            //std::cout << "RealEarthProduct::latest parsed " << iso8601 << " to utc " <<  utc.format("%F-%T") << std::endl;
            dateTime = utc.to_local();
            //std::cout << "RealEarthProduct::latest local " <<  dateTime.format("%F-%T") << std::endl;
            return true;
        }
        else {
            std::cout << "WebMapProduct::latest latest " << iso8601 << " not parsed" << std::endl;
        }
    }
    return false;
}


void
WebMapService::capabilities()
{
    auto message = std::make_shared<SpoonMessageDirect>(getAddress(), "");
    message->addQuery("service", "WMS");
    message->addQuery("version", "1.3.0");
    message->addQuery("request", "GetCapabilities");
    message->signal_receive().connect(sigc::mem_fun(*this, &WebMapService::inst_on_capabilities_callback));
    #ifdef WEATHER_DEBUG
    std::cout << "WebMapService::capabilities"
              << " message->get_url() " << message->get_url() << std::endl;
    #endif
    getSpoonSession()->send(message);
}

void
WebMapService::inst_on_capabilities_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message)
{
    if (!error.empty()) {
        std::cout << "WebMapService::inst_on_capabilities_callback capabilities " << error << std::endl;
        return;
    }
    if (status != SpoonMessage::OK) {
        std::cout << "WebMapService::inst_on_capabilities_callback capabilities response " << status << std::endl;
        return;
    }
    auto data = message->get_bytes();
    if (!data) {
        std::cout << "WebMapService::inst_on_capabilities_callback capabilities no data" << std::endl;
        return;
    }

    #ifdef SPOON_DEBUG
    std::cout << "WebMapService::inst_on_capabilities_callback len " <<  data->size() << std::endl;
    #endif
    NXMLParser parser(this);
    Glib::Markup::ParseContext context(parser);	// , Glib::Markup::ParseFlags::TREAT_CDATA_AS_TEXT
    m_products.clear();
    auto start = reinterpret_cast<const char*>(data->get_data());
    auto end = start + data->size();
    try {
        context.parse(start, end);  // could feed this incrementally
        context.end_parse();
    }
    catch (const Glib::MarkupError& ex) {
        std::cerr << "WebMapService::inst_on_capabilities_callback error " << ex.what() << std::endl;
    }
    #ifdef WEATHER_DEBUG
    int usable = 0;
    for (auto entry : m_products) {
        auto prod = entry.second;
        if (prod->is_displayable()) {
            ++usable;
        }
        else {
            std::cout << "WebMapService::inst_on_capabilities_callback unusable  " << prod->get_id() << std::endl;
        }
    }
    std::cout << "WebMapService::inst_on_capabilities_callback got " << m_products.size() << " products usable " << usable << std::endl;
    #endif
    m_signal_products_completed.emit();
}


WebMapService::WebMapService(WeatherConsumer* consumer, const Glib::ustring& name, const Glib::ustring& address, int delaySec, uint32_t minPeriodSec)
: Weather(consumer)
, m_address{address}
, m_name{name}
, m_delaySec{delaySec}
, m_minPeriodSec{minPeriodSec}
{
}

void
WebMapService::request(const Glib::ustring& productId)
{
    auto wproduct = find_product(productId);
    auto product = std::dynamic_pointer_cast<WebMapProduct>(wproduct);
    if (!product) {
        return;
    }

    int image_size = m_consumer->get_weather_image_size() ;
    int image_size2 = image_size / 2;
    // always query in four steps
    // as we reduced the number of requests send them all at once
    CoordRefSystem crs84(CoordRefSystem::CRS_84);
    auto greenLon = product->getWestSouth().getCoordRefSystem().fromLinearLon(crs84.toLinearLon(0.0));
    auto equatorLat = product->getWestSouth().getCoordRefSystem().fromLinearLat(crs84.toLinearLat(0.0));
    if (product->getEastNorth().getLatitude() > 0.0) {    // query if needed
        if (product->getWestSouth().getLongitude() < 0.0) {
            double linLonWest = std::abs(product->getWestSouth().getLinearLongitude());
            int xOffs = static_cast<int>(linLonWest * image_size2);
            GeoBounds boundsWN{
                product->getWestSouth().getLongitude(), equatorLat
                , greenLon, product->getEastNorth().getLatitude()
                , product->getWestSouth().getCoordRefSystem()};
            auto requestWN = std::make_shared<WebMapImageRequest>(this
                        , boundsWN
                        , image_size2 - xOffs, 0
                        , xOffs, image_size2
                        , product);
            getSpoonSession()->send(requestWN);
        }
        if (product->getEastNorth().getLongitude() > 0.0) {
            double linLonEast = product->getEastNorth().getLinearLongitude();
            int xOffs = static_cast<int>(linLonEast * image_size2);
            GeoBounds boundsEN{
                greenLon, equatorLat
                , product->getEastNorth().getLongitude(), product->getEastNorth().getLatitude()
                , product->getEastNorth().getCoordRefSystem()
            };
            auto requestEN = std::make_shared<WebMapImageRequest>(this
                        , boundsEN
                        , image_size2, 0
                        , xOffs, image_size2
                        , product);
            #ifdef WEATHER_DEBUG
            std::cout << "WebMapService::request product " << product->get_id() << " url " << requestEN->get_url() << std::endl;
            #endif
            getSpoonSession()->send(requestEN);
        }
    }
    if (product->getWestSouth().getLatitude() < 0.0) {    // query if needed
        if (product->getWestSouth().getLongitude() < 0.0) {
            double linLonWest = std::abs(product->getWestSouth().getLinearLongitude());
            int xOffs = static_cast<int>(linLonWest * image_size2);
            GeoBounds boundsWS{
                product->getWestSouth().getLongitude(), product->getWestSouth().getLatitude()
                , greenLon, equatorLat
                , product->getEastNorth().getCoordRefSystem()
            };
            auto requestWS = std::make_shared<WebMapImageRequest>(this
                        , boundsWS
                        , image_size2 - xOffs, image_size2
                        , xOffs, image_size2
                        , product);
            getSpoonSession()->send(requestWS);
        }
        if (product->getEastNorth().getLongitude() > 0.0) {
            double linLonEast = product->getEastNorth().getLinearLongitude();
            int xOffs = static_cast<int>(linLonEast * image_size2);
            GeoBounds boundsES{
                greenLon, product->getWestSouth().getLatitude()
                , product->getEastNorth().getLongitude(), equatorLat
                , product->getWestSouth().getCoordRefSystem()
            };
            auto requestES = std::make_shared<WebMapImageRequest>(this
                        , boundsES
                        , image_size2, image_size2
                        , xOffs, image_size2
                        , product);
            getSpoonSession()->send(requestES);
        }
    }
}

void
WebMapService::check_product(const Glib::ustring& weatherProductId)
{
    if (!weatherProductId.empty() && !m_products.empty()) {
        auto wproduct = find_product(weatherProductId);
        auto product = std::dynamic_pointer_cast<WebMapProduct>(wproduct);
        if (product && !product->is_latest()) {
            #ifdef WEATHER_DEBUG
            std::cout << "WebMapService::check_product requested" << std::endl;
            #endif
            request(weatherProductId);
        }
    }
}

Glib::RefPtr<Gdk::Pixbuf>
WebMapService::get_legend(std::shared_ptr<WeatherProduct>& product)
{
    Glib::RefPtr<Gdk::Pixbuf> legend = product->get_legend();
    if (!legend) {
        auto webMapProduct = std::dynamic_pointer_cast<WebMapProduct>(product);
        if (!webMapProduct) {
            std::cerr << "WebMapService::get_legend got wrong product type" << std::endl;
            return Glib::RefPtr<Gdk::Pixbuf>();
        }
        auto legendURL = webMapProduct->get_legend_url();
        auto legendReq = std::make_shared<SpoonMessageDirect>(legendURL, "");
        legendReq->signal_receive().connect(
                    sigc::bind<std::shared_ptr<WeatherProduct>>(
                        sigc::mem_fun(*this, &WebMapService::inst_on_legend_callback), webMapProduct));
        #ifdef WEATHER_DEBUG
        std::cout << "WebMapService::get_legend " << legendURL  << std::endl;
        #endif
        getSpoonSession()->send(legendReq);
    }
    return legend;
}


NXMLParser::NXMLParser(WebMapService* webMapService)
: m_webMapService{webMapService}
, m_webMapProduct{}
{
}

void
NXMLParser::on_start_element(Glib::Markup::ParseContext& context,
        const Glib::ustring& element_name,
        const Glib::Markup::Parser::AttributeMap& attributes)
{
    if (element_name == "Layer") {
        auto queryable = attributes.find("queryable");
        if (queryable != attributes.end()
         && queryable->second == "1") {    // ignore global layer, and not queryable ?
            m_webMapProduct = std::make_shared<WebMapProduct>(m_webMapService);
        }
    }
    else if (m_webMapProduct) {
        m_webMapProduct->start_element(context, element_name, attributes);
    }
}

void
NXMLParser::on_end_element(Glib::Markup::ParseContext& context,
		const Glib::ustring& element_name)
{
    if (element_name == "Layer") {
        if (m_webMapProduct) {
            m_webMapService->add_product(m_webMapProduct);
        }
        m_webMapProduct.reset();
    }
    if (m_webMapProduct) {
        m_webMapProduct->end_element(context, element_name);
    }
}

void
NXMLParser::on_text(Glib::Markup::ParseContext& context,
		const Glib::ustring& text)
{
    if (m_webMapProduct) {
        auto repl = replaceAll(text, "&#13;", "\r");
        repl = replaceAll(repl, "&#10;", "\n");
        m_webMapProduct->text(context, repl);
    }
}

// allow replacement of some extra entities
Glib::ustring
NXMLParser::replaceAll(const Glib::ustring& text, const Glib::ustring& replace, const Glib::ustring& with)
{
    Glib::ustring ret = text;
    size_t pos = 0;
    while ((pos = ret.find(replace, pos)) != Glib::ustring::npos) {
         ret.replace(pos, replace.length(), with);
         pos += with.length();
    }
    return ret;
}


void
NXMLParser::on_error(Glib::Markup::ParseContext& context,
		const Glib::MarkupError& error)
{
	throw error;
}
