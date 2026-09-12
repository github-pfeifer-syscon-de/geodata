/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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


#include <StringUtils.hpp>
#include <psc_format.hpp>
#include <psc_i18n.hpp>
#include <clocale>

#include "WeatherConfig.hpp"
#include "WeatherConfigGrid.hpp"
#include "geodata_config.h"

#include "WeatherDialog.hpp"

ProtocolPlugin::ProtocolPlugin(ProtocolColumns& protocolColumns, const Glib::RefPtr<Gtk::ListStore>& protocolModel)
: psc::log::LogPlugin("weatherDebug")
, m_protocolColumns{protocolColumns}
, m_protocolModel{protocolModel}
{
}

std::shared_ptr<ProtocolPlugin>
ProtocolPlugin::create(ProtocolColumns& protocolColumns, const Glib::RefPtr<Gtk::ListStore>& protocolModel)
{
    return std::make_shared<ProtocolPlugin>(protocolColumns, protocolModel);
}

void
ProtocolPlugin::log(psc::log::Level level
            , const Glib::ustring& msg
            , const std::source_location location)
{
    Glib::DateTime now = Glib::DateTime::create_now_local();
    auto iter = m_protocolModel->append();
    auto row = *iter;
    row.set_value(m_protocolColumns.m_date, now.format("%T"));
    Glib::ustring slevel{psc::log::Log::getLevel(level)};
    row.set_value(m_protocolColumns.m_level, slevel);
    row.set_value(m_protocolColumns.m_message, msg);
    //std::cout << slevel << " " << msg << std::endl;
}


WeatherDialog::WeatherDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder, const std::shared_ptr<WeatherConfig>& config, const Glib::ustring& id, bool add)
: Gtk::Dialog(cobject)
, m_config{config}
, m_id{id}
, m_add{add}
{
    refBuilder->get_widget("type", m_comboType);
    m_comboType->append(WeatherConfig::WEATHER_WMS_CONF, "WebMapService");
    m_comboType->append(WeatherConfig::WEATHER_REAL_EARTH_CONF, "RealEarth");
    refBuilder->get_widget("name", m_entryName);
    refBuilder->get_widget("address", m_entryAddress);
    Gtk::Button* btnTest{nullptr};
    refBuilder->get_widget("test", btnTest);
    refBuilder->get_widget("delay", m_spinDelay);
    refBuilder->get_widget("localTime", m_cbLocalTime);
    refBuilder->get_widget("list", m_productList);
    refBuilder->get_widget("description", m_description);
    refBuilder->get_widget("protocol", m_protocolList);
    btnTest->signal_clicked().connect(
            sigc::mem_fun(*this, &WeatherDialog::on_action_test));
    Gtk::Button* btnHelp{nullptr};
    refBuilder->get_widget("help", btnHelp);
    btnHelp->signal_clicked().connect(
            sigc::mem_fun(*this, &WeatherDialog::on_action_help));
    refBuilder->get_widget("save", m_btnSave);
    m_btnSave->signal_clicked().connect(
            sigc::mem_fun(*this, &WeatherDialog::on_action_save));
    m_btnSave->set_sensitive(false);

    int col = m_productList->append_column(_("Name"), m_weatherColumns.m_name);
    auto column = m_productList->get_column(col-1);
    column->set_fixed_width(300);       // don't allow to push other columns out of view
    m_productList->append_column(_("Start"), m_weatherColumns.m_dimStart);
    m_productList->append_column(_("End"), m_weatherColumns.m_dimEnd);
    m_productList->append_column(_("Period"), m_weatherColumns.m_dimPeriod);
    m_productModel = Gtk::ListStore::create(m_weatherColumns);
    m_productList->set_model(m_productModel);
    m_productList->get_selection()->signal_changed().connect(
            sigc::mem_fun(*this, &WeatherDialog::on_action_select));
    m_protocolList->append_column(_("Date"), m_protocolColumns.m_date);
    m_protocolList->append_column(_("Level"), m_protocolColumns.m_level);
    m_protocolList->append_column(_("Message"), m_protocolColumns.m_message);
    m_protocolModel = Gtk::ListStore::create(m_protocolColumns);
    m_protocolList->set_model(m_protocolModel);

    auto services = m_config->getWebMapServices();
    //std::cout << "WeatherDialog id " << m_id << std::endl;
    for (uint32_t idx = 0; idx < services.size(); ++idx) {
        auto service = services[idx];
        if (m_id == service->getName()) {
            m_idx = idx;
            auto service = services[idx];
            m_comboType->set_active_id(service->getType());
            m_entryName->set_text(service->getName());
            m_entryAddress->set_text(service->getAddress());
            m_spinDelay->set_value(service->getDelaySec() / DELAY_SCALE);
            m_cbLocalTime->set_active(service->isViewCurrentTime());
            break;
        }
    }
    if (add || m_idx < 0) {
        m_idx = services.size();
    }
}

void
WeatherDialog::on_action_save()
{
    if (!save()) {
        return;
    }
    response(Gtk::ResponseType::RESPONSE_OK);   // this closes
}

void
WeatherDialog::on_action_help()
{
    Gtk::MessageDialog helpdialog("", false, Gtk::MessageType::MESSAGE_OTHER);
    helpdialog.set_size_request(480, 320);
    auto box = helpdialog.get_content_area();
    auto chlds = box->get_children();
    if (chlds.size() > 0) { // remove default message
        box->remove(*(chlds[0]));
    }
    Gtk::ScrolledWindow* scroll = Gtk::make_managed<Gtk::ScrolledWindow>();
    scroll->set_vexpand(true);
    Gtk::TextView* text = Gtk::make_managed<Gtk::TextView>();
    scroll->add(*text);
    box->add(*scroll);
    scroll->set_visible(true);
    text->set_visible(true);
    text->set_editable(false);
    text->set_wrap_mode(Gtk::WrapMode::WRAP_WORD);
    Glib::RefPtr<const Glib::Bytes> refHelp;
    std::string localeMsg = std::setlocale(LC_MESSAGES, nullptr);
    if (localeMsg.length() >= 2) {  // try to find language
        try {
            auto testName = "help_" + localeMsg.substr(0, 2) + ".txt";
            std::string testRes = std::string(GEODATA_RESOURCE_PREFIX) + testName;
            refHelp = Gio::Resource::lookup_data_global(testRes);
        }
        catch (const Glib::Error& ex) { // this may fail, so use en
        }
    }
    if (!refHelp) {
        std::string helpRes = std::string(GEODATA_RESOURCE_PREFIX) + "help.txt";
        refHelp = Gio::Resource::lookup_data_global(helpRes);
    }
    gsize size{};
    gconstpointer gp = refHelp->get_data(size);
    // use string as the length is "bytes" (ustring expects utf-8 chars)!
    std::string helpText(static_cast<const char*>(gp), size);
    text->get_buffer()->set_text(helpText);
    helpdialog.set_transient_for(*this);
    helpdialog.set_modal(false);
    helpdialog.run();
    helpdialog.hide();
}

void
WeatherDialog::on_action_test()
{
    Glib::ustring newName;
    if (!validate(newName, false)) {
        return;
    }
    auto service = std::make_shared<WebMapServiceConf>(newName, "", 0, "", false);
    updateWeatherConf(service);
    m_weather = m_config->getService(this, service);
    if (m_weather) {
        m_btnSave->set_sensitive(false);
        m_productModel->clear();
        m_protocolModel->clear();
        auto protocolLog = ProtocolPlugin::create(m_protocolColumns, m_protocolModel);
        auto debugLog = std::make_shared<psc::log::Log>(protocolLog);
        debugLog->setLevel(psc::log::Level::Debug);
        m_weather->setLog(debugLog);
        m_weather->signal_products_completed().connect(
            sigc::mem_fun(*this, &WeatherDialog::request_weather_product));
        m_weather->capabilities();
    }
    else {
        showMessage("Not a valid config");
    }
}

void WeatherDialog::on_action_select()
{
    auto iter = m_productList->get_selection()->get_selected();
    if (iter) {
        auto row = *iter;
        auto desc = row.get_value(m_weatherColumns.m_description);
        m_description->get_buffer()->set_text(desc);
    }
}

void
WeatherDialog::weather_image_notify(WeatherImageRequest& request)
{
    std::cout << "WeatherDialog::weather_image_notify" << std::endl;
}

int
WeatherDialog::get_weather_image_size()
{
    std::cout << "WeatherDialog::get_weather_image_size" << std::endl;
    return 0;
}

void
WeatherDialog::request_weather_product()
{
    auto prods = m_weather->get_products();
    m_btnSave->set_sensitive(!prods.empty());   // require are reasonable response, to make save work
    //std::cout << "WeatherDialog::request_weather_product " << prods.size() << std::endl;
    for (auto& prod : prods) {
        auto iter = m_productModel->append();
        auto row = *iter;
        row.set_value(m_weatherColumns.m_name, prod->get_name());
        auto dim = prod->get_dimension();
        std::vector<Glib::ustring> dims;
        StringUtils::split(dim, '/', dims);
        if (dims.size() >= 1) {
            auto dimStart = Glib::DateTime::create_from_iso8601(dims[0]);
            auto dimStartLocal = dimStart.to_local();
            row.set_value(m_weatherColumns.m_dimStart, dimStartLocal ? dimStartLocal.format_iso8601() : dims[0]);
        }
        if (dims.size() >= 2) {
            auto dimEnd = Glib::DateTime::create_from_iso8601(dims[1]);
            auto dimEndLocal = dimEnd.to_local();
            row.set_value(m_weatherColumns.m_dimEnd, dimEndLocal ? dimEndLocal.format_iso8601() : dims[1]);
        }
        if (dims.size() >= 3) {
            row.set_value(m_weatherColumns.m_dimPeriod, dims[2]);
        }
        row.set_value(m_weatherColumns.m_description, prod->get_description());
    }
    //m_productList->columns_autosize();
}


void
WeatherDialog::showMessage(const Glib::ustring& msg, Gtk::MessageType msgType)
{
    Gtk::MessageDialog messagedialog(msg, false, msgType);
    messagedialog.run();
    messagedialog.hide();
}

bool
WeatherDialog::validate(Glib::ustring& newName, bool checkProducts)
{
    newName = m_entryName->get_text();
    StringUtils::trim(newName);
    if (newName.empty()) {
        showMessage(
                psc::fmt::vformat(
                    _("Please select a non empty name \"{}\".")
                    , psc::fmt::make_format_args(newName)));
        return false;
    }
    auto services = m_config->getWebMapServices();
    for (uint32_t idx = 0; idx < services.size(); ++idx) {
        auto service = services[idx];
        if (idx != static_cast<uint32_t>(m_idx)
         && service->getName() == newName) {
            showMessage(psc::fmt::vformat(
                    _("Please select a unique name \"{}\" has already been used.")
                    , psc::fmt::make_format_args(newName)));
            return false;
        }
    }
    if (checkProducts && m_productModel->children().empty()) {
        showMessage( _("Please try \"Refresh\" first."));
        return false;
    }
    return true;
}

void
WeatherDialog::updateWeatherConf(const std::shared_ptr<WebMapServiceConf>& service)
{
    service->setType(m_comboType->get_active_id());
    service->setAddress(m_entryAddress->get_text());
    service->setDelaySec(m_spinDelay->get_value_as_int() * DELAY_SCALE);
    service->setViewCurrentTime(m_cbLocalTime->get_active());
}

bool
WeatherDialog::save()
{
    if (m_idx >= 0) {
        Glib::ustring newName;
        if (!validate(newName, true)) {
            return false;
        }
        auto services = m_config->getWebMapServices();
        std::shared_ptr <WebMapServiceConf> service;
        if (static_cast<uint32_t>(m_idx) >= services.size()) {
            service = m_config->addWebMapService(newName);
        }
        else {
            service = services[m_idx];
            service->setName(newName);
        }
        updateWeatherConf(service);
        return true;
    }
    return false;
}


WeatherDialog*
WeatherDialog::create(const std::shared_ptr<WeatherConfig>& config, const Glib::ustring& id, bool add)
{
    auto refBuilder = Gtk::Builder::create();
    try {
        refBuilder->add_from_resource(GEODATA_RESOURCE_PREFIX"weather-dlg.ui");
        WeatherDialog* weatherDlg;
        refBuilder->get_widget_derived("weather-dlg", weatherDlg, config, id, add);
        if (weatherDlg) {
            return weatherDlg;
        }
        else {
            std::cerr << "WeatherDialog::create: No \"weather-dlg\" object in weather-dlg.ui"
                << std::endl;
        }
    }
    catch (const Glib::Error& ex) {
        std::cerr << "WeatherDialog::create " << ex.what() << std::endl;
    }
    return nullptr;
}
