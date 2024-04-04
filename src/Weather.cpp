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
#include <JsonHelper.hpp>
#include <StringUtils.hpp>

#include "Weather.hpp"

WebMapServiceConf::WebMapServiceConf(const Glib::ustring& name, const Glib::ustring& address, int delay_sec, const Glib::ustring& type, bool viewCurrentTime)
: m_name{name}
, m_address{address}
, m_delay_sec{delay_sec}
, m_type{type}
, m_viewCurrentTime{viewCurrentTime}
{
}

WeatherImageRequest::WeatherImageRequest(const Glib::ustring& host, const Glib::ustring& path, WeatherLog* weatherLog)
: SpoonMessageStream(host, path)
, m_weatherLog{weatherLog}
{
}


Glib::RefPtr<Gdk::Pixbuf>
WeatherImageRequest::get_pixbuf()
{
    GInputStream *stream = get_stream();
    #ifdef WEATHER_DEBUG
    std::cout << "WeatherImageRequest::get_pixbuf stream " << std::hex << stream << std::dec << std::endl;
    #endif
    if (stream) {
        try {
            Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
            GError *error = nullptr;
            unsigned char data[8192];
            while (true) {
                gssize len = g_input_stream_read(stream, data, sizeof(data), nullptr, &error);
                if (len <= 0) {
                    break;
                }
                if (error) {
                    if (m_weatherLog) {
                        m_weatherLog->logMsg(psc::log::Level::Error, Glib::ustring::sprintf("Error reading http %s", error->message));
                    }
                    g_error_free(error);
                    break;
                }
                loader->write(data, len);
            }
            #ifdef WEATHER_DEBUG
            std::cout << "WeatherImageRequest::get_pixbuf close " << std::endl;
            #endif
            g_input_stream_close(stream, nullptr, nullptr);
            loader->close();
            return loader->get_pixbuf();
        }
        catch (const Glib::Error& ex) {    // Gdk::PixbufError
            if (m_weatherLog) {
                m_weatherLog->logMsg(psc::log::Level::Error, Glib::ustring::sprintf("Error reading image pixmap %s", ex.what()));
            }
        }
    }
    else {
        if (m_weatherLog) {
            m_weatherLog->logMsg(psc::log::Level::Error, "WeatherRequest::get_pixbuf no data ");
        }
    }
    return Glib::RefPtr<Gdk::Pixbuf>();
}


Glib::ustring
WeatherProduct::get_id()
{
    return m_id;
}

Glib::ustring
WeatherProduct::get_name()
{
    return m_name;
}

WeatherProduct::type_signal_legend
WeatherProduct::signal_legend()
{
    return m_signal_legend;
}

Weather::Weather(WeatherConsumer* consumer)
: m_consumer{consumer}
{
}

WeatherConsumer*
Weather::get_consumer()
{
    return m_consumer;
}

void Weather::logMsg(
      psc::log::Level level
    , const Glib::ustring& msg
    , std::source_location source)
{
    if (m_log) {
        m_log->log(level, msg, source);
    }
    else {
        std::cout << source << msg << std::endl;
    }
}

void
Weather::inst_on_image_callback(const Glib::ustring& error, int status, SpoonMessageStream* message)
{
    if (!error.empty()) {
        logMsg(psc::log::Level::Warn, Glib::ustring::sprintf("error image %s", error));
        return;
    }
    if (status != SpoonMessage::OK) {
        logMsg(psc::log::Level::Warn,Glib::ustring::sprintf("Error image response %d", status));
        return;
    }
    auto stream = message->get_stream();
    if (!stream) {
        logMsg(psc::log::Level::Warn, "Error image no data");
        return;
    }
    auto request = dynamic_cast<WeatherImageRequest*>(message);
    if (request) {
        if (m_consumer) {
            m_consumer->weather_image_notify(*request);
        }
    }
    else {
        logMsg(psc::log::Level::Warn, "Could not reconstruct weather request");
    }
}

void
Weather::inst_on_legend_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message, std::shared_ptr<WeatherProduct> product)
{
   if (!error.empty()) {
        logMsg(psc::log::Level::Warn, Glib::ustring::sprintf("error legend %s", error));
        return;
    }
    if (status != SpoonMessage::OK) {
        logMsg(psc::log::Level::Warn, Glib::ustring::sprintf("Error legend response %d", status));
        return;
    }
    auto data = message->get_bytes();
    if (!data) {
        logMsg(psc::log::Level::Warn, "Error legend no data");
        return;
    }
    try {
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
            logMsg(psc::log::Level::Warn, "Error loading legend empty pixbuf");
        }
    }
    catch (const Glib::Error& ex) {
        logMsg(psc::log::Level::Warn, Glib::ustring::sprintf("Error reading legend pixmap %s",  ex.what()));
    }
}

void
Weather::add_product(std::shared_ptr<WeatherProduct> product)
{
    const Glib::ustring& productId = product->get_id();
    auto entry = std::pair<Glib::ustring, std::shared_ptr<WeatherProduct>>(productId, product);
    m_products.insert(entry);
}

std::shared_ptr<WeatherProduct>
Weather::find_product(const Glib::ustring& productId)
{
    auto entry = m_products.find(productId);
    if (entry != m_products.end()) {
        return (*entry).second;
    }
    return std::shared_ptr<WeatherProduct>();
}

std::vector<std::shared_ptr<WeatherProduct>>
Weather::get_products()
{
    std::vector<std::shared_ptr<WeatherProduct>> weatherProducts;
    weatherProducts.reserve(m_products.size());
    for (auto prod : m_products) {
        weatherProducts.push_back(prod.second);
    }
    #ifdef WEATHER_DEBUG
    std::cout << "Weather::get_products " << weatherProducts.size() << std::endl;
    #endif
    return weatherProducts;
}

std::shared_ptr<SpoonSession>
Weather::getSpoonSession()
{
    if (!spoonSession) {
        spoonSession = std::make_shared<SpoonSession>("map private use ");// last ws will add libsoup3
    }
    return spoonSession;
}

std::string
Weather::dump(const guint8 *data, gsize size)
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

Weather::type_signal_products_completed
Weather::signal_products_completed()
{
    return m_signal_products_completed;
}

void
Weather::setLog(const std::shared_ptr<psc::log::Log>& log)
{
    m_log = log;
}