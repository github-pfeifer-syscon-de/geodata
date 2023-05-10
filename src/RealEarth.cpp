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
#include <cmath>
#include <strings.h>
#include <memory.h>


#include "RealEarth.hpp"
#include "JsonHelper.hpp"

const char* RealEarth::m_base_url{"https://realearth.ssec.wisc.edu/"};


RealEarth::RealEarth(WeatherConsumer* consumer)
: Weather(consumer)
, m_spoonSession{"map private use "}// last ws will add libsoup3
{
}

void
RealEarth::inst_on_capabilities_callback(const Glib::ustring& error, int status, SpoonMessage* message)
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
            auto product = std::make_shared<WeatherProduct>(jsProduct);
            m_products.push_back(product);
        }
        m_consumer->weather_products_ready();
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
    auto message = std::make_shared<SpoonMessage>(m_base_url, "api/products");
    message->addQuery("search", "global");
    message->addQuery("timespan", "-6h");
    message->signal_receive().connect(sigc::mem_fun(*this, &RealEarth::inst_on_capabilities_callback));
    #ifdef WEATHER_DEBUG
    std::cout << "Weather::capabilities"
              << " message->get_url() " << message->get_url() << std::endl;
    #endif
    m_spoonSession.send(message);
}


void
RealEarth::inst_on_latest_callback(const Glib::ustring& error, int status, SpoonMessage* message)
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
            auto prod = find_product(key);
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

void
RealEarth::inst_on_image_callback(const Glib::ustring& error, int status, SpoonMessage* message)
{
    if (!error.empty()) {
        std::cout << "error image " << error << std::endl;
        return;
    }
    if (status != SpoonMessage::OK) {
        std::cout << "Error image response " << status << std::endl;
        return;
    }
    auto data = message->get_bytes();
    if (!data) {
        std::cout << "Error image no data" << std::endl;
        return;
    }
    #ifdef WEATHER_DEBUG
    auto bytes = message->get_bytes();
    std::cout << "Weather load len "
              << bytes->size()
              << std::endl << dump(bytes->get_data(), std::min(64u, bytes->size()))
              << std::endl;
    #endif
    WeatherImageRequest* request = dynamic_cast<WeatherImageRequest*>(message);
    if (request) {
        if (m_consumer) {
            m_consumer->weather_image_notify(*request);
        }
    }
    else {
        std::cout << "Could not reconstruct weather request" << std::endl;
    }
}

// queue a latest request and if it not do a request (this is not useful for products that are not currently displayed!)
void
RealEarth::check_product(const Glib::ustring& weatherProductId)
{
    if (!weatherProductId.empty() && !m_products.empty()) { // while not ready ignore request
        auto product= std::make_shared<SpoonMessage>(m_base_url, "api/latest");
        product->addQuery("products", weatherProductId);
        product->signal_receive().connect(sigc::mem_fun(*this, &RealEarth::inst_on_latest_callback));
        #ifdef WEATHER_DEBUG
        std::cout << "RealEarth::check_product"
                  << " url " << product->get_url() << std::endl;
        #endif
        m_spoonSession.send(product);
    }
}

void
RealEarth::inst_on_extend_callback(const Glib::ustring& error, int status, SpoonMessage* message)
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
            auto prod = find_product(key);
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
RealEarth::get_extend(std::shared_ptr<WeatherProduct>& product)
{
    auto extend = std::make_shared<SpoonMessage>(m_base_url, "api/extents");
    extend->addQuery("products", product->get_id());
    extend->signal_receive().connect(sigc::mem_fun(*this, &RealEarth::inst_on_extend_callback));
    #ifdef WEATHER_DEBUG
    std::cout << "Weather::get_extend " << extend->get_url()  << std::endl;
    #endif
    m_spoonSession.send(extend);
}

void
RealEarth::inst_on_legend_callback(const Glib::ustring& error, int status, SpoonMessage* message, std::shared_ptr<WeatherProduct> product)
{
   if (!error.empty()) {
        std::cout << "error legend " << error << std::endl;
        return;
    }
    if (status != SpoonMessage::OK) {
        std::cout << "Error legend response " << status << std::endl;
        return;
    }
    auto data = message->get_bytes();
    if (!data) {
        std::cout << "Error legend no data" << std::endl;
        return;
    }
    Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
    loader->write(data->get_data(), data->size());
    loader->close();
    if (loader->get_pixbuf()) {
        auto pixbuf = loader->get_pixbuf();
        #ifdef WEATHER_DEBUG
        std::cout << "Loading legend pixbuf"
                  << " chan " << pixbuf->get_n_channels()
                  << " width " << pixbuf->get_width()
                  << " height " << pixbuf->get_height()
                  << std::endl;
        #endif
        if (product) {
            product->set_legend(pixbuf);
        }
    }
    else {
        std::cout << "Error loading legend pixbuf" << std::endl;
    }
}

Glib::RefPtr<Gdk::Pixbuf>
RealEarth::get_legend(std::shared_ptr<WeatherProduct>& product)
{
    Glib::RefPtr<Gdk::Pixbuf> legend = product->get_legend();
    if (!legend) {
        auto legend = std::make_shared<SpoonMessage>(m_base_url, "api/legend");
        legend->addQuery("products", product->get_id());
        legend->signal_receive().connect(
                sigc::bind<std::shared_ptr<WeatherProduct>>(
                    sigc::mem_fun(*this, &RealEarth::inst_on_legend_callback), product));
        #ifdef WEATHER_DEBUG
        std::cout << "RealEarth::get_legend " << legend->get_url()  << std::endl;
        #endif
        m_spoonSession.send(legend);
    }
    return legend;
}


double
RealEarth::normToRadians(double norm) {
	return norm * M_PI_2;
}

// not much to project in this case
double
RealEarth::xAxisProjection(double input)
{
    double xm = input / 180.0;
    return xm;
}

// do a spherical web-mercator projection
// works with relative values 0..1
double
RealEarth::yAxisProjection(double input)
{
    double ym = std::log(std::tan(M_PI_4 + normToRadians(input) / 2.0));
    return ym / M_PI;   // keep range 0...1
}

double
RealEarth::yAxisUnProjection(double input)
{
    double yr = 2.0 * (std::atan(std::exp(input * M_PI)) -  M_PI_4);
    return yr / (M_PI_2);   // keep range 0...1
}

std::string
RealEarth::dump(const guint8 *data, gsize size)
{
    std::ostringstream out;
    gsize offset = 0u;
    while (offset < size) {
        if (offset > 0u) {
            out << std::endl;
        }
        out << std::hex << std::setw(4) << std::setfill('0') << offset << ":";
        for (gsize i = 0; i < std::min(size-offset, (gsize)16u); ++i)  {
            out << std::setw(2) << std::setfill('0') << (int)data[offset+i] << " ";
        }
        out << std::dec << std::setw(1) << " ";
        for (gsize i = 0; i < std::min(size-offset, (gsize)16u); ++i)  {
            if (data[offset+i] >= 32 && data[offset+i] < 127) {
                out << data[offset+i];
            }
            else {
                out << ".";
            }
        }
        offset += 16u;
    }
    return out.str();
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
    std::shared_ptr<WeatherProduct> product = find_product(productId);
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
    auto requestWN = std::make_shared<WeatherImageRequest>(this
                ,0.0, -180.0
                ,product->get_extend_north(), 0.0
                ,0, 0
                ,image_size2, image_size2
                ,product);
    m_spoonSession.send(requestWN);
    auto requestWS = std::make_shared<WeatherImageRequest>(this
                ,product->get_extend_south(), -180.0
                ,0.0, 0.0
                ,0, image_size2
                ,image_size2, image_size2
                ,product);
    m_spoonSession.send(requestWS);
    auto requestEN = std::make_shared<WeatherImageRequest>(this
                ,0.0, 0.0
                ,product->get_extend_north(), 180.0
                ,image_size2, 0
                ,image_size2, image_size2
                ,product);
    m_spoonSession.send(requestEN);
    auto requestES = std::make_shared<WeatherImageRequest>(this
                ,product->get_extend_south(), 0.0
                ,0.0, 180.0
                ,image_size2, image_size2
                ,image_size2, image_size2
                ,product);
    m_spoonSession.send(requestES);
}

std::shared_ptr<WeatherProduct>
RealEarth::find_product(const Glib::ustring& weatherProductId)
{
    //std::cout << "RealEarth::find_product"
    //      << " prod " << weatherProductId
    //      << " products " << m_products.size() << std::endl;
    if (!weatherProductId.empty()) {
        for (auto prod : m_products) {
            if (prod->get_id() == weatherProductId) {
                return prod;
            }
        }
        if (!m_products.empty()) {
            std::cout << "RealEarth::find_product the requested product " << weatherProductId << " was not found!" << std::endl;
        }
    }
    return nullptr;
}
