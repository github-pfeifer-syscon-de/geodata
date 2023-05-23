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

#include <gtkmm.h>
#include <stdint.h>
#include <memory>
#include <json-glib/json-glib.h>
#include <vector>

#include "Weather.hpp"

class RealEarth;
class RealEarthProduct;

class RealEarthImageRequest
: public WeatherImageRequest
{
public:
    RealEarthImageRequest(RealEarth* weather, double south, double west, double north, double east
            , int pixX, int pixY, int pixWidth, int pixHeight
            , std::shared_ptr<RealEarthProduct>& product);
    virtual ~RealEarthImageRequest() = default;
    RealEarth* get_weather() {
        return m_realEarth;
    }
    int get_pixX();
    int get_pixY();
    void mapping(Glib::RefPtr<Gdk::Pixbuf> pix, Glib::RefPtr<Gdk::Pixbuf>& weather) override;
protected:
    void build_url(std::shared_ptr<RealEarthProduct>& product);
private:
    RealEarth* m_realEarth;
    double m_south;
    double m_west;
    double m_north;
    double m_east;
    int m_pixX;
    int m_pixY;
    int m_pixWidth;
    int m_pixHeight;
};


class RealEarthProduct
: public WeatherProduct
{
public:
    RealEarthProduct(JsonObject* obj);
    virtual ~RealEarthProduct() = default;

    Glib::ustring get_dataid() {
        return m_dataid;
    }
    Glib::ustring get_description() override {
        return m_description;
    }
    std::vector<Glib::ustring> get_times()
    {
        return m_times;
    }
    double get_extend_north() {
        return std::min(m_bounds.getEastNorth().getLatitude(), m_seedlatbound);  // some images report 90 and can't handle it afterwards as it seems
    }
    double get_extend_south() {
        return std::max(m_bounds.getWestSouth().getLatitude(), -m_seedlatbound);
    }
    double get_seedlatbound() {
        return m_seedlatbound;
    }
    bool is_displayable() override;
    bool is_latest(const Glib::ustring& latest);
    bool latest(Glib::DateTime& datetime) override;
    void set_extent(JsonObject* entry);

    Glib::RefPtr<Gdk::Pixbuf> get_legend() override;
    void set_legend(Glib::RefPtr<Gdk::Pixbuf>& legend) override;

private:
    Glib::ustring m_dataid; // this is the base e.g. globalir for all ir based images
    Glib::ustring m_description;
    std::vector<Glib::ustring> m_times;
    Glib::ustring m_type;       // this is the representation e.g. "raster" for images, "shape" for symbols
    Glib::ustring m_outputtype; // png24 for íamges
    Glib::RefPtr<Gdk::Pixbuf> m_legend;

};

class RealEarth
: public Weather
{
public:
    RealEarth(WeatherConsumer* consumer, const Glib::ustring& base_url);
    virtual ~RealEarth() = default;

    void capabilities() override;
    void request(const Glib::ustring& productId) override;
    Glib::ustring get_base_url() {
        return m_base_url;
    }
    void check_product(const Glib::ustring& weatherProductId) override;
    void send(WeatherImageRequest& request, std::shared_ptr<WeatherProduct>& product);
    void inst_on_capabilities_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message);
    Glib::RefPtr<Gdk::Pixbuf> get_legend(std::shared_ptr<WeatherProduct>& product);

protected:
    void inst_on_latest_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message);
    void inst_on_extend_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message);
    void get_extend(std::shared_ptr<RealEarthProduct>& product);

private:
    Glib::ustring m_base_url;
    Glib::ustring queued_product_request;
};

