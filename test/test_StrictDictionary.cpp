/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "catch.hpp"
#include "strictdictionary.h"
#include "helpers/mocklight.h"

TEST_CASE( "StrictDictionary Standard API", "[strict-dictionary]" ) {
    cor::StrictDictionary<int> dict;
    dict.insert("four twenty", 420);

    SECTION("insert") {
        REQUIRE(dict.size() == 1);
        dict.insert("four twenty one", 421);
        REQUIRE(dict.size() == 2);
    }

    SECTION("remove by item") {
        REQUIRE(dict.size() == 1);
        dict.remove(420);
        REQUIRE(dict.size() == 0);
    }

    SECTION("remove invalid item") {
       REQUIRE_THROWS_AS(dict.remove(333), cor::Exception);
       REQUIRE(dict.size() == 1);
    }

    SECTION("remove by key") {
        REQUIRE(dict.size() == 1);
        dict.removeKey("four twenty");
        REQUIRE(dict.size() == 0);
    }

    SECTION("remove invalid key") {
        REQUIRE_THROWS_AS(dict.removeKey("invalid key"), cor::Exception);
        REQUIRE(dict.size() == 1);
    }

    SECTION("key from item") {
        std::string key = dict.key(420);
        REQUIRE(key == "four twenty");
    }

    SECTION("item from key") {
        int item = dict.item("four twenty");
        REQUIRE(item == 420);
    }

    SECTION("update") {
        cor::StrictDictionary<mock::Light> dict;
        mock::Light light("light1", 23);
        mock::Light lightUpdated("light1", 24);

        REQUIRE(dict.size() == 0);
        dict.insert(light.uniqueID, light);
        REQUIRE(dict.size() == 1);
        auto returnLight = dict.item("light1");
        REQUIRE(returnLight.value == 23);
        dict.update(lightUpdated);
        returnLight = dict.item("light1");
        REQUIRE(returnLight.value == 24);
    }


    SECTION("std::string") {
        cor::StrictDictionary<std::string> dict;
        REQUIRE(dict.size() == 0);
        dict.insert("key", "item");
        REQUIRE(dict.size() == 1);
        dict.insert("key2", "item2");
        REQUIRE(dict.size() == 2);
        dict.insert("key3", "item3");
        REQUIRE(dict.size() == 3);
        dict.remove("item3");
        REQUIRE(dict.size() == 2);
        dict.removeKey("key");
        REQUIRE(dict.size() == 1);
        auto keyResult = dict.key("item2");
        REQUIRE(keyResult == "key2");
    }
}
