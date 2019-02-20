#include "Tests/CppUnitTestFramework.hpp"
#include "Base64.hpp"

namespace {
    struct Base64Test {};
}

TEST_CASE(Base64Test, Simple) {
    const std::string Input = "Hello world";

    auto encoded = base64::encode_to_string(Input);
    auto decoded = base64::decode_to_string(encoded);
}