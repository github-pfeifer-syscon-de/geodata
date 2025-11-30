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
#include <Log.hpp>

#include "Spoon.hpp"
#include "GeoCoordinate.hpp"

#undef WEATHER_DEBUG

class WeatherLog
{
public:
    virtual void logMsg(psc::log::Level level, const Glib::ustring& msg, std::source_location source = std::source_location::current()) = 0;
};

class WebMapServiceConf {
public:
    WebMapServiceConf(const Glib::ustring& name, const Glib::ustring& address, int delay_sec, const Glib::ustring& type, bool viewCurrentTime);
    explicit WebMapServiceConf(const WebMapServiceConf& oth) = delete;
    virtual ~WebMapServiceConf() = default;

    Glib::ustring getName() const
    {
        return m_name;
    }
    void setName(const Glib::ustring& name)
    {
        m_name = name;
    }
    Glib::ustring getAddress() const
    {
        return m_address;
    }
    void setAddress(const Glib::ustring& address)
    {
        m_address = address;
    }
    // beside the document period see WMS doc (the interval between updates)
    //   there is a delay (the place when this becomes visible is with the time
    //     dimension the latest values always is some minutes behind the actual time).
    //   e.g. the precipitation is announced with a interval P which presumably means ask any time, we will give you the nearest value.
    //     but if you try to ask for now, there is a error when requesting the images,
    //     some fiddling suggested the use of a 30 minutes delay
    //     and the use of a minimum interval of 5 minutes is due
    //     to the nature off our application as a resource friendly tool.
    int getDelaySec() const
    {
        return m_delay_sec;
    }
    void setDelaySec(int delay_sec)
    {
        m_delay_sec = delay_sec;
    }
    Glib::ustring getType() const
    {
        return m_type;
    }
    void setType(const Glib::ustring& type)
    {
        m_type = type;
    }
    // some WMS server offer prognosis but you prefer the current time
    bool isViewCurrentTime() const
    {
        return m_viewCurrentTime;
    }
    void setViewCurrentTime(bool viewCurrentTime)
    {
        m_viewCurrentTime = viewCurrentTime;
    }
private:
    Glib::ustring m_name;
    Glib::ustring m_address;
    int m_delay_sec;
    Glib::ustring m_type;
    bool m_viewCurrentTime;
};

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
protected:
private:
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
    virtual bool latest(Glib::DateTime& datetime) = 0;
    virtual bool is_displayable() = 0;
    virtual void set_legend(Glib::RefPtr<Gdk::Pixbuf>& pixbuf) = 0;
    virtual Glib::ustring get_dimension() = 0;

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
: public WeatherLog
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
    static std::string dump(const guint8 *data, gsize size);

    using type_signal_products_completed = sigc::signal<void()>;
    type_signal_products_completed signal_products_completed();
    void setLog(const std::shared_ptr<psc::log::Log>& log);
    void logMsg(psc::log::Level level, const Glib::ustring& msg, std::source_location source = std::source_location::current()) override;
protected:
    type_signal_products_completed m_signal_products_completed;
    WeatherConsumer* m_consumer;
    std::map<Glib::ustring, std::shared_ptr<WeatherProduct>> m_products;
    std::shared_ptr<SpoonSession> getSpoonSession();
    std::shared_ptr<psc::log::Log> m_log;
private:
    std::shared_ptr<SpoonSession> spoonSession;

};

