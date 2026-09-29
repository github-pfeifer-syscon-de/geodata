
public_headers = include_directories('../include')

geo_test = executable('geo_test'
    , 'geo_test.cpp'
    , dependencies: deps
    , include_directories : public_headers
    , link_with : project_target)
test('geo_test', geo_test)

# needs file(s) to test as parameter
geo2_test = executable('geo2_test'
    , 'geo2_test.cpp'
    , dependencies: deps
    , include_directories : public_headers
    , link_with : project_target)

# not local so leave this for manual testing
flight_test = executable('flight_test'
    , 'FlightTest.cpp'
    , dependencies: deps
    , include_directories : public_headers
    , link_with : project_target)
