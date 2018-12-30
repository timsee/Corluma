/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "catch.hpp"
#include "dictionary.h"
#include "helpers/mocklight.h"

TEST_CASE( "Dictionary Standard API", "[dictionary]" ) {
    cor::Dictionary<int> dict;
    dict.insert("four thirty three", 433);

    SECTION("insert") {
        REQUIRE(dict.size() == 1);
        dict.insert("four twenty one", 421);
        REQUIRE(dict.size() == 2);
    }

    SECTION("remove by item") {
        REQUIRE(dict.size() == 1);
        bool result = dict.remove(433);
        REQUIRE(result == true);
        REQUIRE(dict.size() == 0);
    }

    SECTION("remove invalid item") {
       bool result = dict.remove(333);
       REQUIRE(result == false);
       REQUIRE(dict.size() == 1);
    }

    SECTION("remove by key") {
        REQUIRE(dict.size() == 1);
        bool result = dict.removeKey("four thirty three");
        REQUIRE(result == true);
        REQUIRE(dict.size() == 0);
    }

    SECTION("remove invalid key") {
        bool result = dict.removeKey("invalid key");
        REQUIRE(result == false);
        REQUIRE(dict.size() == 1);
    }

    SECTION("key from item") {
        auto result = dict.key(433);
        REQUIRE(result.second == true);
        REQUIRE(result.first == "four thirty three");
    }

    SECTION("item from key") {
        auto result = dict.item("four thirty three");
        REQUIRE(result.second == true);
        REQUIRE(result.first == 433);
    }

    SECTION("update") {
        cor::Dictionary<mock::Light> dict;
        mock::Light light("light1", 23);
        mock::Light lightUpdated("light1", 24);

        REQUIRE(dict.size() == 0);
        dict.insert(light.uniqueID, light);
        REQUIRE(dict.size() == 1);
        auto returnLightResult = dict.item("light1");
        REQUIRE(returnLightResult.second == true);
        REQUIRE(returnLightResult.first.value == 23);
        dict.update(lightUpdated.uniqueID, lightUpdated);
        returnLightResult = dict.item("light1");
        REQUIRE(returnLightResult.second == true);
        REQUIRE(returnLightResult.first.value == 24);
    }
}


TEST_CASE( "Different Datatypes", "[dictionary-datatypes]" ) {
    SECTION("std::string") {
        cor::Dictionary<std::string> dict;
        REQUIRE(dict.size() == 0);
        dict.insert("key", "item");
        REQUIRE(dict.size() == 1);
        dict.insert("key2", "item2");
        REQUIRE(dict.size() == 2);
        dict.insert("key3", "item3");
        REQUIRE(dict.size() == 3);
        bool result = dict.remove("item3");
        REQUIRE(result == true);
        REQUIRE(dict.size() == 2);
        result = dict.removeKey("key");
        REQUIRE(result == true);
        auto keyResult = dict.key("item2");
        REQUIRE(keyResult.second == true);
        REQUIRE(keyResult.first == "key2");
    }


    SECTION("std::string matches key") {
        cor::Dictionary<std::string> dict;
        REQUIRE(dict.size() == 0);
        bool result = dict.insert("key", "key");
        REQUIRE(result == true);
        REQUIRE(dict.size() == 1);
        result = dict.insert("key2", "key2");
        REQUIRE(dict.size() == 2);
        REQUIRE(result == true);
    }

    SECTION("float") {
        cor::Dictionary<float> dict;
        REQUIRE(dict.size() == 0);
        dict.insert("key", 0.5f);
        REQUIRE(dict.size() == 1);
        dict.insert("key2", 1.0f);
        REQUIRE(dict.size() == 2);
        dict.insert("key3", 2.0f);
        REQUIRE(dict.size() == 3);
        bool result = dict.remove(2.0f);
        REQUIRE(result == true);
        REQUIRE(dict.size() == 2);
        result = dict.removeKey("key");
        REQUIRE(result == true);
        auto keyResult = dict.key(1.0f);
        REQUIRE(keyResult.second == true);
        REQUIRE(keyResult.first == "key2");
    }

    SECTION("custom struct") {
        cor::Dictionary<mock::Light> dict;
        mock::Light light1("light1", 23);
        mock::Light light2("light2", 24);
        mock::Light light3("light3", 23);

        REQUIRE(dict.size() == 0);
        dict.insert(light1.uniqueID, light1);
        REQUIRE(dict.size() == 1);
        dict.insert(light2.uniqueID, light2);
        REQUIRE(dict.size() == 2);
        dict.insert(light3.uniqueID, light3);
        REQUIRE(dict.size() == 3);
        bool result = dict.remove(light3);
        REQUIRE(result == true);
        REQUIRE(dict.size() == 2);
        result = dict.removeKey(light1.uniqueID);
        REQUIRE(result == true);
        auto keyResult = dict.key(light2);
        REQUIRE(keyResult.second == true);
        REQUIRE(keyResult.first == light2.uniqueID);
    }
}
