/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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


#include "WeatherConfig.hpp"
#include "WeatherDialog.hpp"
#include "WeatherConfig.hpp"
#include "BoundsDisplay.hpp"
#include "WeatherConfigGrid.hpp"

BaseConfigGrid::BaseConfigGrid(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder, BaseConfigListener* sphereView)
: Gtk::Grid(cobject)
, m_sphereView{sphereView}
{

}


ConfigWeatherGrid::ConfigWeatherGrid(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder, BaseConfigListener* sphereView)
: BaseConfigGrid(cobject, refBuilder, sphereView)
{
    m_config = m_sphereView->get_config();
    Gtk::Scale* pWeatherTransp{nullptr};
    refBuilder->get_widget("scaleWeather", pWeatherTransp);
    if (pWeatherTransp) {
        pWeatherTransp->set_increments(0.01, 0.1);
        pWeatherTransp->set_range(0.1, 2.0);
        pWeatherTransp->set_value(m_config->getWeatherTransparency());
        pWeatherTransp->signal_value_changed().connect(sigc::bind<Gtk::Scale *>(
                                   sigc::mem_fun(*m_sphereView, &BaseConfigListener::weather_transparency_changed),
                                   pWeatherTransp));
    }
    refBuilder->get_widget("descWeather", m_DescWeather);
    refBuilder->get_widget("legendWeather", m_LegendWeather);
    refBuilder->get_widget("comboWeatherProduct", m_weatherProductCombo);
    if (m_weatherProductCombo) {
        m_weatherProductCombo->append("", "");  // keep empty element
        refreshWeatherProducts();
        m_weatherProductCombo->signal_changed().connect(
            sigc::mem_fun(*this, &ConfigWeatherGrid::weather_product_changed));

    }
    refBuilder->get_widget("comboWeatherService", m_weatherServiceCombo);
    if (m_weatherServiceCombo) {
        auto confs = m_config->getWebMapServices();
        m_weatherServiceCombo->append("", "");  // allow empty selection
        for (auto servConf : confs) {
            m_weatherServiceCombo->append(servConf->getName(), servConf->getName());
        }
        m_weatherServiceCombo->set_active_id(m_config->getWeatherServiceId());
        m_weatherServiceCombo->signal_changed().connect(
                    sigc::mem_fun(*this, &ConfigWeatherGrid::weather_service_changed));
    }
    refBuilder->get_widget_derived("weatherBounds", m_boundsDisplay, sphereView);
    refBuilder->get_widget("weatherEdit", m_weatherEdit);
    m_weatherEdit->signal_clicked().connect([&] {
        auto appl = Glib::RefPtr<Gtk::Application>::cast_dynamic(Gtk::Application::get_default());
        auto win = dynamic_cast<Gtk::ApplicationWindow*>(appl->get_active_window());
        if (win) {
            auto selected = m_weatherServiceCombo->get_active_id();
            show_weather_edit(win, selected , false);
            //auto var = Glib::Variant<Glib::ustring>::create(selected);
            //win->activate_action("weatherEdit", var);
        }
        else {
            std::cout << "No window was found!" << std::endl;
        }
    });
    refBuilder->get_widget("weatherAdd", m_weatherAdd);
    m_weatherAdd->signal_clicked().connect([&] {
        auto appl = Glib::RefPtr<Gtk::Application>::cast_dynamic(Gtk::Application::get_default());
        auto win = dynamic_cast<Gtk::ApplicationWindow*>(appl->get_active_window());
        if (win) {
            auto selected = m_weatherServiceCombo->get_active_id();
            show_weather_edit(win, selected , true);
            //auto var = Glib::Variant<Glib::ustring>::create(selected);
            //win->activate_action("weatherAdd", var);
        }
    });
    m_weatherAdd->set_sensitive(m_config->getWebMapServices().size() < WeatherConfig::MAX_WEATHER_SERVICES);

    setWeatherDescription();
}

void
ConfigWeatherGrid::show_weather_edit(Gtk::ApplicationWindow* appWin, const Glib::ustring& idStr, bool add)
{
    //std::cout << "GlGlobeWindow::on_action_weather id" << idStr << std::endl;
    // to simplify the overall handling
    //   close weather dialog here and reopen when weather setup is done
    m_sphereView->closeConfigDlg();
    auto weatherDlg = WeatherDialog::create(m_config, idStr, add);
    if (weatherDlg) {

        weatherDlg->set_transient_for(*appWin);
        int ret = weatherDlg->run();
        weatherDlg->hide();
        if (ret == Gtk::RESPONSE_OK) {
            m_sphereView->save_config();
            m_sphereView->on_action_preferences();    // reopen config
        }
        delete weatherDlg;      // cleanup
    }
}


void
ConfigWeatherGrid::setLegendWeather(Glib::RefPtr<Gdk::Pixbuf> legend)
{
    if (m_LegendWeather) {
        m_LegendWeather->set(legend);
    }
}

void
ConfigWeatherGrid::setWeatherDescription()
{
    auto weatherProdId = m_sphereView->get_config()->getWeatherProductId();
    std::shared_ptr<WeatherProduct> weatherProd;
    if (m_sphereView->get_weather()) {
        weatherProd = m_sphereView->get_weather()->find_product(weatherProdId);
    }
    Glib::ustring desc;
    if (weatherProd) {
        desc = weatherProd->get_description();
    }
    if (m_DescWeather) {
        m_DescWeather->get_buffer()->set_text(desc);
    }
    if (m_LegendWeather) {
        if (weatherProd) {
            auto legend = m_sphereView->get_weather()->get_legend(weatherProd);
            if (legend) {
                m_LegendWeather->set(legend);
            }
            else {  // notify if legend becomes available
                weatherProd->signal_legend().connect(
                        sigc::mem_fun(*this, &ConfigWeatherGrid::setLegendWeather));
            }
        }
        else {
            m_LegendWeather->clear();
        }
    }
    if (m_boundsDisplay && weatherProd) {
        m_boundsDisplay->setBounds(weatherProd->getBounds());
    }
}


void
ConfigWeatherGrid::weather_product_changed()
{
    if (!m_blockWeatherProductUpdate) {
        #ifdef CONFIG_DEBUG
        std::cout << "ConfigWeatherGrid::weather_product_changed" << std::endl;
        #endif
        auto id = m_weatherProductCombo->get_active_id();
        m_sphereView->get_config()->setWeatherProductId(id);
        setWeatherDescription();
        m_sphereView->request_weather_product();
    }
}

void
ConfigWeatherGrid::weather_service_changed()
{
    #ifdef CONFIG_DEBUG
    std::cout << "ConfigWeatherGrid::weather_service_changed" << std::endl;
    #endif
    auto id = m_weatherServiceCombo->get_active_id();
    m_weatherEdit->set_sensitive(!id.empty());
    m_sphereView->get_config()->setWeatherServiceId(id);
    m_sphereView->get_config()->setWeatherProductId("");
    m_blockWeatherProductUpdate = true;
    m_weatherProductCombo->remove_all();
    std::shared_ptr<Weather> weather = m_sphereView->refresh_weather_service();
    if (weather) {
        weather->signal_products_completed().connect(
            sigc::mem_fun(*this, &ConfigWeatherGrid::refreshWeatherProducts));
    }
    setWeatherDescription();
    m_blockWeatherProductUpdate = false;
}


void
ConfigWeatherGrid::refreshWeatherProducts()
{
    #ifdef CONFIG_DEBUG
    std::cout << "ConfigWeatherGrid::refreshWeatherProducts" << std::endl;
    #endif
    m_blockWeatherProductUpdate = true;
    m_weatherProductCombo->unset_active();
    auto list = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_weatherProductCombo->get_model());
    if (list) {
        #ifdef CONFIG_DEBUG
        std::cout << "ConfigWeatherGrid::refreshWeatherProducts got list" << std::endl;
        #endif
        auto chlds = list->children();
        int i = 0;
        for (auto chld : chlds) {
            if (i > 0) {
                list->erase(chld);
            }
            ++i;
        }
    }
    else {
        std::cout << "ConfigWeatherGrid::refreshWeatherProducts got list no!" << std::endl;
    }
    auto weather = m_sphereView->get_weather();
    if (weather) {
        auto products = weather->get_products();
        #ifdef CONFIG_DEBUG
        std::cout << "ConfigWeatherGrid::refreshWeatherProducts products " << products.size() << std::endl;
        #endif

        for (auto product : products) {
            if (product->is_displayable()) {
                m_weatherProductCombo->append(product->get_id(), product->get_name());
            }
        }
    }
    else {
        std::cout << "ConfigWeatherGrid::refreshWeatherProducts have no weather." << std::endl;
    }
    m_blockWeatherProductUpdate = false;
    auto config = m_sphereView->get_config();
    if (!config->getWeatherProductId().empty()) {
        m_weatherProductCombo->set_active_id(config->getWeatherProductId());
    }
    else {
        m_weatherProductCombo->set_active(0);
    }
}
