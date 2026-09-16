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

#pragma once

#include <gtkmm.h>

class WeatherProduct;
class WeatherConfig;
class BoundsDisplay;
class Weather;

class BaseConfigListener
{
public:
    virtual std::string findFile(const std::string& name) = 0;
    virtual std::shared_ptr<WeatherConfig> get_config() = 0;
    virtual void weather_transparency_changed(Gtk::Scale *scale) = 0;
    virtual std::shared_ptr<Weather> get_weather() = 0;
    virtual void request_weather_product() = 0;
    virtual std::shared_ptr<Weather> refresh_weather_service() = 0;
    virtual void closeConfigDlg() = 0;
    virtual void save_config() = 0;
    virtual void on_action_preferences() = 0;    // reopen config
};

class BaseConfigGrid
: public Gtk::Grid
{
protected:
    BaseConfigGrid(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder, BaseConfigListener* sphereView);
    virtual ~BaseConfigGrid() = default;
    BaseConfigListener* m_sphereView;

};

class ConfigWeatherGrid
: public BaseConfigGrid
{
public:
    ConfigWeatherGrid(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder, BaseConfigListener* sphereView);
    virtual ~ConfigWeatherGrid() = default;
    void setLegendWeather(Glib::RefPtr<Gdk::Pixbuf> legend);
    void setWeatherDescription();
    void refreshWeatherProducts();
    void on_action_weater(bool add);

protected:
    void weather_product_changed();
    void weather_service_changed();
    void show_weather_edit(
        Gtk::ApplicationWindow* appWin
        , const Glib::ustring& idStr
        , bool add);

private:
    Gtk::Image* m_LegendWeather{nullptr};
    Gtk::TextView* m_DescWeather{nullptr};
    Gtk::ComboBoxText* m_weatherProductCombo{nullptr};
    Gtk::ComboBoxText* m_weatherServiceCombo{nullptr};
    bool m_blockWeatherProductUpdate{false};
    BoundsDisplay* m_boundsDisplay{nullptr};
    std::shared_ptr<WeatherConfig> m_config;
    Gtk::Button* m_weatherEdit{nullptr};
    Gtk::Button* m_weatherAdd{nullptr};
};
