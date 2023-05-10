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

#pragma once

#include <gtkmm.h>
#include <stdint.h>
#include <memory>
#include <json-glib/json-glib.h>
#include <vector>

#include "Spoon.hpp"

#undef WEATHER_DEBUG

class WeatherImageRequest;
class WeatherProduct;

class WeatherConsumer
{
public:
    virtual void weather_products_ready() = 0;
    virtual void weather_image_notify(WeatherImageRequest& request) = 0;
    virtual int get_weather_image_size() = 0;
};

class Weather;
class WeatherProduct;


class WeatherImageRequest
: public SpoonMessage
{
public:
    WeatherImageRequest(Weather* weather, double south, double west, double north, double east
            , int pixX, int pixY, int pixWidth, int pixHeight
            , std::shared_ptr<WeatherProduct>& product);
    WeatherImageRequest(const WeatherImageRequest& other) = default;
    virtual ~WeatherImageRequest() = default;
    Weather* get_weather() {
        return m_weather;
    }
    int get_pixX();
    int get_pixY();
    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf();
    void mapping(Glib::RefPtr<Gdk::Pixbuf> pix, Glib::RefPtr<Gdk::Pixbuf>& weather);
protected:
    void build_url(std::shared_ptr<WeatherProduct>& product);
private:
    Weather* m_weather;
    double m_south;
    double m_west;
    double m_north;
    double m_east;
    int m_pixX;
    int m_pixY;
    int m_pixWidth;
    int m_pixHeight;
};

class WeatherProduct
{
public:
    WeatherProduct(JsonObject* obj);
    virtual ~WeatherProduct() = default;
    Glib::ustring get_id() {
        return m_id;
    }
    Glib::ustring get_dataid() {
        return m_dataid;
    }
    Glib::ustring get_name() {
        return m_name;
    }
    Glib::ustring get_description() {
        return m_description;
    }
    std::vector<Glib::ustring> get_times() {
        return m_times;
    }
    double get_seedlatbound() {
        return m_seedlatbound;
    }
    bool is_displayable();
    bool is_latest(const Glib::ustring& latest);
    bool latest(Glib::DateTime& datetime);
    void set_extent(JsonObject* entry);
    double  get_extend_north() {
        return std::min(m_extent_north, m_seedlatbound);  // some images report 90 and can't handle it afterwards as it seems
    }
    double  get_extend_south() {
        return std::max(m_extent_south, -m_seedlatbound);
    }
    double get_extent_west() {
        return m_extent_west;
    }
    double get_extent_east() {
        return m_extent_east;
    }
    int get_extent_width() {
        return m_extent_width;
    }
    int get_extent_height() {
        return m_extent_height;
    }
    Glib::RefPtr<Gdk::Pixbuf> get_legend();
    void set_legend(const Glib::RefPtr<Gdk::Pixbuf>& legend);
    using type_signal_legend = sigc::signal<void(Glib::RefPtr<Gdk::Pixbuf>)>;
    type_signal_legend signal_legend();
protected:
    type_signal_legend m_signal_legend;

private:
    Glib::ustring m_id;
    Glib::ustring m_dataid; // this is the base e.g. globalir for all ir based images
    Glib::ustring m_name;
    Glib::ustring m_description;
    std::vector<Glib::ustring> m_times;
    Glib::ustring m_type;       // this is the representation e.g. "raster" for images, "shape" for symbols
    Glib::ustring m_outputtype; // png24 for íamges
    double m_extent_north{0.0};
    double m_extent_south{0.0};
    double m_extent_west{0.0};
    double m_extent_east{0.0};
    int m_extent_width{0};
    int m_extent_height{0};
    double m_seedlatbound;      // e.g. 85 for images limited to latitude north/south
    Glib::RefPtr<Gdk::Pixbuf> m_legend;
    static constexpr double MAX_MERCATOR_LAT = 85.0;   // beyond this simple/web-mercator mapping isn't useful
};


// used to implement a single weather service
class Weather
{
public:
    Weather(WeatherConsumer* consumer);
    virtual ~Weather() = default;
    WeatherConsumer* get_consumer();
    virtual double yAxisProjection(double input); // needs to be overridden for non linear mappings

    static std::vector<Glib::ustring> get_services();
    static std::shared_ptr<Weather> create_service(const Glib::ustring &name, WeatherConsumer* consumer);
    virtual Glib::ustring get_base_url() = 0;
    virtual void inst_on_capabilities_callback(const Glib::ustring& error, int status, SpoonMessage* message) = 0;
    virtual void inst_on_image_callback(const Glib::ustring& error, int status, SpoonMessage* message) = 0;
    virtual void check_product(const Glib::ustring& weatherProductId) = 0;
    virtual void capabilities() = 0;
    virtual void request(const Glib::ustring& productId) = 0;
    virtual std::shared_ptr<WeatherProduct> find_product(const Glib::ustring& weatherProductId) = 0;
    virtual std::vector<std::shared_ptr<WeatherProduct>> get_products() = 0;
    virtual Glib::RefPtr<Gdk::Pixbuf> get_legend(std::shared_ptr<WeatherProduct>& product) = 0;


protected:
    WeatherConsumer* m_consumer;

};