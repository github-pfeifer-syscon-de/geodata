/*
 * Copyright (C) 2024 RPf <gpl3@pfeifer-syscon.de>
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
#include <cstdlib>
#include <cmath>
#include <iterator>

#include "GeoCoordinate.hpp"


// test conversion functions for C-locale
static bool
convertTest()
{
    std::cout << "convertTest --------------" << std::endl;
    auto str = GeoCoordinate::formatDouble(3.141527, std::chars_format::fixed, 4);
    std::cout << "fmt " << str << std::endl;
    if (str != "3.1415") {
        std::cout << "fmt not matching" << std::endl;
        return false;
    }
    auto val = GeoCoordinate::parseDouble("3.141527");
    std::cout << "val " << val << std::endl;
    if (std::abs(val - 3.141527) > 0.000001) {
        std::cout << "val not matching" << std::endl;
        return false;
    }
    std::cout << "convertTest --------------" << std::endl;
    return true;
}


int
main(int argc, char** argv) {
    setlocale(LC_ALL, "");      // use locale formating
    if (!convertTest()) {
        return 1;
    }

    return 0;
}

