#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace base64 {

    namespace detail {
        constexpr std::string_view Base64LUT{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
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

    inline size_t get_base64_length(size_t binary_length, bool padded = true) {
        if (padded) {
            return (binary_length + 2) / 3 * 4;
        } else {
            return ((binary_length / 3) * 4) + (binary_length % 3);
        }
    }

    inline size_t get_binary_length(const std::string_view& data) {
        if (data.empty()) {
            return 0;
        }

        size_t octet_count = data.size() / 4;
        size_t remainder = data.size() - (octet_count * 4);
        if (remainder != 0) {
            // Unpadded data
            return octet_count + (remainder - 1);
        }

        // Either binary % 3 == 0 || padded 
        octet_count *= 3;
        if (data[data.size()-2] == '=') {
            return octet_count - 2;
        } else if (data.back() == '=') {
            return octet_count - 1;
        }

        return octet_count;
    }

    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    template <typename TQuadCallback, typename TByteCallback>
    void encode_callbacks(
        const std::string_view& data,
        TQuadCallback quad_callback,
        TByteCallback byte_callback,
        bool padded = true
    ) {
        size_t octet_count = (data.size() / 3);

        size_t octet_end = (octet_count * 3);
        for (size_t i = 0; i < octet_end; i += 3) {
            uint8_t b0 = static_cast<uint8_t>(data[i  ]);
            uint8_t b1 = static_cast<uint8_t>(data[i+1]);
            uint8_t b2 = static_cast<uint8_t>(data[i+2]);

            uint32_t quad = static_cast<uint32_t>(detail::Base64LUT[b0 >> 2]);
            quad |= static_cast<uint32_t>(detail::Base64LUT[(b0 & 0x03) << 4 | b1 >> 4]) << 8;
            quad |= static_cast<uint32_t>(detail::Base64LUT[(b1 & 0x0F) << 2 | b2 >> 6]) << 16;
            quad |= static_cast<uint32_t>(detail::Base64LUT[b2 & 0x3F]) << 24;

            quad_callback(quad);
        }

        size_t remainder = data.size() - octet_end;
        if (remainder == 2) {
            uint8_t b0 = static_cast<uint8_t>(data[octet_end  ]);
            uint8_t b1 = static_cast<uint8_t>(data[octet_end+1]);

            byte_callback(detail::Base64LUT[b0 >> 2]);
            byte_callback(detail::Base64LUT[(b0 & 0x03) << 4 | b1 >> 4]);
            byte_callback(detail::Base64LUT[(b1 & 0x0F) << 2]);
            if (padded) {
                byte_callback('=');
            }

        } else if (remainder == 1) {
            uint8_t b0 = static_cast<uint8_t>(data[octet_end]);
            
            byte_callback(detail::Base64LUT[b0 >> 2]);
            byte_callback(detail::Base64LUT[(b0 & 0x03) << 4]);

            if (padded) {
                byte_callback('=');
                byte_callback('=');
            }
        } 
    }

    //--------------------------------------------------------------------------------------------------------

    inline std::string encode_to_string(const std::string_view& data, bool padded = true) {
        std::string buf;
        buf.reserve(get_base64_length(data.size(), padded));

        encode_callbacks(
            data,
            [&](uint32_t value) {
                buf.append(reinterpret_cast<char*>(&value), 4);
            },
            [&](uint8_t value) {
                buf.append(1, static_cast<char>(value));
            },
            padded
        );

        return buf;
    }

    //--------------------------------------------------------------------------------------------------------

    inline std::vector<uint8_t> encode_to_byte_vector(const std::string_view& data, bool padded = true) {
        std::vector<uint8_t> buf;
        buf.resize(get_base64_length(data.size(), padded));

        auto buf_ptr = buf.data();

        encode_callbacks(
            data,
            [&](uint32_t value) {
                *reinterpret_cast<uint32_t*>(buf_ptr) = value;
                buf_ptr += 4;
            },
            [&](uint8_t value) {
                *buf_ptr++ = value;
            },
            padded
        );

        return buf;
    }

    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    template <typename TTripleCallback, typename TByteCallback>
    void decode(
        const std::string_view& data,
        TTripleCallback triple_callback,
        [[maybe_unused]] TByteCallback byte_callback
    ) {
        size_t binary_length = get_binary_length(data);
        size_t octet_count = binary_length / 3;

        size_t octet_end = (octet_count * 4);
        for (size_t i = 0; i < octet_end; i += 4) {
            uint8_t b0 = detail::Base64InverseLUT[data[i  ]];
            uint8_t b1 = detail::Base64InverseLUT[data[i+1]];
            uint8_t b2 = detail::Base64InverseLUT[data[i+2]];
            uint8_t b3 = detail::Base64InverseLUT[data[i+3]];

            uint8_t c0 = b0 << 2 | b1 >> 4;
            uint8_t c1 = b1 << 4 | b2 >> 2;
            uint8_t c2 = b2 << 6 | b3;

            triple_callback(c0, c1, c2);
        }

        size_t remainder = binary_length - (octet_count * 3);
        if (remainder == 2) {
            uint8_t b0 = detail::Base64InverseLUT[data[octet_end  ]];
            uint8_t b1 = detail::Base64InverseLUT[data[octet_end+1]];
            uint8_t b2 = detail::Base64InverseLUT[data[octet_end+2]];

            byte_callback(b0 << 2 | b1 >> 4);
            byte_callback(b1 << 4 | b2 >> 2);

        } else if (remainder == 1) {
            uint8_t b0 = detail::Base64InverseLUT[data[octet_end  ]];
            uint8_t b1 = detail::Base64InverseLUT[data[octet_end+1]];

            byte_callback(b0 << 2 | b1 >> 4);
        }
    }

    //--------------------------------------------------------------------------------------------------------

    inline std::string decode_to_string(const std::string_view& data) {
        std::string str;
        str.reserve(get_binary_length(data));

        decode(
            data,
            [&](uint8_t b0, uint8_t b1, uint8_t b2) {
                str.append(1, b0);
                str.append(1, b1);
                str.append(1, b2);
            },
            [&](uint8_t b) {
                str.append(1, b);
            }
        );

        return str;
    }

    //--------------------------------------------------------------------------------------------------------

    inline std::vector<uint8_t> decode_to_vector(const std::string_view& data) {
        std::vector<uint8_t> buf;
        buf.resize(get_binary_length(data));

        auto buf_ptr = buf.data();

        decode(
            data,
            [&](uint8_t b0, uint8_t b1, uint8_t b2) {
                buf_ptr[0] = b0;
                buf_ptr[1] = b1;
                buf_ptr[2] = b2;
                buf_ptr += 3;
            },
            [&](uint8_t b) {
                *buf_ptr++ = b;
            }
        );

        return buf;
    }

}