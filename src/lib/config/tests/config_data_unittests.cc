
// Copyright (C) 2009  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <gtest/gtest.h>

#include <config/tests/data_def_unittests_config.h>
#include <config/config_data.h>

#include <iostream>

using namespace bundy::data;
using namespace bundy::config;

ConfigData
setupSpec2() {
    ModuleSpec spec2 = moduleSpecFromFile(std::string(TEST_DATA_PATH) + "/spec22.spec");
    return (ConfigData(spec2));
}

TEST(ConfigData, Creation) {
    ConfigData cd = setupSpec2();
    EXPECT_TRUE(true);
}

TEST(ConfigData, getValue) {
    ModuleSpec spec22 = moduleSpecFromFile(std::string(TEST_DATA_PATH) + "/spec22.spec");
    ConfigData cd = ConfigData(spec22);
    //std::cout << "[XX] SPEC2: " << cd.getModuleSpec().getFullSpec() << std::endl;
    bool is_default;
    //ElementPtr value = cd.getValue(is_default, "item1");
    EXPECT_EQ(9, cd.getValue("value1")->intValue());
    EXPECT_EQ(9, cd.getValue(is_default, "value1")->intValue());
    EXPECT_TRUE(is_default);
    EXPECT_EQ(9.9, cd.getValue("value2")->doubleValue());
    EXPECT_EQ(9.9, cd.getValue(is_default, "value2")->doubleValue());
    EXPECT_TRUE(is_default);
    EXPECT_FALSE(cd.getValue("value3")->boolValue());
    EXPECT_FALSE(cd.getValue(is_default, "value3")->boolValue());
    EXPECT_TRUE(is_default);
    EXPECT_EQ("default_string", cd.getValue("value4")->stringValue());
    EXPECT_EQ("default_string", cd.getValue(is_default, "value4")->stringValue());
    EXPECT_TRUE(is_default);
    EXPECT_EQ("a", cd.getValue("value5")->get(0)->stringValue());
    EXPECT_EQ("a", cd.getValue(is_default, "value5")->get(0)->stringValue());
    EXPECT_TRUE(is_default);
    EXPECT_EQ("b", cd.getValue("value5")->get(1)->stringValue());
    EXPECT_EQ("b", cd.getValue(is_default, "value5")->get(1)->stringValue());
    EXPECT_EQ("b", cd.getValue(is_default, "value5/")->get(1)->stringValue());
    EXPECT_TRUE(is_default);
    EXPECT_EQ("{  }", cd.getValue("value6")->str());
    EXPECT_EQ("{  }", cd.getValue(is_default, "value6")->str());
    EXPECT_EQ("{  }", cd.getValue(is_default, "value6/")->str());
    EXPECT_TRUE(is_default);
    EXPECT_EQ("[  ]", cd.getValue("value8")->str());
    EXPECT_EQ("[  ]", cd.getDefaultValue("value8")->str());
    EXPECT_EQ("empty", cd.getValue("value8/a")->stringValue());

    EXPECT_THROW(cd.getValue("")->str(), DataNotFoundError);
    EXPECT_THROW(cd.getValue("/")->str(), DataNotFoundError);
    EXPECT_THROW(cd.getValue("no_such_item")->str(), DataNotFoundError);
    EXPECT_THROW(cd.getValue("value6/a")->str(), DataNotFoundError);
    EXPECT_THROW(cd.getValue("value6/no_such_item")->str(), DataNotFoundError);
    EXPECT_THROW(cd.getValue("value8/b")->str(), DataNotFoundError);

    // The default generation ID is 0.
    EXPECT_EQ(0, cd.getValue("_generation_id")->intValue());
    EXPECT_EQ(0, cd.getValue(is_default, "_generation_id")->intValue());
    EXPECT_TRUE(is_default);

    ModuleSpec spec1 = moduleSpecFromFile(std::string(TEST_DATA_PATH) + "/spec1.spec");
    ConfigData cd1 = ConfigData(spec1);
    EXPECT_THROW(cd1.getValue("anything")->str(), DataNotFoundError);
}

TEST(ConfigData, getDefaultValue) {
    ModuleSpec spec31 = moduleSpecFromFile(std::string(TEST_DATA_PATH) + "/spec31.spec");
    ConfigData cd = ConfigData(spec31);
    EXPECT_EQ("[  ]", cd.getDefaultValue("first_list_items")->str());
    EXPECT_EQ("\"foo\"", cd.getDefaultValue("first_list_items/foo")->str());
    EXPECT_EQ("{  }", cd.getDefaultValue(
                  "first_list_items/second_list_items/map_element")->str());
    EXPECT_EQ("[  ]", cd.getDefaultValue(
                  "first_list_items/second_list_items/map_element/list1")->
              str());
    EXPECT_EQ("1", cd.getDefaultValue(
                  "first_list_items/second_list_items/map_element/list1/number")
              ->str());

    EXPECT_EQ(cd.getDefaultValue("_generation_id")->intValue(), 0);

    EXPECT_THROW(cd.getDefaultValue("doesnotexist")->str(), DataNotFoundError);
    EXPECT_THROW(cd.getDefaultValue("first_list_items/second_list_items/map_element/list1/doesnotexist")->str(), DataNotFoundError);
}


TEST(ConfigData, setLocalConfig) {
    ModuleSpec spec2 = moduleSpecFromFile(std::string(TEST_DATA_PATH) + "/spec2.spec");
    ConfigData cd = ConfigData(spec2);
    bool is_default;

    ElementPtr my_config = Element::fromJSON("{ \"item1\": 2 }");
    ElementPtr my_config2 = Element::fromJSON("{ \"item6\": { \"value1\": \"a\" } }");

    EXPECT_EQ("{  }", cd.getValue("item6")->str());

    cd.setLocalConfig(my_config);
    EXPECT_EQ(2, cd.getValue(is_default, "item1")->intValue());
    EXPECT_FALSE(is_default);
    EXPECT_EQ("{  }", cd.getValue("item6")->str());
    EXPECT_EQ(1.1, cd.getValue(is_default, "item2")->doubleValue());
    EXPECT_TRUE(is_default);

    cd.setLocalConfig(my_config2);
    EXPECT_EQ("{ \"value1\": \"a\" }", cd.getValue("item6")->str());
}

TEST(ConfigData, getLocalConfig) {
    ModuleSpec spec2 = moduleSpecFromFile(std::string(TEST_DATA_PATH) + "/spec2.spec");
    ConfigData cd = ConfigData(spec2);
    EXPECT_EQ("{  }", cd.getLocalConfig()->str());

    ElementPtr my_config = Element::fromJSON("{ \"item1\": 2 }");
    cd.setLocalConfig(my_config);
    EXPECT_EQ("{ \"item1\": 2 }", cd.getLocalConfig()->str());

    ElementPtr my_config2 = Element::fromJSON("{ \"item6\": { \"value1\": \"a\" } }");
    cd.setLocalConfig(my_config2);
    EXPECT_EQ("{ \"item6\": { \"value1\": \"a\" } }", cd.getLocalConfig()->str());
}

TEST(ConfigData, getItemList) {
    const ModuleSpec spec2 = moduleSpecFromFile(std::string(TEST_DATA_PATH) +
                                                "/spec2.spec");
    const ConfigData cd = ConfigData(spec2);

    // Top-level search, non recursive.  Reserved item included.  If the top
    // level is specified as "/", this and an additional slash will be
    // prepended to non-reserved items (although they won't be accessible via
    // getValue(), so probably less useful in practice).
    EXPECT_EQ("[ \"item1\", \"item2\", \"item3\", \"item4\", \"item5\", "
              "\"item6\", \"_generation_id\" ]", cd.getItemList()->str());
    EXPECT_EQ("[ \"//item1\", \"//item2\", \"//item3\", \"//item4\", "
              "\"//item5\", \"//item6\", \"_generation_id\" ]",
              cd.getItemList("/")->str());
    // Top-level search, recursive.  Reserved item included.
    EXPECT_EQ("[ \"item1\", \"item2\", \"item3\", \"item4\", \"item5\", "
              "\"item6/value1\", \"item6/value2\", \"_generation_id\" ]",
              cd.getItemList("", true)->str());
    // Search at a specific level.  Reserved item NOT included.
    EXPECT_EQ("[ \"item6/value1\", \"item6/value2\" ]",
              cd.getItemList("item6")->str());
}

TEST(ConfigData, getFullConfig) {
    const ModuleSpec spec2 = moduleSpecFromFile(std::string(TEST_DATA_PATH) +
                                                "/spec2.spec");
    ConfigData cd = ConfigData(spec2);

    // Default config values
    EXPECT_EQ("{ \"_generation_id\": 0, \"item1\": 1, \"item2\": 1.1, "
              "\"item3\": true, \"item4\": \"test\", "
              "\"item5\": [ \"a\", \"b\" ], \"item6\": {  } }",
              cd.getFullConfig()->str());

    // Partially update the config, then the local and default will be merged.
    const ElementPtr my_config = Element::fromJSON(
        "{ \"_generation_id\": 1, \"item1\": 2 }");
    cd.setLocalConfig(my_config);
    EXPECT_EQ("{ \"_generation_id\": 1, \"item1\": 2, \"item2\": 1.1, "
              "\"item3\": true, \"item4\": \"test\", "
              "\"item5\": [ \"a\", \"b\" ], \"item6\": {  } }",
              cd.getFullConfig()->str());
    const ElementPtr my_config2 =
        Element::fromJSON("{ \"item6\": { \"value1\": \"a\" } }");
    cd.setLocalConfig(my_config2);
    EXPECT_EQ("{ \"_generation_id\": 0, \"item1\": 1, \"item2\": 1.1, "
              "\"item3\": true, \"item4\": \"test\", "
              "\"item5\": [ \"a\", \"b\" ], "
              "\"item6\": { \"value1\": \"a\" } }", cd.getFullConfig()->str());
    const ElementPtr my_config3 =
        Element::fromJSON("{ \"item6\": { \"value2\": 123 } }");
    cd.setLocalConfig(my_config3);
    EXPECT_EQ("{ \"_generation_id\": 0, \"item1\": 1, \"item2\": 1.1, "
              "\"item3\": true, \"item4\": \"test\", "
              "\"item5\": [ \"a\", \"b\" ], \"item6\": { \"value2\": 123 } }",
              cd.getFullConfig()->str());
}

