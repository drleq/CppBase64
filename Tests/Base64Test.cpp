#include "Tests/CppUnitTestFramework.hpp"
#include "Base64.hpp"

namespace {
    struct Base64Test {
        static std::string TestEncode(const std::string_view& str, bool padded) {
            return base64::encode_to_string(
                reinterpret_cast<const uint8_t*>(str.data()),
                str.size(),
                padded
            );
        }

        static std::string TestDecode(const std::string_view& str) {
            return base64::decode_to_string(
                reinterpret_cast<const uint8_t*>(str.data()),
                str.size()
            );
        }
    };
}

namespace base64_test {

    TEST_CASE(Base64Test, Encode) {
        SECTION("Padded") {
            CHECK_EQUAL(TestEncode("", true), "");
            CHECK_EQUAL(TestEncode("f", true), "Zg==");
            CHECK_EQUAL(TestEncode("fo", true), "Zm8=");
            CHECK_EQUAL(TestEncode("foo", true), "Zm9v");
            CHECK_EQUAL(TestEncode("foob", true), "Zm9vYg==");
            CHECK_EQUAL(TestEncode("fooba", true), "Zm9vYmE=");
            CHECK_EQUAL(TestEncode("foobar", true), "Zm9vYmFy");

            CHECK_EQUAL(
                TestEncode(
                    "Man is distinguished, not only by his reason, but by this singular passion from "
                    "other animals, which is a lust of the mind, that by a perseverance of delight "
                    "in the continued and indefatigable generation of knowledge, exceeds the short "
                    "vehemence of any carnal pleasure.",
                    true
                ),
                "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
                "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
                "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
                "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
                "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4="
            );
        }

        SECTION("Unpadded") {
            CHECK_EQUAL(TestEncode("", false), "");
            CHECK_EQUAL(TestEncode("f", false), "Zg");
            CHECK_EQUAL(TestEncode("fo", false), "Zm8");
            CHECK_EQUAL(TestEncode("foo", false), "Zm9v");
            CHECK_EQUAL(TestEncode("foob", false), "Zm9vYg");
            CHECK_EQUAL(TestEncode("fooba", false), "Zm9vYmE");
            CHECK_EQUAL(TestEncode("foobar", false), "Zm9vYmFy");

            CHECK_EQUAL(
                TestEncode(
                    "Man is distinguished, not only by his reason, but by this singular passion from "
                    "other animals, which is a lust of the mind, that by a perseverance of delight "
                    "in the continued and indefatigable generation of knowledge, exceeds the short "
                    "vehemence of any carnal pleasure.",
                    false
                ),
                "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
                "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
                "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
                "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
                "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4"
            );
        }
    }

    TEST_CASE(Base64Test, Decode) {
        SECTION("Padded") {
            CHECK_EQUAL(TestDecode(""), "");
            CHECK_EQUAL(TestDecode("Zg=="), "f");
            CHECK_EQUAL(TestDecode("Zm8="), "fo");
            CHECK_EQUAL(TestDecode("Zm9v"), "foo");
            CHECK_EQUAL(TestDecode("Zm9vYg=="), "foob");
            CHECK_EQUAL(TestDecode("Zm9vYmE="), "fooba");
            CHECK_EQUAL(TestDecode("Zm9vYmFy"), "foobar");

            CHECK_EQUAL(
                TestDecode(
                    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
                    "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
                    "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
                    "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
                    "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4="
                ),
                "Man is distinguished, not only by his reason, but by this singular passion from "
                "other animals, which is a lust of the mind, that by a perseverance of delight "
                "in the continued and indefatigable generation of knowledge, exceeds the short "
                "vehemence of any carnal pleasure."
            );
        }

        SECTION("Unpadded") {
            CHECK_EQUAL(TestDecode("Zg"), "f");
            CHECK_EQUAL(TestDecode("Zm8"), "fo");
            CHECK_EQUAL(TestDecode("Zm9v"), "foo");
            CHECK_EQUAL(TestDecode("Zm9vYg"), "foob");
            CHECK_EQUAL(TestDecode("Zm9vYmE"), "fooba");
            CHECK_EQUAL(TestDecode("Zm9vYmFy"), "foobar");

            CHECK_EQUAL(
                TestDecode(
                    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
                    "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
                    "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
                    "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
                    "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4"
                ),
                "Man is distinguished, not only by his reason, but by this singular passion from "
                "other animals, which is a lust of the mind, that by a perseverance of delight "
                "in the continued and indefatigable generation of knowledge, exceeds the short "
                "vehemence of any carnal pleasure."
            );
        }
    }

}