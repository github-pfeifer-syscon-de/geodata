/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2023 RPf
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
#include <psc_format.hpp>
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

WeatherImageRequest::WeatherImageRequest(const Glib::ustring& host, const Glib::ustring& path)
: SpoonMessageStream(host, path)
{
}


Glib::RefPtr<Gdk::Pixbuf>
WeatherImageRequest::get_pixbuf()
{
    GInputStream *stream = get_stream();
    psc::log::Log::logAdd(psc::log::Level::Debug, [&] {
        return psc::fmt::format("pixbuf stream {}", static_cast<void*>(stream));
    });
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
                    psc::log::Log::logAdd(psc::log::Level::Error, [&] {
                        return psc::fmt::format("Error reading http {}", error->message);
                    });
                    g_error_free(error);
                    break;
                }
                loader->write(data, len);
            }
            psc::log::Log::logAdd(psc::log::Level::Debug, [&] {
                return psc::fmt::format("pixbuf close {}", static_cast<void*>(stream));
            });
            g_input_stream_close(stream, nullptr, nullptr);
            loader->close();
            return loader->get_pixbuf();
        }
        catch (const Glib::Error& ex) {    // Gdk::PixbufError
            psc::log::Log::logAdd(psc::log::Level::Error, [&] {
                return psc::fmt::format("Error reading image pixmap {}", ex);
            });
        }
    }
    else {
        psc::log::Log::logAdd(psc::log::Level::Error, "WeatherRequest::get_pixbuf no data ");
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
        logMsg(psc::log::Level::Warn,Glib::ustring::sprintf("Error image response %d %s", status, SpoonMessage::decodeStatus(status)));
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
        logMsg(psc::log::Level::Warn, Glib::ustring::sprintf("Error legend response %d %s", status, SpoonMessage::decodeStatus(status)));
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
            logMsg(psc::log::Level::Debug, Glib::ustring::sprintf("Loading legend pixbuf chan %d width %d height %d", pixbuf->get_n_channels(), pixbuf->get_width(), pixbuf->get_height()));
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
    for (auto& prod : m_products) {
        weatherProducts.push_back(prod.second);
    }
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
        out << psc::fmt::format("{:04x}:", offset);
        for (gsize i = 0; i < std::min(size-offset, (gsize)16u); ++i)  {
            out << psc::fmt::format(" {:02x}", data[offset+i]);
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