#include "Base64.hpp"
#include <sstream>

namespace {
    struct Base64Test {};
}

namespace CppUnitTestFramework::Ext {
    inline std::string ToString(const std::vector<uint8_t>& value) {
        std::ostringstream ss;
        ss << "vector: ";
        ss.write(
            reinterpret_cast<const char*>(value.data()),
            value.size()
        );
        return ss.str();
    }
}
#include "Tests/CppUnitTestFramework.hpp"

TEST_CASE(Base64Test, Empty) {
    CHECK_EQUAL(base64::encode_to_string(""), "");
    CHECK_EQUAL(base64::decode_to_string(""), "");
}

TEST_CASE(Base64Test, String_Padded) {
    SECTION("OneChar") {
        CHECK_EQUAL(base64::encode_to_string("a"), "YQ==");
        CHECK_EQUAL(base64::decode_to_string("YQ=="), "a");
    }
    SECTION("TwoChar") {
        CHECK_EQUAL(base64::encode_to_string("aa"), "YWE=");
        CHECK_EQUAL(base64::decode_to_string("YWE="), "aa");
    }
    SECTION("ThreeChar") {
        CHECK_EQUAL(base64::encode_to_string("aaa"), "YWFh");
        CHECK_EQUAL(base64::decode_to_string("YWFh"), "aaa");
    }
}

TEST_CASE(Base64Test, String_UnPadded) {
    SECTION("OneChar") {
        CHECK_EQUAL(base64::encode_to_string("a", false), "YQ");
        CHECK_EQUAL(base64::decode_to_string("YQ"), "a");
    }
    SECTION("TwoChar") {
        CHECK_EQUAL(base64::encode_to_string("aa", false), "YWE");
        CHECK_EQUAL(base64::decode_to_string("YWE"), "aa");
    }
    SECTION("ThreeChar") {
        CHECK_EQUAL(base64::encode_to_string("aaa", false), "YWFh");
        CHECK_EQUAL(base64::decode_to_string("YWFh"), "aaa");
    }
}

TEST_CASE(Base64Test, Vector_Padded) {
    SECTION("OneChar") {
        CHECK_EQUAL(base64::encode_to_byte_vector("a"), std::vector<uint8_t>({ 'Y', 'Q', '=', '=' }));
        CHECK_EQUAL(base64::decode_to_byte_vector("YQ=="), std::vector<uint8_t>({ 'a' }));
    }
    SECTION("TwoChar") {
        CHECK_EQUAL(base64::encode_to_byte_vector("aa"), std::vector<uint8_t>({ 'Y', 'W', 'E', '=' }));
        CHECK_EQUAL(base64::decode_to_byte_vector("YWE="), std::vector<uint8_t>({ 'a', 'a' }));
    }
    SECTION("ThreeChar") {
        CHECK_EQUAL(base64::encode_to_byte_vector("aaa"), std::vector<uint8_t>({ 'Y', 'W', 'F', 'h' }));
        CHECK_EQUAL(base64::decode_to_byte_vector("YWFh"), std::vector<uint8_t>({ 'a', 'a', 'a' }));
    }
}

TEST_CASE(Base64Test, Vector_UnPadded) {
    SECTION("OneChar") {
        CHECK_EQUAL(base64::encode_to_byte_vector("a", false), std::vector<uint8_t>({ 'Y', 'Q' }));
        CHECK_EQUAL(base64::decode_to_byte_vector("YQ"), std::vector<uint8_t>({ 'a' }));
    }
    SECTION("TwoChar") {
        CHECK_EQUAL(base64::encode_to_byte_vector("aa", false), std::vector<uint8_t>({ 'Y', 'W', 'E' }));
        CHECK_EQUAL(base64::decode_to_byte_vector("YWE"), std::vector<uint8_t>({ 'a', 'a' }));
    }
    SECTION("ThreeChar") {
        CHECK_EQUAL(base64::encode_to_byte_vector("aaa", false), std::vector<uint8_t>({ 'Y', 'W', 'F', 'h' }));
        CHECK_EQUAL(base64::decode_to_byte_vector("YWFh"), std::vector<uint8_t>({ 'a', 'a', 'a' }));
    }
}