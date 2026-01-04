/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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
#include <memory>
#include <stack>
#include <glibmm.h>

#include "Weather.hpp"
#include "GeoCoordinate.hpp"

class WebMapService;
class WebMapProduct;

class WebMapImageRequest
: public WeatherImageRequest
{
public:
    WebMapImageRequest(WebMapService* webMapService
        , const GeoBounds& bounds
        , int pixX, int pixY, int pixWidth, int pixHeight
        , std::shared_ptr<WebMapProduct>& product);
    virtual ~WebMapImageRequest() = default;
    void mapping(Glib::RefPtr<Gdk::Pixbuf> pix, Glib::RefPtr<Gdk::Pixbuf>& weather_pix);

private:
    WebMapService *m_webMapService;
    GeoBounds m_bounds;
    int m_pixX;
    int m_pixY;
    int m_pixWidth;
    int m_pixHeight;

};

enum class ParseContext {
    None,
    Name,
    Title,
    Abstract,
    KeywordList,
    Keyword,
    CRS,
    EX_GeographicBoundingBox,
    westBoundLongitude,
    eastBoundLongitude,
    southBoundLatitude,
    northBoundLatitude,
    BoundingBox,
    Dimension,
    Style,
    LegendURL,
    Format,
    OnlineResource,
    Attribution,
    MinScaleDenominator,
    MaxScaleDenominator
};

// Example
/*
<Name>copernicus:sentinel3a_olci_l2_chl_fullres</Name>
<Title>OLCI Level 2 CHL Concentration - Sentinel-3A</Title>
<Abstract>
This Ocean Colour product represents the algal pigment (Chlorophyll a) concentration in clear open waters, and it is defined by the "OC4Me" Maximum Band Ratio (MBR) semi-analytical algorithm. The product is derived from S3A OLCI L2 NRT water-leaving reflectances (calculated from the Baseline Atmospheric Correction). A maximum band ratio approach is used for reflectances at 443, 490 and 510 nm, over that 560 nm (O3 to O6). It is expressed in Units of mg/m3 as: log10 [Chl]= ∑4 x=0 =(Ax * (log10(Rij))x) which is the ratio of reflectance of band i, among 443, 490 and 510 nm, over that of band j at 560 nm. The following flags were applied: INVALID, LAND, CLOUD, CLOUD_AMBIGUOUS, CLOUD_MARGIN, SNOW_ICE, SUSPECT, HISOLZEN, SATURATED, HIGHGLINT, WHITECAPS, AC_FAIL, OC4ME_FAIL, ANNOT_TAU06, RWNEG_O2, RWNEG_O3, RWNEG_O4, RWNEG_O5, RWNEG_O6, RWNEG_O7 and RWNEG_O8. Full details of the chl_oc4me algorithm can be found in the case 1 resource link. Sentinel-3 is part of a series of Sentinel satellites, under the umbrella of the EU Copernicus programme.
</Abstract>
<KeywordList>
<Keyword>copernicus_sentinel3a_olci_l2_chl_fullres</Keyword>
<Keyword>WCS</Keyword>
<Keyword>ImageMosaic</Keyword>
</KeywordList>
<CRS>EPSG:4326</CRS>
<CRS>CRS:84</CRS>
<EX_GeographicBoundingBox>
<westBoundLongitude>-180.0</westBoundLongitude>
<eastBoundLongitude>180.010009765625</eastBoundLongitude>
<southBoundLatitude>-84.317268371582</southBoundLatitude>
<northBoundLatitude>69.792121887207</northBoundLatitude>
</EX_GeographicBoundingBox>
<BoundingBox CRS="CRS:84" minx="-180.0" miny="-84.317268371582" maxx="180.010009765625" maxy="69.792121887207"/>
<BoundingBox CRS="EPSG:4326" minx="-84.317268371582" miny="-180.0" maxx="69.792121887207" maxy="180.010009765625"/>
<Dimension name="time" default="2023-05-10T08:55:00Z" units="ISO8601" nearestValue="1">
2020-02-17T03:01:00.000Z/2023-05-10T08:55:00.000Z/PT1H41M
</Dimension>
<Style>
<Name>olci_l2_details_gradient</Name>
<Title>SLD OLCI L2 CC CHL DETAILS GRADIENT</Title>
<LegendURL width="640" height="80">
<Format>image/png</Format>
<OnlineResource xlink:type="simple" xlink:href="https://view.eumetsat.int/geoserver/ows?service=WMS&request=GetLegendGraphic&format=image%2Fpng&width=20&height=20&layer=copernicus%3Asentinel3a_olci_l2_chl_fullres"/>
</LegendURL>
</Style>
<Style>
<Name>olci_l2_full_gradient</Name>
<Title>SLD OLCI L2 CC CHL Gradient</Title>
<LegendURL width="640" height="80">
<Format>image/png</Format>
<OnlineResource xlink:type="simple" xlink:href="https://view.eumetsat.int/geoserver/ows?service=WMS&request=GetLegendGraphic&format=image%2Fpng&width=20&height=20&layer=copernicus%3Asentinel3a_olci_l2_chl_fullres&style=olci_l2_full_gradient"/>
</LegendURL>
</Style> */
class WebMapProduct
: public WeatherProduct
{
public:
    WebMapProduct(WebMapService* webMapService);
    virtual ~WebMapProduct() = default;

    void start_element(Glib::Markup::ParseContext& context,
	const Glib::ustring& element_name,
	const Glib::Markup::Parser::AttributeMap& attributes);
    void end_element(Glib::Markup::ParseContext& context,
        const Glib::ustring& element_name);
    void text(Glib::Markup::ParseContext& context,
        const Glib::ustring& text);

    Glib::DateTime getLatestTime();
    Glib::RefPtr<Gdk::Pixbuf> get_legend() override;
    void set_legend(Glib::RefPtr<Gdk::Pixbuf>& legend) override;
    Glib::ustring get_description() override;
    bool latest(Glib::DateTime& datetime) override;
    bool is_displayable() override;
    Glib::ustring get_dimension() override;
    Glib::ustring get_legend_url();
    CoordRefSystem getCoordRefSystem();
    bool is_latest();


    static constexpr auto SECS_PER_MINUTE{60};
    static constexpr auto SECS_PER_HOUR{60 * SECS_PER_MINUTE};
    static constexpr auto SECS_PER_DAY{24 * SECS_PER_HOUR};
    static constexpr auto SECS_PER_MONTH{30 * SECS_PER_DAY};
    static constexpr auto SECS_PRE_YEAR{364 * SECS_PER_DAY};
protected:
private:
    void parseDimension(const Glib::ustring& text);
    int periodSeconds(const Glib::ustring& timeDimPeriod);

    Glib::ustring m_abstract;
    Glib::ustring m_keywords;
    CoordRefSystem m_crs{CoordRefSystem::None};
    Glib::ustring m_attribution;
    Glib::ustring m_timeDimStart;
    Glib::ustring m_timeDimEnd;
    Glib::ustring m_timeDimPeriod;
    int m_timePeriodSec;
    ParseContext m_context{ParseContext::None};
    std::stack<ParseContext>  m_parseLevel;
    std::vector<Glib::ustring> m_legends;
    Glib::RefPtr<Gdk::Pixbuf> m_legendImage;
    Glib::ustring m_LastLegendWidth;
    WebMapService* m_webMapService;
    Glib::ustring m_dimension;
};

class WebMapService
: public Weather
{
public:
    WebMapService(WeatherConsumer* consumer, const std::shared_ptr<WebMapServiceConf>& mapServiceConf, int minPeriodSec);
    virtual ~WebMapService() = default;

    std::shared_ptr<WebMapServiceConf> getServiceConf()
    {
        return m_mapServiceConf;
    }
    int getMinPeriodSec()
    {
        return m_minPeriodSec;
    }
protected:
    void capabilities();
    void inst_on_capabilities_callback(const Glib::ustring& error, int status, SpoonMessageDirect* message);
    void request(const Glib::ustring& productId) override;
    void check_product(const Glib::ustring& weatherProductId) override;
    Glib::RefPtr<Gdk::Pixbuf> get_legend(std::shared_ptr<WeatherProduct>& product);
    std::shared_ptr<WebMapServiceConf> m_mapServiceConf;
private:

    int m_minPeriodSec;
};

class NXMLParser : public Glib::Markup::Parser {
public:
    NXMLParser(WebMapService* webMapService);
    virtual ~NXMLParser() = default;
protected:
    void on_start_element(Glib::Markup::ParseContext& context,
		const Glib::ustring& element_name,
		const Glib::Markup::Parser::AttributeMap& attributes) override;
    void on_end_element(Glib::Markup::ParseContext& context,
		const Glib::ustring& element_name ) override;
    void on_text(Glib::Markup::ParseContext& context,
		const Glib::ustring& text) override;
    void on_error(Glib::Markup::ParseContext& context,
		const Glib::MarkupError& error) override;
protected:

private:
    WebMapService *m_webMapService;
    std::shared_ptr<WebMapProduct> m_webMapProduct;
};

