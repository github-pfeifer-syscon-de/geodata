
# make this config persistent, to share infos at runtime -> makes in header reference possible
conf_file = configure_file( output : meson.project_name() + '_config.h'
               , configuration : conf
               , install : true
               , install_dir : 'include/' + meson.project_name())

project_headers = [
      'Spoon.hpp'
    , 'Weather.hpp'
    , 'RealEarth.hpp'
    , 'WebMapService.hpp'
    , 'GeoCoordinate.hpp'
    , 'MapProjection.hpp'
    , 'GeoJson.hpp'
    , 'GeoJson2.hpp'
    , 'BoundsDisplay.hpp'
    , 'WeatherConfigGrid.hpp'
    , 'WeatherConfig.hpp'
    , 'WeatherDialog.hpp'
    , 'Flight.hpp'
    , 'Flights.hpp'
    , 'OpenskyFlights.hpp'
    , 'GenericGeo.hpp'
    , 'GeoKlm.hpp'
]
# Make this library usable from the system's
# package manager.
install_headers(project_headers, subdir : meson.project_name())
