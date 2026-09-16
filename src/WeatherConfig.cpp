/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2018 rpf
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

#include <iostream>
#include <psc_format.hpp>
#include <Log.hpp>
#include <StringUtils.hpp>
#include <cmath>

#include "Weather.hpp"
#include "RealEarth.hpp"
#include "WebMapService.hpp"
#include "WeatherConfig.hpp"

WeatherConfig::WeatherConfig(const char* config_name)
: KeyConfig{config_name}
{

}

void
WeatherConfig::read()
{
    auto file = Gio::File::create_for_path(getConfigName());
    if (file->query_exists()) {  // it is not a error, if the file doesn't exists
        try {
            if (!m_config->load_from_file(getConfigName(), Glib::KEY_FILE_NONE)) {
                std::cerr << "Error loading " << getConfigName() << std::endl;
            }
        }
        catch (const Glib::FileError& exc) {
            Glib::ustring msg{psc::fmt::format("Error {} loading config {}", exc.what(), m_configName)};
            psc::log::Log::logAdd(psc::log::Level::Error, msg);
        }
    }
    if (!m_config->has_group(get_main_config_group())) {   // create group
        m_config->set_string(get_main_config_group(), LOG_LEVEL, DEFAULT_LOG_LEVEL);
    }
    m_weatherServices.clear(); // since read may get used repeatedly avoid duplicates
    for (uint32_t i = 0; i < MAX_WEATHER_SERVICES; ++i) {
        migrateWeatherServices(i);
        std::shared_ptr<WebMapServiceConf> weatherService;
        auto weatherGrp = Glib::ustring::sprintf("%s%d", GRP_WEATHER, i);
        if (m_config->has_group(weatherGrp)
         && m_config->has_key(weatherGrp, WEATHER_SERVICE_ADDRESS)
         && m_config->has_key(weatherGrp, WEATHER_SERVICE_NAME)) {
            auto weatherAddress = m_config->get_string(weatherGrp, WEATHER_SERVICE_ADDRESS);
            auto weatherName = m_config->get_string(weatherGrp, WEATHER_SERVICE_NAME);
            int delay_sec = DEF_UPDATE_DELAY_SEC;
            if (m_config->has_key(weatherGrp, WEATHER_SERVICE_DELAY)) {
                delay_sec = m_config->get_integer(weatherGrp, WEATHER_SERVICE_DELAY);
                if (delay_sec < MIN_UPDATE_DELAY_SEC) {
                    delay_sec = MIN_UPDATE_DELAY_SEC;
                }
            }
            Glib::ustring weatherType{WEATHER_WMS_CONF};
            if (m_config->has_key(weatherGrp, WEATHER_SERVICE_TYPE)) {
                weatherType = m_config->get_string(weatherGrp, WEATHER_SERVICE_TYPE);
            }
            bool viewCurrentTime{false};
            if (m_config->has_key(weatherGrp, WEATHER_SERVICE_LOCAL_TIME)) {
                viewCurrentTime = m_config->get_boolean(weatherGrp, WEATHER_SERVICE_LOCAL_TIME);
            }
            weatherService = std::make_shared<WebMapServiceConf>(weatherName, weatherAddress, delay_sec, weatherType, viewCurrentTime);
        }
        else {
            switch (i) {    // prepare some defaults
            case 0:
                weatherService = std::make_shared<WebMapServiceConf>("RealEarth", "https://realearth.ssec.wisc.edu/", MIN_UPDATE_DELAY_SEC, WEATHER_REAL_EARTH_CONF, false);
                break;
            case 1:
                weatherService = std::make_shared<WebMapServiceConf>("EumetSat", "https://view.eumetsat.int/geoserver/wms", DEF_UPDATE_DELAY_SEC, WEATHER_WMS_CONF, false);
                break;
            case 2:
                weatherService = std::make_shared<WebMapServiceConf>("DeutscherWetterDienst", "https://maps.dwd.de/geoserver/ows", MIN_UPDATE_DELAY_SEC, WEATHER_WMS_CONF, true);
                break;
            }
        }
        if (weatherService) {
            m_weatherServices.push_back(weatherService);
        }
    }
}

void
WeatherConfig::saveConfig()
{
    // since we migrated this class, don't use base function
    std::cout << "This function KeyConfig::saveConfig was disabled!" << std::endl;
}

void
WeatherConfig::loadConfig()
{
    // since we migrated this class, don't use base function
    std::cout << "This function KeyConfig::loadConfig was disabled!" << std::endl;
}


// the decision to put all config into the main-grp was questionable,
//   so now move them into separate groups
void
WeatherConfig::migrateWeatherServices(uint32_t i)
{
    auto addressKey = Glib::ustring::sprintf("%s%d", WEATHER_SERVICE_ADDRESS, i);
    auto nameKey = Glib::ustring::sprintf("%s%d", WEATHER_SERVICE_NAME, i);
    if (m_config->has_key(get_main_config_group(), addressKey)
     && m_config->has_key(get_main_config_group(), nameKey)) {
        auto weatherGrp = Glib::ustring::sprintf("%s%d", GRP_WEATHER, i);
        auto weatherAddress = m_config->get_string(get_main_config_group(), addressKey);
        m_config->remove_key(get_main_config_group(), addressKey);
        m_config->set_string(weatherGrp, WEATHER_SERVICE_ADDRESS, weatherAddress);
        auto weatherName = m_config->get_string(get_main_config_group(), nameKey);
        m_config->remove_key(get_main_config_group(), nameKey);
        m_config->set_string(weatherGrp, WEATHER_SERVICE_NAME, weatherName);
        auto delayKey = Glib::ustring::sprintf("%s%d", WEATHER_SERVICE_DELAY, i);
        if (m_config->has_key(get_main_config_group(), delayKey)) {
            int delay_sec = m_config->get_integer(get_main_config_group(), delayKey);
            m_config->remove_key(get_main_config_group(), delayKey);
            m_config->set_integer(weatherGrp, WEATHER_SERVICE_DELAY, delay_sec);
        }
        auto typeKey = Glib::ustring::sprintf("%s%d", WEATHER_SERVICE_TYPE, i);
        if (m_config->has_key(get_main_config_group(), typeKey)) {
            Glib::ustring  weatherType = m_config->get_string(get_main_config_group(), typeKey);
            m_config->remove_key(get_main_config_group(), typeKey);
            m_config->set_string(weatherGrp, WEATHER_SERVICE_TYPE, weatherType);
        }
        auto localTimeKey = Glib::ustring::sprintf("%s%d", WEATHER_SERVICE_LOCAL_TIME, i);
        if (m_config->has_key(get_main_config_group(), localTimeKey)) {
            bool viewCurrentTime = m_config->get_boolean(get_main_config_group(), localTimeKey);
            m_config->remove_key(get_main_config_group(), localTimeKey);
            m_config->set_boolean(weatherGrp, WEATHER_SERVICE_LOCAL_TIME, viewCurrentTime);
        }
    }
}


bool
WeatherConfig::save()
{
    bool ret{false};
    if (m_config) {
        if (!m_config->has_group(get_main_config_group())) {   // create group
            m_config->set_string(get_main_config_group(), LOG_LEVEL, DEFAULT_LOG_LEVEL);
        }
        if (m_weatherImageSize > 0) {
            m_config->set_integer(get_main_config_group(), WEATHER_IMAGE_SIZE, m_weatherImageSize);
        }
        int n{};
        for (uint32_t i = 0; i < m_weatherServices.size(); ++i) {
            auto weatherService = m_weatherServices[i];
            if (weatherService) {
                auto weatherGrp = Glib::ustring::sprintf("%s%d", GRP_WEATHER, n);
                m_config->set_string(weatherGrp, WEATHER_SERVICE_ADDRESS, weatherService->getAddress());
                m_config->set_string(weatherGrp, WEATHER_SERVICE_NAME, weatherService->getName());
                m_config->set_integer(weatherGrp, WEATHER_SERVICE_DELAY, weatherService->getDelaySec());
                m_config->set_string(weatherGrp, WEATHER_SERVICE_TYPE, weatherService->getType());
                m_config->set_boolean(weatherGrp, WEATHER_SERVICE_LOCAL_TIME, weatherService->isViewCurrentTime());
                ++n;
            }
        }
        try {
            ret = m_config->save_to_file(getConfigName());
        }
        catch (const Glib::FileError& exc) {
            std::string msg{psc::fmt::format("Error {} saving config {}", exc.what(), getConfigName())};
            psc::log::Log::logAdd(psc::log::Level::Error, msg);
        }
    }
    return ret;
}

unsigned int
WeatherConfig::getDebug()
{
    return m_debug;
}

void
WeatherConfig::setDebug(unsigned int debug)
{
    m_debug = debug;
}

void
WeatherConfig::setWeatherProductId(const std::string& weatherProductId)
{
    m_config->set_string(get_main_config_group(), WEATHER_PRODUCT, weatherProductId);
}

std::string
WeatherConfig::getWeatherProductId()
{
    std::string weatherProductId;
    if (m_config->has_key(get_main_config_group(), WEATHER_PRODUCT))
        weatherProductId = m_config->get_string(get_main_config_group(), WEATHER_PRODUCT);
    return weatherProductId;
}

void
WeatherConfig::setWeatherServiceId(const std::string& weatherServiceId)
{
    m_config->set_string(get_main_config_group(), WEATHER_SERVICE, weatherServiceId);
}

std::string
WeatherConfig::getWeatherServiceId()
{
    std::string weatherServiceId;
    if (m_config->has_key(get_main_config_group(), WEATHER_SERVICE))
        weatherServiceId = m_config->get_string(get_main_config_group(), WEATHER_SERVICE);
    return weatherServiceId;
}

void
WeatherConfig::setWeatherTransparency(double transp)
{
    m_config->set_double(get_main_config_group(), WEATHER_TRANSP, transp);
}

double
WeatherConfig::getWeatherTransparency()
{
    double weatherTransparency{1.0};
    if (m_config->has_key(get_main_config_group(), WEATHER_TRANSP))
        weatherTransparency = m_config->get_double(get_main_config_group(), WEATHER_TRANSP);
    return weatherTransparency;
}

std::vector<std::shared_ptr<WebMapServiceConf>>
WeatherConfig::getWebMapServices()
{
    return m_weatherServices;
}

// as the vector is copied on return, need this to add entries
std::shared_ptr<WebMapServiceConf>
WeatherConfig::addWebMapService(const Glib::ustring& newName)
{
    auto service = std::make_shared<WebMapServiceConf>(newName, "", 0, "", false);
    m_weatherServices.push_back(service);
    return service;
}


int
WeatherConfig::getWeatherMinPeriodSec()
{
    int waether_min_period_sec{5*SECS_PER_MINUTE};
    if (m_config->has_key(get_main_config_group(), WEATHER_MIN_PERIOD_SECONDS))
        waether_min_period_sec = m_config->get_integer(get_main_config_group(), WEATHER_MIN_PERIOD_SECONDS);
    if (waether_min_period_sec < SECS_PER_MINUTE) {
        waether_min_period_sec = SECS_PER_MINUTE;
    }
    if (waether_min_period_sec > SECS_PER_DAY) {
        waether_min_period_sec = SECS_PER_DAY;
    }
    return waether_min_period_sec;
}

void
WeatherConfig::setWeatherMinPeriodSec(uint32_t sec)
{
    m_config->set_integer(get_main_config_group(), WEATHER_MIN_PERIOD_SECONDS, sec);
}

Glib::ustring
WeatherConfig::getLogLevel()
{
    Glib::ustring logLevel{DEFAULT_LOG_LEVEL};
    if (m_config->has_key(get_main_config_group(), LOG_LEVEL))
        logLevel = m_config->get_string(get_main_config_group(), LOG_LEVEL);
    return logLevel;
}

int
WeatherConfig::getWeatherImageSize()
{
    if (m_weatherImageSize > 0) {   // only do computation once
        return m_weatherImageSize;
    }
    if (m_config->has_key(get_main_config_group(), WEATHER_IMAGE_SIZE)) {
        m_weatherImageSize = m_config->get_integer(get_main_config_group(), WEATHER_IMAGE_SIZE);
        // check if it is power of two, as it will be used as OpenGL texture
        double pow2 = std::log2(m_weatherImageSize);
        double integral;
        double rem = std::modf(pow2, &integral);
        if (rem > 0.0001) {
            uint32_t shift = static_cast<uint32_t>(integral);
            m_weatherImageSize = 1u << shift;
        }
        // limit to useful range
        m_weatherImageSize = std::max(std::min(m_weatherImageSize, MAX_WEATHER_IMAGE_SIZE), MIN_WEATHER_IMAGE_SIZE);
    }
    else {
        m_weatherImageSize = DEFAULT_WEATHER_IMAGE_SIZE;
    }
    return m_weatherImageSize;
}

std::shared_ptr<WebMapServiceConf>
WeatherConfig::getActiveWebMapServiceConf()
{
    std::shared_ptr<WebMapServiceConf> conf;
    auto name = getWeatherServiceId();
    if (!name.empty()) {
        for (auto cnf : getWebMapServices()) {
            if (cnf->getName() == name) {
                conf = cnf;
                break;
            }
        }
    }
    return conf;
}

std::shared_ptr<Weather>
WeatherConfig::getService(WeatherConsumer* consumer,const std::shared_ptr<WebMapServiceConf>& serviceConf)
{
    Glib::ustring typeStr = serviceConf->getType();
    if (typeStr == WeatherConfig::WEATHER_REAL_EARTH_CONF) {
        return std::make_shared<RealEarth>(consumer, serviceConf->getAddress());
    }
    if (typeStr == WeatherConfig::WEATHER_WMS_CONF) {
        return std::make_shared<WebMapService>(consumer, serviceConf, getWeatherMinPeriodSec());
    }
    else {
        psc::log::Log::logAdd(psc::log::Level::Warn, [&] {
            return psc::fmt::format("refresh serviceId typeStr {}", typeStr);
        });
    }
    return std::shared_ptr<Weather>();
}

