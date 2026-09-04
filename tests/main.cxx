#include "libjson/json.hpp"
#include "libjson/parser_error.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

TEST(JsonParseTest, ParsesNull) {
  libjson::Json json = libjson::Json::parse("null");

  EXPECT_TRUE(json.is_null());
}

TEST(JsonParseTest, ParsesBooleans) {
  libjson::Json true_value = libjson::Json::parse("true");
  libjson::Json false_value = libjson::Json::parse("false");

  ASSERT_TRUE(true_value.is_bool());
  ASSERT_TRUE(false_value.is_bool());
  EXPECT_TRUE(true_value.as_bool());
  EXPECT_FALSE(false_value.as_bool());
}

TEST(JsonParseTest, ParsesNumbers) {
  EXPECT_DOUBLE_EQ(libjson::Json::parse("0").as_number(), 0.0);
  EXPECT_DOUBLE_EQ(libjson::Json::parse("-12").as_number(), -12.0);
  EXPECT_DOUBLE_EQ(libjson::Json::parse("3.5").as_number(), 3.5);
  EXPECT_DOUBLE_EQ(libjson::Json::parse("1.25e2").as_number(), 125.0);
  EXPECT_DOUBLE_EQ(libjson::Json::parse("-2E-3").as_number(), -0.002);
}

TEST(JsonParseTest, ParsesStrings) {
  libjson::Json json = libjson::Json::parse(R"("hello")");

  ASSERT_TRUE(json.is_string());
  EXPECT_EQ(json.as_string(), "hello");
}

TEST(JsonParseTest, ParsesStringEscapes) {
  libjson::Json json = libjson::Json::parse(R"("quote: \" slash: \\ newline: \n tab: \t")");

  EXPECT_EQ(json.as_string(), "quote: \" slash: \\ newline: \n tab: \t");
}

TEST(JsonParseTest, ParsesUnicodeEscapes) {
  libjson::Json json = libjson::Json::parse(R"("\u0041\u00A3\u20AC")");

  EXPECT_EQ(json.as_string(), "A\xC2\xA3\xE2\x82\xAC");
}

TEST(JsonParseTest, ParsesUnicodeSurrogatePairs) {
  libjson::Json json = libjson::Json::parse(R"("\uD83D\uDE00")");

  EXPECT_EQ(json.as_string(), "\xF0\x9F\x98\x80");
}

TEST(JsonParseTest, ParsesArrays) {
  libjson::Json json = libjson::Json::parse(R"([null,true,2,"three"])");

  ASSERT_TRUE(json.is_array());
  ASSERT_EQ(json.as_array().size(), 4U);
  EXPECT_TRUE(json[0].is_null());
  EXPECT_TRUE(json[1].as_bool());
  EXPECT_DOUBLE_EQ(json[2].as_number(), 2.0);
  EXPECT_EQ(json[3].as_string(), "three");
}

TEST(JsonParseTest, ParsesObjects) {
  libjson::Json json = libjson::Json::parse(R"({"name":"Alice","age":30,"admin":false})");

  ASSERT_TRUE(json.is_object());
  EXPECT_EQ(json["name"].as_string(), "Alice");
  EXPECT_DOUBLE_EQ(json["age"].as_number(), 30.0);
  EXPECT_FALSE(json["admin"].as_bool());
}

TEST(JsonParseTest, ParsesNestedValuesAndWhitespace) {
  libjson::Json json = libjson::Json::parse(R"(
    {
      "user": {
        "name": "Alice",
        "roles": ["admin", "user"]
      }
    }
  )");

  EXPECT_EQ(json["user"]["name"].as_string(), "Alice");
  EXPECT_EQ(json["user"]["roles"][0].as_string(), "admin");
  EXPECT_EQ(json["user"]["roles"][1].as_string(), "user");
}

TEST(JsonParseTest, RejectsInvalidJson) {
  const std::string invalid_inputs[] = {
      "",
      "nul",
      "truth",
      "01",
      "-",
      "1.",
      "1e",
      R"("unterminated)",
      R"("bad\q")",
      "\"bad\n\"",
      R"("\uZZZZ")",
      R"("\uD800")",
      "[1,]",
      "[1 2]",
      R"({"name"})",
      R"({"name":})",
      R"({"name":"Alice" "age":30})",
  };

  for (const std::string &input : invalid_inputs) {
    EXPECT_THROW(libjson::Json::parse(input), libjson::parse_error) << input;
  }
}

TEST(JsonStringifyTest, StringifiesScalars) {
  EXPECT_EQ(libjson::Json(nullptr).stringify(), "null");
  EXPECT_EQ(libjson::Json(true).stringify(), "true");
  EXPECT_EQ(libjson::Json(false).stringify(), "false");
  EXPECT_EQ(libjson::Json(42).stringify(), "42");
  EXPECT_EQ(libjson::Json("hello").stringify(), R"("hello")");
}

TEST(JsonStringifyTest, StringifiesEscapedStrings) {
  libjson::Json json(std::string("quote: \" slash: \\ newline: \n tab: \t"));

  EXPECT_EQ(json.stringify(), R"("quote: \" slash: \\ newline: \n tab: \t")");
}

TEST(JsonStringifyTest, StringifiesArrays) {
  libjson::Json::array values = {nullptr, true, 2, "three"};
  libjson::Json json(values);

  EXPECT_EQ(json.stringify(), R"([null,true,2,"three"])");
}

TEST(JsonStringifyTest, StringifiesObjectsWithSortedKeys) {
  libjson::Json::object values;
  values["b"] = 2;
  values["a"] = true;

  libjson::Json json(values);

  EXPECT_EQ(json.stringify(), R"({"a":true,"b":2})");
}

TEST(JsonValueTest, SupportsMutableObjectAccess) {
  libjson::Json json(libjson::Json::object{});

  json["name"] = "Alice";
  json["age"] = 30;

  EXPECT_EQ(json["name"].as_string(), "Alice");
  EXPECT_DOUBLE_EQ(json["age"].as_number(), 30.0);
}

TEST(JsonValueTest, SupportsMutableArrayAccess) {
  libjson::Json json(libjson::Json::array{1, 2, 3});

  json[1] = 20;

  EXPECT_DOUBLE_EQ(json[1].as_number(), 20.0);
}

TEST(JsonValueTest, ThrowsForWrongAccessType) {
  libjson::Json json("hello");

  EXPECT_THROW(json.as_number(), std::bad_variant_access);
}
 
int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
