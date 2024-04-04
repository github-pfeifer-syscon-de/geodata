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

#include <sstream>      // std::ostringstream
#include <iostream>
#include <iomanip>
#include <strings.h>
#include <memory.h>
#include <JsonHelper.hpp>


#include "RealEarth.hpp"
#include "MapProjection.hpp"
#include "LocaleContext.hpp"

// it might be an option to use WMS here as well
//   but the current implementation has some nice features like query avail times...
// https://realearth.ssec.wisc.edu/cgi-bin/mapserv?map=SNOWDEPTH24.map&service=wms&version=1.3.0&request=GetCapabilities

RealEarthImageRequest::RealEarthImageRequest(RealEarth* realEarth
    , double south, double west, double north, double east
    , int pixX, int pixY, int pixWidth, int pixHeight
    , std::shared_ptr<RealEarthProduct>& product)
: WeatherImageRequest(realEarth->get_base_url(), "api/image", realEarth)
, m_realEarth{realEarth}
, m_south{south}
, m_west{west}
, m_north{north}
, m_east{east}
, m_pixX{pixX}
, m_pixY{pixY}
, m_pixWidth{pixWidth}
, m_pixHeight{pixHeight}
{
    build_url(product);
    signal_receive().connect(
        sigc::mem_fun(*realEarth, &RealEarth::inst_on_image_callback));
}

void
RealEarthImageRequest::build_url(std::shared_ptr<RealEarthProduct>& product)
{
    addQuery("products", product->get_id());
    Glib::ustring bound;
    LocaleContext localectx(LC_NUMERIC);
    if (localectx.set(LocaleContext::en_US)) {
        bound = Glib::ustring::sprintf("%.3f,%.3f,%.3f,%.3f"
                , m_south, m_west, m_north, m_east);
    }
    else {
        bound = Glib::ustring::sprintf("%d,%d,%d,%d"
            ,(int)m_south, (int)m_west, (int)m_north, (int)m_east);
    }
    #ifdef WEATHER_DEBUG
    std::cout << "Bounds " << bound << std::endl;
    #endif
    addQuery("bounds", bound);
    std::vector<Glib::ustring> times = product->get_times();
    if (!times.empty()) {
        Glib::ustring time = times[times.size()-1];
        addQuery("time", time);
    }
    addQuery("width", Glib::ustring::sprintf("%d", m_pixWidth));
    addQuery("height", Glib::ustring::sprintf("%d", m_pixWidth));
}

// undo mercator mapping (correctly named coordinate transform) of pix.
//  By scanning every linear latitude, transform it into a index for the mercator map
//  and copying this row into weather_pix at the right position.
//  This expects tiles aligned to equator.
void
RealEarthImageRequest::mapping(Glib::RefPtr<Gdk::Pixbuf> pix, Glib::RefPtr<Gdk::Pixbuf>& weather_pix)
{
    // create a compatible pixmap to clear a requested area
    Glib::RefPtr<Gdk::Pixbuf> clearPix = Gdk::Pixbuf::create(weather_pix->get_colorspace(), weather_pix->get_has_alpha(), weather_pix->get_bits_per_sample(), pix->get_width(), 1);
    clearPix->fill(0x0);   // transp. black
	bool isnorth = m_north > 0.0;
    //std::string inname = Glib::ustring::sprintf("/home/rpf/in%f%f.png", std::floor(m_west), std::floor(m_north));
    //pix->save(inname, "png");
    double pix_height = pix->get_height();
    MapProjectionMercator projectMercator;
	double relMercOrigin = projectMercator.fromLinearLatitude((isnorth ? m_north : std::abs(m_south)) / 90.0);
	for (int linY = 0; linY < pix_height; ++linY) {
	    double realRelLat = isnorth
		            ? ((double)(pix_height - linY) / pix_height)
		            : ((double)linY / pix_height);
	    double relMerc = projectMercator.fromLinearLatitude(realRelLat);
	    if (relMerc < relMercOrigin) {
            // relMerc is now right for a full view 0..90 -> 0-1
            double relMercMap = isnorth
                                ? 1.0 - (relMerc / relMercOrigin)
                                : (relMerc / relMercOrigin);
            // relMercMap adjust mercator to our map
            int mercImageY = (int)(relMercMap * pix_height);
            //std::cout << "Map "
            //          << realRelLat
            //          << " to " << relMerc
            //          << " i " << y
            //          << " to " << mercImageY
            //          << std::endl;
            if (mercImageY >= 0 && mercImageY < pix_height) {     // just to be safe (better than to crash)
                pix->copy_area(0, mercImageY, pix->get_width(), 1, weather_pix, get_pixX(), get_pixY()+linY);
            }
            else {
                std::cout << "Generated y " << mercImageY << " while mapping exceeded size " << pix->get_height() << std::endl;
            }
    	}
        else {
            clearPix->copy_area(0, 0, clearPix->get_width(), 1, weather_pix, get_pixX(), get_pixY()+linY);
        }
    }
}

int
RealEarthImageRequest::get_pixX()
{
    return m_pixX;
}

int
RealEarthImageRequest::get_pixY()
{
    return m_pixY;
}


RealEarthProduct::RealEarthProduct(JsonObject* obj)
: WeatherProduct()
, m_legend{}
{
    m_id = json_object_get_string_member(obj, "id");
    m_dataid = json_object_get_string_member(obj, "dataid");
    m_name = json_object_get_string_member(obj, "name");
    m_description = json_object_get_string_member(obj, "description");
    m_type = json_object_get_string_member(obj, "type");
    m_outputtype = json_object_get_string_member(obj, "outputtype");
    m_seedlatbound = json_object_get_double_member_with_default(obj, "seedlatbound", MAX_MERCATOR_LAT);
    auto times = json_object_get_array_member(obj, "times");
    guint len = json_array_get_length(times);
    for (guint i = 0; i < len; ++i) {
        Glib::ustring time = json_array_get_string_element(times, i);
        if (!time.empty()) {
            m_times.push_back(time);
        }
    }
    GeoBounds bounds{-180.0, -m_seedlatbound, 180.0, m_seedlatbound, CoordRefSystem::CRS_84};
    m_bounds = bounds;   // avoid querying extend as it doesn't reveal much
}

// info about it is displayable for us
bool
RealEarthProduct::is_displayable()
{
    return m_outputtype == "png24" && !m_times.empty();
}

// check if the given latest is the contained, if not is it added
bool
RealEarthProduct::is_latest(const Glib::ustring& latest)
{
    for (auto time : m_times) {
        if (time == latest) {
            return true;
        }
    }
    m_times.push_back(latest);
    return false;
}

/**
 *  return time for latest
 * @param dateTime set date&time to local for latest if possible
 * @return true -> date&time was set, false -> something went wrong
 */
bool
RealEarthProduct::latest(Glib::DateTime& dateTime)
{
    if (!m_times.empty()) {
        Glib::ustring latest = m_times[m_times.size()-1];
        Glib::ustring iso8601 = latest;
        auto pos = iso8601.find(".");
        if (pos == Glib::ustring::npos)  {
            std::cout << "RealEarthProduct::latest latest " << iso8601 << " no '.' found" << std::endl;
        }
        else {
            iso8601.replace(pos, 1, "T"); // make it iso
        }
        auto tz = Glib::TimeZone::create_utc();
        auto utc = Glib::DateTime::create_from_iso8601(iso8601, tz);
        if (utc) {
            //std::cout << "RealEarthProduct::latest parsed " << iso8601 << " to utc " <<  utc.format("%F-%T") << std::endl;
            dateTime = utc.to_local();
            //std::cout << "RealEarthProduct::latest local " <<  dateTime.format("%F-%T") << std::endl;
            return true;
        }
        else {
            std::cout << "RealEarthProduct::latest latest " << iso8601 << " not parsed" << std::endl;
        }
    }
    return false;
}

void
RealEarthProduct::set_extent(JsonObject* entry)
{
    Glib::ustring north = json_object_get_string_member_with_default(entry, "north", "85");
    Glib::ustring south = json_object_get_string_member_with_default(entry, "south", "-85");
    Glib::ustring west = json_object_get_string_member_with_default(entry, "west", "-180");
    Glib::ustring east = json_object_get_string_member_with_default(entry, "east", "180");
    Glib::ustring width = json_object_get_string_member_with_default(entry, "width", "1024");
    Glib::ustring height = json_object_get_string_member_with_default(entry, "height", "1024");
    #ifdef WEATHER_DEBUG
    std::cout << "WeatherProduct::set_extent str"
              << " north " << north
              << " south " << south
              << " west " << west
              << " east " << east
              << " width " << width
              << " height " << height
              << std::endl;
    #endif
    m_bounds.getEastNorth().parseLatitude(north);
    m_bounds.getEastNorth().parseLongitude(east);
    m_bounds.getEastNorth().setCoordRefSystem(CoordRefSystem::CRS_84);
    m_bounds.getWestSouth().parseLatitude(south);
    m_bounds.getWestSouth().parseLongitude(west);
    m_bounds.getWestSouth().setCoordRefSystem(CoordRefSystem::CRS_84);
    m_extent_width = std::stoi(width);
    m_extent_height = std::stoi(height);
    #ifdef WEATHER_DEBUG
    std::cout << "WeatherProduct::set_extent num"
              << " bounds (eswn) " << m_bounds.printValue()
              << " width " << m_extent_width
              << " height " << m_extent_height
              << std::endl;
    #endif
}

Glib::RefPtr<Gdk::Pixbuf>
RealEarthProduct::get_legend() {
    return m_legend;
}

void
RealEarthProduct::set_legend(Glib::RefPtr<Gdk::Pixbuf>& legend) {
    m_legend = legend;
    m_signal_legend.emit(m_legend);
}

RealEarth::RealEarth(WeatherConsumer* consumer, const Glib::ustring& base_url)
: Weather(consumer)
, m_base_url{base_url}
{
}

void
RealEarth::inst_on_capabilities_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message)
{
    if (!error.empty()) {
        std::cout << "error capabilities " << error << std::endl;
        return;
    }
    if (status != SpoonMessage::OK) {
        std::cout << "Error capabilities response " << status << std::endl;
        return;
    }
    auto data = message->get_bytes();
    if (!data) {
        std::cout << "Error capabilities no data" << std::endl;
        return;
    }
    try {
        JsonHelper parser;
        parser.load_data(data);
        JsonArray* array = parser.get_root_array();
        guint len = json_array_get_length(array);
        m_products.clear();
        for (guint i = 0; i < len; ++i) {
            JsonObject* jsProduct = parser.get_array_object(array, i);
            auto product = std::make_shared<RealEarthProduct>(jsProduct);
            add_product(product);
        }
        m_signal_products_completed.emit();
    }
    catch (const JsonException& ex) {
        char head[64];
        strncpy(head, (const gchar*)data->get_data(), std::min((guint)sizeof(head)-1, data->size()));
        std::cout << "Unable to parse " << head << "... " << ex.what() << std::endl;
    }
}

void
RealEarth::capabilities()
{
    auto message = std::make_shared<SpoonMessageDirect>(get_base_url(), "api/products");
    message->addQuery("search", "global");
    message->addQuery("timespan", "-8h");
    message->signal_receive().connect(sigc::mem_fun(*this, &RealEarth::inst_on_capabilities_callback));
    #ifdef WEATHER_DEBUG
    std::cout << "Weather::capabilities"
              << " message->get_url() " << message->get_url() << std::endl;
    #endif
    getSpoonSession()->send(message);
}


void
RealEarth::inst_on_latest_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message)
{
    if (!error.empty()) {
        std::cout << "error latest " << error << std::endl;
        return;
    }
    if (status != SpoonMessage::OK) {
        std::cout << "Error latest response " << status << std::endl;
        return;
    }
    auto data = message->get_bytes();
    if (!data) {
        std::cout << "Error latest no data" << std::endl;
        return;
    }
    try {
        JsonHelper parser;
        parser.load_data(data);

        JsonObject* object = parser.get_root_object();
        std::vector<Glib::ustring> keys = parser.get_keys(object);
        for (auto key : keys) {
            const gchar* latest = json_object_get_string_member(object, key.c_str());
            #ifdef WEATHER_DEBUG
            std::cout << "Weather::inst_on_latest_callback"
                      <<  " item " <<  key
                      <<  " latest " << latest << std::endl;
            #endif
            auto prod = std::dynamic_pointer_cast<RealEarthProduct>(find_product(key));
            if (prod) {
                if (!prod->is_latest(latest)) {
                    request(key);  // as the given latest is not latest queue a request
                }
            }
        }
    }
    catch (const JsonException& ex) {
        char head[64];
        strncpy(head, (const gchar*)data->get_data(), std::min((guint)sizeof(head)-1, data->size()));
        std::cout << "Unable to parse " << head << "... " << ex.what() << std::endl;
    }

}


// queue a latest request and if it not do a request (this is not useful for products that are not currently displayed!)
void
RealEarth::check_product(const Glib::ustring& weatherProductId)
{
    if (!weatherProductId.empty() && !m_products.empty()) { // while not ready ignore request
        auto product= std::make_shared<SpoonMessageDirect>(get_base_url(), "api/latest");
        product->addQuery("products", weatherProductId);
        product->signal_receive().connect(sigc::mem_fun(*this, &RealEarth::inst_on_latest_callback));
        #ifdef WEATHER_DEBUG
        std::cout << "RealEarth::check_product"
                  << " url " << product->get_url() << std::endl;
        #endif
        getSpoonSession()->send(product);
    }
}

void
RealEarth::inst_on_extend_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message)
{
    if (!error.empty()) {
        std::cout << "error extend " << error << std::endl;
        return;
    }
    if (status != SpoonMessage::OK) {
        std::cout << "Error extend response " << status << std::endl;
        return;
    }
    auto data = message->get_bytes();
    if (!data) {
        std::cout << "Error extend no data" << std::endl;
        return;
    }
    try {
        JsonHelper parser;
        parser.load_data(data);
        JsonObject* object = parser.get_root_object();
        std::vector<Glib::ustring> keys = parser.get_keys(object);
        for (auto key : keys) {
            JsonObject* entry = parser.get_object(object, key);
            #ifdef WEATHER_DEBUG
            std::cout << "Weather::inst_on_extend_callback got"
                      <<  " item " <<  key << std::endl;
            #endif
            auto prod = std::dynamic_pointer_cast<RealEarthProduct>(find_product(key));
            if (prod) {
                prod->set_extent(entry);
            }
        }
    }
    catch (const JsonException& ex) {
        char head[64];
        strncpy(head, (const gchar*)data->get_data(), std::min((guint)sizeof(head)-1, data->size()));
        std::cout << "Unable to parse " << head << "... " << ex.what() << std::endl;
    }

    // check if we have a queued request that may now work
    if (!queued_product_request.empty()) {
        auto prodReq = queued_product_request;
        queued_product_request = "";
        request(prodReq);
    }
}

void
RealEarth::get_extend(std::shared_ptr<RealEarthProduct>& product)
{
    auto extend = std::make_shared<SpoonMessageDirect>(get_base_url(), "api/extents");
    extend->addQuery("products", product->get_id());
    extend->signal_receive().connect(sigc::mem_fun(*this, &RealEarth::inst_on_extend_callback));
    #ifdef WEATHER_DEBUG
    std::cout << "Weather::get_extend " << extend->get_url()  << std::endl;
    #endif
    getSpoonSession()->send(extend);
}

Glib::RefPtr<Gdk::Pixbuf>
RealEarth::get_legend(std::shared_ptr<WeatherProduct>& product)
{
    Glib::RefPtr<Gdk::Pixbuf> legend = product->get_legend();
    if (!legend) {
        auto earthProduct = std::dynamic_pointer_cast<RealEarthProduct>(product);
        if (earthProduct) {
            auto legend = std::make_shared<SpoonMessageDirect>(get_base_url(), "api/legend");
            legend->addQuery("products", product->get_id());
            legend->signal_receive().connect(
                    sigc::bind<std::shared_ptr<RealEarthProduct>>(
                        sigc::mem_fun(*this, &RealEarth::inst_on_legend_callback), earthProduct));
            #ifdef WEATHER_DEBUG
            std::cout << "RealEarth::get_legend " << legend->get_url()  << std::endl;
            #endif
            getSpoonSession()->send(legend);
        }
        else {
            std::cerr << "the passed instance for product was not of type RealEarthProduct" << std::endl;
        }
    }
    return legend;
}

// example of the "C-way"
//void
//RealEarth::static_on_image_callback(GObject *source, GAsyncResult *result, gpointer user_data)
//{
//    RealEarth* weather = (Weather*)user_data;
//    //std::cout << "WeatherImageRequest::static_on_load_callback " << request << std::endl;
//    SoupMessage* msg = soup_session_get_async_result_message(SOUP_SESSION(source), result);
//    SoupStatus stat = soup_message_get_status(msg);
//    #ifdef WEATHER_DEBUG
//    std::cout << "WeatherImageRequest::static_on_load_callback status " << stat << std::endl;
//    #endif
//    if (stat == SOUP_STATUS_OK) {
//        GError *error = nullptr;
//        GBytes *bytes = soup_session_send_and_read_finish(SOUP_SESSION(source), result, &error);
//        if (error) {
//            std::cout << "error load " << error->message << std::endl;
//            g_error_free(error);
//        }
//        else  {
//            auto gbytes = Glib::wrap(bytes);
//            GUri* uri = soup_message_get_uri(msg);
//            const gchar* query = g_uri_get_query(uri);
//            if (query) {
//                GHashTable* hash = g_uri_parse_params(query, -1, "&", G_URI_PARAMS_PARSE_RELAXED, &error);
//                if (error) {
//                    std::cout << "error load parsing " << " query " << query << " error " << error->message << std::endl;
//                    g_error_free(error);
//                }
//                else {
//                    #ifdef WEATHER_DEBUG
//                    GList* keys = g_hash_table_get_keys(hash);
//                    for (GList* key = keys; key; key = key->next) {
//                        std::cout << "Got table " << (const char*)key->data << std::endl;
//                    }
//                    g_list_free(keys);
//                    #endif
//                    gpointer bounds = g_hash_table_lookup(hash, "bounds");
//                    if (bounds != nullptr) {
//                        gchar** comp = g_strsplit((const gchar*)bounds, ",", 4);
//                        if (!comp ) {
//                            std::cout << "error cannot split value " << bounds << "!" << std::endl;
//                        }
//                        else {
//                            double south = comp[0] ? std::stod(comp[0]) : 0.0;
//                            double west = comp[1] ? std::stod(comp[1]) : 0.0;
//                            double north = comp[2] ? std::stod(comp[2]) : 0.0;
//                            double east = comp[3] ? std::stod(comp[3]) : 0.0;
//                            g_strfreev(comp);
//                            gpointer swidth = g_hash_table_lookup(hash, "width");
//                            gpointer sheight = g_hash_table_lookup(hash, "height");
//                            if (!swidth || !sheight) {
//                                std::cout << "error query no width or height " << query << "!" << std::endl;
//                            }
//                            else {
//                                int width = std::stoi((const char*)swidth);
//                                int height = std::stoi((const char*)sheight);
//                                #ifdef WEATHER_DEBUG
//                                std::cout << "Got request"
//                                          << " south " << south
//                                          << " west " << west
//                                          << " north " << north
//                                          << " east " << east
//                                          << " width " << width
//                                          << " height " << height
//                                          << std::endl;
//                                #endif
//                                if (!weather) {
//                                    std::cout << "user_data no instance " << query << "!" << std::endl;
//                                }
//                                else {
//                                    // reconstruct request
//                                    WeatherImageRequest request(weather, south, west, north, east, width, height);
//                                    request.set_bytes(gbytes);
//                                    weather->inst_on_image_callback(request);
//                                }
//                            }
//                        }
//                    }
//                    else {
//                        std::cout << "error no query parameter bounds " << query << "!" << std::endl;
//                    }
//                    g_hash_table_unref(hash);
//                }
//            }
//        }
//    }
//    else {
//        std::cout << "Error load response " << stat << std::endl;
//    }
//}

void
RealEarth::request(const Glib::ustring& productId)
{
    auto product = std::dynamic_pointer_cast<RealEarthProduct>(find_product(productId));
    if (!product) {
        return;
    }
    if (product->get_extend_north() == 0.0) {
        queued_product_request = product->get_id();
        #ifdef WEATHER_DEBUG
        std::cout << "RealEarth::request queued " << product->get_id() << std::endl;
        #endif
        get_extend(product);
        return;
    }
    #ifdef WEATHER_DEBUG
    std::cout << "RealEarth::request request " << product->get_id() << std::endl;
    #endif

    int image_size = m_consumer->get_weather_image_size() ;
    int image_size2 = image_size / 2;
    // always query in four steps
    // as we reduced the number of requests send them all at once
    auto requestWN = std::make_shared<RealEarthImageRequest>(this
                ,0.0, -180.0
                ,product->get_extend_north(), 0.0
                ,0, 0
                ,image_size2, image_size2
                ,product);
    getSpoonSession()->send(requestWN);
    auto requestWS = std::make_shared<RealEarthImageRequest>(this
                ,product->get_extend_south(), -180.0
                ,0.0, 0.0
                ,0, image_size2
                ,image_size2, image_size2
                ,product);
    getSpoonSession()->send(requestWS);
    auto requestEN = std::make_shared<RealEarthImageRequest>(this
                ,0.0, 0.0
                ,product->get_extend_north(), 180.0
                ,image_size2, 0
                ,image_size2, image_size2
                ,product);
    getSpoonSession()->send(requestEN);
    auto requestES = std::make_shared<RealEarthImageRequest>(this
                ,product->get_extend_south(), 0.0
                ,0.0, 180.0
                ,image_size2, image_size2
                ,image_size2, image_size2
                ,product);
    getSpoonSession()->send(requestES);
}
