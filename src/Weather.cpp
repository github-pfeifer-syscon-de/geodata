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
#include <GenericGlmCompat.hpp>
#include <sstream>      // std::ostringstream
#include <iostream>
#include <iomanip>
#include <cmath>
#include <strings.h>
#include <memory.h>


#include "Weather.hpp"
#include "RealEarth.hpp"
#include "JsonHelper.hpp"


WeatherImageRequest::WeatherImageRequest(Weather* weather
    , double south, double west, double north, double east
    , int pixX, int pixY, int pixWidth, int pixHeight
    , std::shared_ptr<WeatherProduct>& product)
: SpoonMessage(weather->get_base_url(), "api/image")
, m_weather{weather}
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
    signal_receive().connect(sigc::mem_fun(*weather, &Weather::inst_on_image_callback));
}

void
WeatherImageRequest::build_url(std::shared_ptr<WeatherProduct>& product)
{
    addQuery("products", product->get_id());
    std::string prev_loc = std::setlocale(LC_NUMERIC, nullptr);
    Glib::ustring bound ;
    if (std::setlocale(LC_NUMERIC, "English")) {  // "en_US.utf8" only 4 linux
        bound = Glib::ustring::sprintf("%.3f,%.3f,%.3f,%.3f"
                , m_south, m_west, m_north, m_east);
        #ifdef WEATHER_DEBUG
        std::cout << "Bounds " << bound << std::endl;
        #endif
        std::setlocale(LC_NUMERIC, prev_loc.c_str());  // restore
    }
    else {
        bound = Glib::ustring::sprintf("%d,%d,%d,%d"
            ,(int)m_south, (int)m_west, (int)m_north, (int)m_east);
    }
    addQuery("bounds", bound);
    std::vector<Glib::ustring> times = product->get_times();
    if (!times.empty()) {
        Glib::ustring time = times[times.size()-1];
        addQuery("time", time);
    }
    addQuery("width", Glib::ustring::sprintf("%d", m_pixWidth));
    addQuery("height", Glib::ustring::sprintf("%d", m_pixWidth));
}

Glib::RefPtr<Gdk::Pixbuf>
WeatherImageRequest::get_pixbuf()
{
    Glib::RefPtr<Glib::ByteArray> bytes = get_bytes();
    //std::cout << "got " << byte_size << "bytes" << std::endl;
    if (bytes) {
        Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
        loader->write(bytes->get_data(), bytes->size());
        loader->close();
        return loader->get_pixbuf();
    }
    else {
        std::cout << "WeatherRequest::get_pixbuf no data " << std::endl;
    }
    return Glib::RefPtr<Gdk::Pixbuf>();
}

// undo mercator mapping (correctly named coordinate transform) of pix.
//  By scanning every linear latitude, transform it into a index for the mercator map
//  and copying this row into weather_pix at the right position.
//  This expects tiles aligned to equator.
void
WeatherImageRequest::mapping(Glib::RefPtr<Gdk::Pixbuf> pix, Glib::RefPtr<Gdk::Pixbuf>& weather_pix)
{
    // create a compatible pixmap to clear a requested area
    Glib::RefPtr<Gdk::Pixbuf> clearPix = Gdk::Pixbuf::create(weather_pix->get_colorspace(), weather_pix->get_has_alpha(), weather_pix->get_bits_per_sample(), pix->get_width(), 1);
    clearPix->fill(0x0);   // transp. black
	bool isnorth = m_north > 0.0;
    //std::string inname = Glib::ustring::sprintf("/home/rpf/in%f%f.png", std::floor(m_west), std::floor(m_north));
    //pix->save(inname, "png");
    double pix_height = pix->get_height();
	double relMercOrigin = m_weather->yAxisProjection((isnorth ? m_north : std::abs(m_south)) / 90.0);
	for (int linY = 0; linY < pix_height; ++linY) {
	    double realRelLat = isnorth
		            ? ((double)(pix_height - linY) / pix_height)
		            : ((double)linY / pix_height);
	    double relMerc = m_weather->yAxisProjection(realRelLat);
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
WeatherImageRequest::get_pixX()
{
    return m_pixX;
}

int
WeatherImageRequest::get_pixY()
{
    return m_pixY;
}

WeatherProduct::WeatherProduct(JsonObject* obj)
: m_legend{}
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
    m_extent_south = -m_seedlatbound;   // avoid querying extend as it doesn't reveal much
    m_extent_west = -180.0;
    m_extent_north = m_seedlatbound;
    m_extent_east = 180.0;
}

// info about it is displayable for us
bool
WeatherProduct::is_displayable()
{
    return m_outputtype == "png24" && !m_times.empty();
}

// check if the given latest is the contained, if not is it added
bool
WeatherProduct::is_latest(const Glib::ustring& latest)
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
WeatherProduct::latest(Glib::DateTime& dateTime)
{
    if (!m_times.empty()) {
        Glib::ustring latest = m_times[m_times.size()-1];
        Glib::ustring iso8601 = latest;
        auto pos = iso8601.find(".");
        if (pos == Glib::ustring::npos)  {
            std::cout << "WeatherProduct::latest latest " << iso8601 << " no '.' found" << std::endl;
        }
        else {
            iso8601.replace(pos, 1, "T"); // make it iso
        }
        auto utc = Glib::DateTime::create_from_iso8601(iso8601, Glib::TimeZone::create_utc());
        if (utc) {
            //std::cout << "WeatherProduct::latest parsed " << iso8601 << " to utc " <<  utc.format("%F-%T") << std::endl;
            dateTime = utc.to_local();
            //std::cout << "WeatherProduct::latest local " <<  dateTime.format("%F-%T") << std::endl;
            return true;
        }
        else {
            std::cout << "WeatherProduct::latest latest " << iso8601 << " not parsed" << std::endl;
        }

    }
    return false;
}

void
WeatherProduct::set_extent(JsonObject* entry)
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
    std::string prev_loc = std::setlocale(LC_NUMERIC, nullptr);
    std::cout << "WeatherProduct::set_extent prev "  << prev_loc << std::endl;
    if (std::setlocale(LC_NUMERIC, "English")) {    // "en_US.utf8"only linux
        m_extent_north = std::stod(north);
        m_extent_south = std::stod(south);
        m_extent_west = std::stod(west);
        m_extent_east = std::stod(east);
        std::setlocale(LC_NUMERIC, prev_loc.c_str());  // restore
    }
    else {
        std::cout << "setlocale failed!" << std::endl;
        m_extent_north = std::stoi(north);  // the integer part is most important
        m_extent_south = std::stoi(south);
        m_extent_west = std::stoi(west);
        m_extent_east = std::stoi(east);
    }
    m_extent_width = std::stoi(width);
    m_extent_height = std::stoi(height);
    #ifdef WEATHER_DEBUG
    std::cout << "WeatherProduct::set_extent num"
              << " north " << m_extent_north
              << " south " << m_extent_south
              << " west " << m_extent_west
              << " east " << m_extent_east
              << " width " << m_extent_width
              << " height " << m_extent_height
              << std::endl;
    #endif
}

Glib::RefPtr<Gdk::Pixbuf>
WeatherProduct::get_legend() {
    return m_legend;
}

void
WeatherProduct::set_legend(const Glib::RefPtr<Gdk::Pixbuf>& legend) {
    m_legend = legend;
    m_signal_legend.emit(m_legend);
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

double
    Weather::yAxisProjection(double input)
{
    return input;
}

std::vector<Glib::ustring>
Weather::get_services()
{
    std::vector<Glib::ustring> list;
    list.push_back(RealEarth::NAME);
    return list;
}

std::shared_ptr<Weather>
Weather::create_service(const Glib::ustring &name, WeatherConsumer* consumer)
{
    if (name == RealEarth::NAME) {
        return std::make_shared<RealEarth>(consumer);
    }
    return nullptr;
}
