#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace base64 {

    namespace detail {
        // Static look-up table for 6-bit values to 8-bit base64 characters.  All values are valid.
        constexpr std::string_view Base64LUT{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };

        // Static look-up table for 8-bit base64 characters to 6-bit values.  Invalid values are forced to
        // zero (i.e. no validation).
        constexpr std::array<uint8_t, 256> Base64InverseLUT = {
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x00 - 0x0F
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x10 - 0x1F
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63, // 0x20 - 0x2F
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0, // 0x30 - 0x3F
             0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 0x40 - 0x4F
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0, // 0x50 - 0x5F
             0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 0x60 - 0x6F
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0, // 0x70 - 0x7F
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x80 - 0x8F
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x90 - 0x9F
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xA0 - 0xAF
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xB0 - 0xBF
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xC0 - 0xCF
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xD0 - 0xDF
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xE0 - 0xEF
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  // 0xF0 - 0xFF
        };
    }

    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    // Helper to determine the size of an encoded base64 buffer.
    inline size_t get_base64_length(size_t binary_length, bool padded = true) {
        if (padded) {
            return (binary_length + 2) / 3 * 4;
        } else {
            size_t remainder = binary_length % 3;
            size_t length = (binary_length / 3) * 4;
            if (remainder) {
                length += remainder + 1;
            }
            return length;
        }
    }

    // Helper to determine the size of a decoded binary buffer, given the source base64 data.
    inline size_t get_binary_length(const uint8_t* data, const size_t data_length) {
        if (data_length == 0) {
            return 0;
        }

        size_t octet_count = data_length / 4;
        size_t remainder = data_length % 4;
        if (remainder != 0) {
            // Unpadded data
            return (octet_count * 3) + (remainder - 1);
        }

        // Either binary % 3 == 0 || padded 
        octet_count *= 3;
        if (data[data_length-2] == '=') {
            return octet_count - 2;
        } else if (data[data_length-1] == '=') {
            return octet_count - 1;
        }

        return octet_count;
    }

    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    // Primary base64 encoding method.  Asserts that the destination buffer is _exactly_ the required size.
    inline void encode(
        const uint8_t* source_data,
        const size_t source_data_length,
        uint8_t* dest_data,
        const size_t dest_data_length,
        bool padded = true
    ) {
        if (get_base64_length(source_data_length, padded) != dest_data_length) {
            throw std::logic_error("Dest buffer is incorrect size");
        }

        size_t octet_count = (source_data_length / 3);
        size_t octet_end = (octet_count * 3);
        auto dest_ptr = dest_data;

        // Process three source values at a time.
        for (size_t i = 0; i < octet_end; i += 3, dest_ptr += 4) {
            char b0 = source_data[i  ];
            char b1 = source_data[i+1];
            char b2 = source_data[i+2];

			dest_ptr[0] = detail::Base64LUT[b0 >> 2];
			dest_ptr[1] = detail::Base64LUT[(b0 & 0x03) << 4 | b1 >> 4];
			dest_ptr[2] = detail::Base64LUT[(b1 & 0x0F) << 2 | b2 >> 6];
			dest_ptr[3] = detail::Base64LUT[b2 & 0x3F];
        }

        // Handle the remaining values separately to avoid branches the main loop.
        size_t remainder = source_data_length - octet_end;
        if (remainder == 2) {
            uint8_t b0 = source_data[octet_end  ];
            uint8_t b1 = source_data[octet_end+1];

            dest_ptr[0] = detail::Base64LUT[b0 >> 2];
            dest_ptr[1] = detail::Base64LUT[(b0 & 0x03) << 4 | b1 >> 4];
            dest_ptr[2] = detail::Base64LUT[(b1 & 0x0F) << 2];
            if (padded) {
                dest_ptr[3] = '=';
            }

        } else if (remainder == 1) {
            uint8_t b0 = source_data[octet_end];
            
            dest_ptr[0] = detail::Base64LUT[b0 >> 2];
            dest_ptr[1] = detail::Base64LUT[(b0 & 0x03) << 4];

            if (padded) {
                dest_ptr[2] = '=';
                dest_ptr[3] = '=';
            }
        } 
    }

    //--------------------------------------------------------------------------------------------------------

    // Helper to encode directly to a std::string.
    inline std::string encode_to_string(
        const uint8_t* source_data,
        const size_t source_data_length,
        bool padded = true
    ) {
        std::string str(
			get_base64_length(source_data_length, padded),
			'='
		);

        encode(
            source_data,
            source_data_length,
            reinterpret_cast<uint8_t*>(str.data()),
            str.size(),
            padded
        );

        return str;
    }

    //--------------------------------------------------------------------------------------------------------

    // Helper to encode directly to a std::vector.  This is slightly faster than std::string as it doesn't
    // need to initialize the buffer before encoding.
    inline std::vector<uint8_t> encode_to_byte_vector(
        const uint8_t* source_data,
        const size_t source_data_length,
        bool padded = true
    ) {
        std::vector<uint8_t> buf;
        buf.resize(get_base64_length(source_data_length, padded));

        encode(
            source_data,
            source_data_length,
            buf.data(),
            buf.size(),
            padded
        );

        return buf;
    }

    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    // Primary base64 decoding method.  Asserts that the destination buffer is _exactly_ the required size.
    inline void decode(
        const uint8_t* source_data,
        const size_t source_data_length,
        uint8_t* dest_data,
        const size_t dest_data_length
    ) {
        size_t binary_length = get_binary_length(source_data, source_data_length);
        if (binary_length != dest_data_length) {
            throw std::logic_error("Dest buffer is incorrect size");
        }

        size_t octet_count = binary_length / 3;
        size_t octet_end = (octet_count * 4);
        auto dest_ptr = dest_data;

        // Process four source values at a time.
        for (size_t i = 0; i < octet_end; i += 4, dest_ptr += 3) {
            uint8_t b0 = detail::Base64InverseLUT[source_data[i  ]];
            uint8_t b1 = detail::Base64InverseLUT[source_data[i+1]];
            uint8_t b2 = detail::Base64InverseLUT[source_data[i+2]];
            uint8_t b3 = detail::Base64InverseLUT[source_data[i+3]];

            dest_ptr[0] = b0 << 2 | b1 >> 4;
            dest_ptr[1] = b1 << 4 | b2 >> 2;
            dest_ptr[2] = b2 << 6 | b3;
        }

        // Handle the remaining values separately to avoid branches the main loop.
        size_t remainder = binary_length - (octet_count * 3);
        if (remainder == 2) {
            uint8_t b0 = detail::Base64InverseLUT[source_data[octet_end  ]];
            uint8_t b1 = detail::Base64InverseLUT[source_data[octet_end+1]];
            uint8_t b2 = detail::Base64InverseLUT[source_data[octet_end+2]];

            dest_ptr[0] = b0 << 2 | b1 >> 4;
            dest_ptr[1] = b1 << 4 | b2 >> 2;

        } else if (remainder == 1) {
            uint8_t b0 = detail::Base64InverseLUT[source_data[octet_end  ]];
            uint8_t b1 = detail::Base64InverseLUT[source_data[octet_end+1]];

            dest_ptr[0] = b0 << 2 | b1 >> 4;
        }
    }

    //--------------------------------------------------------------------------------------------------------

    // Helper to decode directly to a std::string.
    inline std::string decode_to_string(
        const uint8_t* source_data,
        const size_t source_data_length
    ) {
        std::string str(get_binary_length(source_data, source_data_length), '\0');

        decode(
            source_data,
            source_data_length,
            reinterpret_cast<uint8_t*>(str.data()),
            str.size()
        );

        return str;
    }

    //--------------------------------------------------------------------------------------------------------

    // Helper to decode directly to a std::vector.  This is slightly faster than std::string as it doesn't
    // need to initialize the buffer before decoding.
    inline std::vector<uint8_t> decode_to_vector(
        const uint8_t* source_data,
        const size_t source_data_length
    ) {
        std::vector<uint8_t> buf;
        buf.resize(get_binary_length(source_data, source_data_length));

        decode(
            source_data,
            source_data_length,
            buf.data(),
            buf.size()
        );

        return buf;
    }

}