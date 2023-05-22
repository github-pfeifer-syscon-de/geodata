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
#include "GeoCoordinate.hpp"

#undef WEATHER_DEBUG

class WeatherImageRequest;
class WeatherProduct;

class WeatherConsumer
{
public:
    virtual void weather_image_notify(WeatherImageRequest& request) = 0;
    virtual int get_weather_image_size() = 0;
};

class Weather;
class WeatherProduct;


class WeatherImageRequest
: public SpoonMessageStream
{
public:
    WeatherImageRequest(const Glib::ustring& host, const Glib::ustring& path);
    virtual ~WeatherImageRequest() = default;
    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf();
    virtual void mapping(Glib::RefPtr<Gdk::Pixbuf> pix, Glib::RefPtr<Gdk::Pixbuf>& weather) = 0;

};

class WeatherProduct
{
public:
    WeatherProduct() = default;
    virtual ~WeatherProduct() = default;

    Glib::ustring get_id();
    Glib::ustring get_name();

    virtual Glib::RefPtr<Gdk::Pixbuf> get_legend() = 0;
    virtual Glib::ustring get_description() = 0;
    virtual bool latest(Glib::DateTime& datetime, bool local) = 0;
    virtual bool is_displayable() = 0;
    virtual void set_legend(Glib::RefPtr<Gdk::Pixbuf>& pixbuf) = 0;

    int get_extent_width() {
        return m_extent_width;
    }
    int get_extent_height() {
        return m_extent_height;
    }
    GeoCoordinate getWestSouth() {
        return m_bounds.getWestSouth();
    }
    GeoCoordinate getEastNorth() {
        return m_bounds.getEastNorth();
    }
    GeoBounds getBounds() {
        return m_bounds;
    }

    static constexpr auto MAX_MERCATOR_LAT{85.0};   // beyond this simple/web-mercator mapping isn't useful
    using type_signal_legend = sigc::signal<void(Glib::RefPtr<Gdk::Pixbuf>)>;
    type_signal_legend signal_legend();
protected:
    type_signal_legend m_signal_legend;

    Glib::ustring m_id;
    Glib::ustring m_name;
    GeoBounds m_bounds;
    int m_extent_width{0};
    int m_extent_height{0};
    double m_seedlatbound = MAX_MERCATOR_LAT; // e.g. 85 for images limited to latitude north/south

private:
};


// used to implement a single weather service
class Weather
{
public:
    Weather(WeatherConsumer* consumer);
    virtual ~Weather() = default;
    WeatherConsumer* get_consumer();

    virtual void check_product(const Glib::ustring& weatherProductId) = 0;
    virtual void capabilities() = 0;
    virtual void request(const Glib::ustring& productId) = 0;
    virtual Glib::RefPtr<Gdk::Pixbuf> get_legend(std::shared_ptr<WeatherProduct>& product) = 0;
    void inst_on_image_callback(const Glib::ustring& error, int status, SpoonMessageStream* message);
    void inst_on_legend_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message, std::shared_ptr<WeatherProduct> product);
    std::vector<std::shared_ptr<WeatherProduct>> get_products();
    std::shared_ptr<WeatherProduct> find_product(const Glib::ustring& productId);
    void add_product(std::shared_ptr<WeatherProduct> product);
    std::string dump(const guint8 *data, gsize size);

    using type_signal_products_completed = sigc::signal<void()>;
    type_signal_products_completed signal_products_completed();
protected:
    type_signal_products_completed m_signal_products_completed;
    WeatherConsumer* m_consumer;
    std::map<Glib::ustring, std::shared_ptr<WeatherProduct>> m_products;
    std::shared_ptr<SpoonSession> getSpoonSession();
private:
    std::shared_ptr<SpoonSession> spoonSession;

};

