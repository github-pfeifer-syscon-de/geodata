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


#pragma once

#include <vector>
#include <glibmm.h>

#include "Weather.hpp"

class WeatherConfig
{
public:
    WeatherConfig() = default;
    virtual ~WeatherConfig() = default;

    virtual void read();
    virtual bool save();

    unsigned int getDebug();
    void setDebug(unsigned int  debug);
    Glib::ustring getLogLevel();
    void setWeatherProductId(const std::string& weatherProduct);
    std::string getWeatherProductId();
    void setWeatherServiceId(const std::string& weatherServiceId);
    std::string getWeatherServiceId();
    std::shared_ptr<WebMapServiceConf> getActiveWebMapServiceConf();
    void setWeatherTransparency(double transp);
    double getWeatherTransparency();

    std::vector<std::shared_ptr<WebMapServiceConf>> getWebMapServices();
    int getWeatherMinPeriodSec();
    void setWeatherMinPeriodSec(uint32_t sec);

    std::shared_ptr<Weather> getService(WeatherConsumer* consumer,const std::shared_ptr<WebMapServiceConf>& serviceConf);
    std::shared_ptr<WebMapServiceConf> addWebMapService(const Glib::ustring& newName);
    int getWeatherImageSize();
    static constexpr auto LOG_LEVEL{"logLevel"};
    static constexpr auto DEFAULT_LOG_LEVEL{"Info"};
    
    static constexpr auto WEATHER_REAL_EARTH_CONF{"RE"};
    static constexpr auto WEATHER_WMS_CONF{"WMS"};
    static constexpr auto MAX_WEATHER_SERVICES{20};
    static constexpr auto SECS_PER_MINUTE{60};
    static constexpr auto SECS_PER_DAY{24 * 60 * SECS_PER_MINUTE};
    static constexpr auto DEFAULT_WEATHER_IMAGE_SIZE{1024}; // used for texture so requires power of two
    static constexpr auto MIN_WEATHER_IMAGE_SIZE{256};      // as above
    static constexpr auto MAX_WEATHER_IMAGE_SIZE{4096};     // as above, higher values (e.g. 2048) lead to size limit exceeded so check with your prefered service

protected:
    virtual std::string get_config_name() = 0;    
    virtual std::string get_main_config_group() = 0;
    void migrateWeatherServices(uint32_t i);

    static constexpr auto GRP_WEATHER{"weather"};
    static constexpr auto WEATHER_IMAGE_SIZE{"weatherImageSize"};
    static constexpr auto WEATHER_SERVICE{"weatherService"};
    static constexpr auto WEATHER_PRODUCT{"weatherProduct"};
    static constexpr auto WEATHER_TRANSP{"weatherTransparency"};
    static constexpr auto WEATHER_MIN_PERIOD_SECONDS{"minWeatherPeriodSeconds"};
    static constexpr auto WEATHER_SERVICE_NAME{"weatherName"};
    static constexpr auto WEATHER_SERVICE_ADDRESS{"weatherAddress"};
    static constexpr auto WEATHER_SERVICE_DELAY{"weatherDelay"};
    static constexpr auto WEATHER_SERVICE_TYPE{"weatherType"};
    static constexpr auto WEATHER_SERVICE_LOCAL_TIME{"weatherLocalTime"};
    static constexpr auto MIN_UPDATE_DELAY_SEC{5 * 60};
    static constexpr auto DEF_UPDATE_DELAY_SEC{30 * 60};

    Glib::KeyFile *m_config{nullptr};
    
    int m_weatherImageSize{0};
    std::vector<std::shared_ptr<WebMapServiceConf>> m_weatherServices;

    unsigned int m_debug{0};

};
