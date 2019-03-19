# CppBase64
A single header C++17 compatible [base64](https://en.wikipedia.org/wiki/Base64) library.  It should compile with any C++17 compliant compiler without any additional source or binary dependencies.  SSSE3 and AVX2 codepaths can be used to accelerate processing.

# Basic usage
The library provides two low-level functions for handling encoding and decoding of base64 buffers.  They accept raw buffers, including lengths, and decode directly into them.  Utility methods are provided for calculating the sizes of buffers.

## Encoding
```cpp
// Input data
const uint8_t* data = ...;
size_t data_length = ...;

// Encoded buffer
size_t buf_length = base64::get_encoded_length(data_length);
auto buf = std::make_unique<uint8_t[]>(buf_length);

base64::encode(
    data, data_length,
    buf.get(), buf_length
);
```

## Decoding
```cpp
// Encoded data
const uint8_t* data = ...;
size_t data_length = ...;

// Decoded buffer
size_t buf_length = base64::get_decoded_length(data, data_length);
auto buf = std::make_unique<uint8_t[]>(buf_length);

base64::decode(
    data, data_length,
    buf.get(), buf_length
);
```

# Padding
Base64 padding bytes are encoded by default but can be optionally controlled by setting the `padding` parameter on the `get_encoded_length` and `encode` methods.
```cpp
// Padded encoding
size_t padded_length = base64::get_encoded_length(data_length);
base64::encode(data, data_length, buf, padded_length);

// Unpadded encoding
size_t unpadded_length = base64::get_encoded_length(data_length, false);
base64::encode(data, data_length, buf, unpadded_length, false);
```

Decoding will automatically handle padding bytes if they are present but will still function correctly without them.  The `get_decoded_length` method requires the full encoded buffer so padding bytes can be detected.
```cpp
// Padded data
assert(base64::get_decoded_length("YWJjZA==", 8) == 4);

// Unpadded data
assert(base64::get_decoded_length("YWJjZA", 6) == 4);
```

# SSSE3 + AVX2 Codepaths
By default the fastest codepath is chosen at runtime (`AVX2` > `SSSE3` > `Basic`), though this can be overriden by providing a specific codepath to the encode and decode methods.  The `Basic` implementation will work on any architecture but will not be optimal.  If your target architecture supports [AVX2](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions) or [SSSE3](https://en.wikipedia.org/wiki/SSSE3) instructions then an alternative implementation can be used instead.

The alternative implementations are based on work by Wojciech Muła: [encoding](http://0x80.pl/notesen/2016-01-12-sse-base64-encoding.html), [decoding](http://0x80.pl/notesen/2016-01-17-sse-base64-decoding.html).