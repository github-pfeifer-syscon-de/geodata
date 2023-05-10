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


class RealEarth : public Weather
{
public:
    RealEarth(WeatherConsumer* consumer);
    virtual ~RealEarth() = default;

    void capabilities();
    void request(const Glib::ustring& productId);

    Glib::ustring get_base_url() override {
        return m_base_url;
    }
    std::vector<std::shared_ptr<WeatherProduct>> get_products() {
        return m_products;
    }
    std::shared_ptr<WeatherProduct> find_product(const Glib::ustring& weatherProductId);
    void check_product(const Glib::ustring& weatherProductId);
    double yAxisProjection(double input) override;
    double yAxisUnProjection(double input);
    void send(WeatherImageRequest& request, std::shared_ptr<WeatherProduct>& product);
    std::string dump(const guint8 *data, gsize size);
    void inst_on_image_callback(const Glib::ustring& error, int status, SpoonMessage* message);
    void inst_on_capabilities_callback(const Glib::ustring& error, int status, SpoonMessage* message) override;
    Glib::RefPtr<Gdk::Pixbuf> get_legend(std::shared_ptr<WeatherProduct>& product);

    static constexpr auto NAME{"RealEarth"};
protected:
    void inst_on_latest_callback(const Glib::ustring& error, int status, SpoonMessage* message);
    void inst_on_extend_callback(const Glib::ustring& error, int status, SpoonMessage* message);
    void get_extend(std::shared_ptr<WeatherProduct>& product);
    void inst_on_legend_callback(const Glib::ustring& error, int status, SpoonMessage* message, std::shared_ptr<WeatherProduct> product);
    double normToRadians(double norm);
    double xAxisProjection(double input);

private:
    SpoonSession m_spoonSession{"map private use "};
    static const char* m_base_url;
    std::vector<std::shared_ptr<WeatherProduct>> m_products;
    Glib::ustring queued_product_request;
};

