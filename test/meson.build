
public_headers = include_directories('../include')

geo_test = executable('geo_test'
    , 'geo_test.cpp'
    , dependencies: deps
    , include_directories : public_headers
    , link_with : project_target)
test('geo_test', geo_test)

