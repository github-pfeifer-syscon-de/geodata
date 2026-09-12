/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * Copyright (C) 2024 RPf 
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
#include <Weather.hpp>
#include <Log.hpp>

class WeatherConfig;

struct WeatherColumns
: public Gtk::TreeModel::ColumnRecord
{
public:
    Gtk::TreeModelColumn<Glib::ustring> m_name;
    Gtk::TreeModelColumn<Glib::ustring> m_dimStart;
    Gtk::TreeModelColumn<Glib::ustring> m_dimEnd;
    Gtk::TreeModelColumn<Glib::ustring> m_dimPeriod;
    Gtk::TreeModelColumn<Glib::ustring> m_description;
    WeatherColumns()
    {
        add(m_name);
        add(m_dimStart);
        add(m_dimEnd);
        add(m_dimPeriod);
        add(m_description);
    }
};

struct ProtocolColumns
: public Gtk::TreeModel::ColumnRecord
{
public:
    Gtk::TreeModelColumn<Glib::ustring> m_date;
    Gtk::TreeModelColumn<Glib::ustring> m_level;
    Gtk::TreeModelColumn<Glib::ustring> m_message;
    ProtocolColumns()
    {
        add(m_date);
        add(m_level);
        add(m_message);
    }
};

class ProtocolPlugin
: public psc::log::LogPlugin
, public std::enable_shared_from_this<ProtocolPlugin>
{
public:
    ProtocolPlugin(ProtocolColumns& protocolColumns, const Glib::RefPtr<Gtk::ListStore>& protocolModel);
    explicit ProtocolPlugin(const ProtocolPlugin& orig) = delete;
    virtual ~ProtocolPlugin() = default;
    void log(psc::log::Level level
      , const Glib::ustring& msg
      , const std::source_location location) override;    // psc::log::LogPlugin

    static std::shared_ptr<ProtocolPlugin> create(ProtocolColumns& protocolColumns, const Glib::RefPtr<Gtk::ListStore>& protocolModel);
private:
    ProtocolColumns& m_protocolColumns;
    Glib::RefPtr<Gtk::ListStore> m_protocolModel;
};

class WeatherDialog
: public Gtk::Dialog
, public WeatherConsumer
{
public:
    WeatherDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder,const std::shared_ptr<WeatherConfig>& config, const Glib::ustring& id, bool add);
    explicit WeatherDialog(const WeatherDialog& orig) = delete;
    virtual ~WeatherDialog() = default;

    bool save();
    void on_action_test();
    void showMessage(const Glib::ustring& msg, Gtk::MessageType msgType = Gtk::MessageType::MESSAGE_INFO);
    void weather_image_notify(WeatherImageRequest& request) override;
    int get_weather_image_size() override;

    static WeatherDialog* create(const std::shared_ptr<WeatherConfig>& config, const Glib::ustring& id, bool add);


    static constexpr auto DELAY_SCALE = 60; // convert between display minutes <-> stored seconds
protected:
    bool validate(Glib::ustring& newName, bool checkProducts);
    void updateWeatherConf(const std::shared_ptr<WebMapServiceConf>& service);
    void request_weather_product();
    void on_action_select();
    void on_action_help();
    void on_action_save();

private:
    std::shared_ptr<WeatherConfig> m_config;
    Glib::ustring m_id;
    int32_t m_idx{-1};
    bool m_add;
    Gtk::ComboBoxText* m_comboType{nullptr};
    Gtk::Entry* m_entryName{nullptr};
    Gtk::Entry* m_entryAddress{nullptr};
    Gtk::SpinButton* m_spinDelay{nullptr};
    Gtk::CheckButton* m_cbLocalTime{nullptr};
    Gtk::TreeView* m_productList{nullptr};
    Gtk::TreeView* m_protocolList{nullptr};
    Gtk::TextView* m_description{nullptr};
    std::shared_ptr<Weather> m_weather;
    WeatherColumns m_weatherColumns;
    ProtocolColumns m_protocolColumns;
    Glib::RefPtr<Gtk::ListStore> m_productModel;
    Glib::RefPtr<Gtk::ListStore> m_protocolModel;
    Gtk::Button* m_btnSave{nullptr};
};

