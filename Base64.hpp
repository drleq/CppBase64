#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace base64 {

    namespace detail {
        constexpr std::string_view Base64Values{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
    }

    //--------------------------------------------------------------------------------------------------------

    inline size_t get_base64_length(size_t binary_length, bool padded = true) {
        if (padded) {
            return (binary_length + 2) / 3 * 4;
        } else {
            return ((binary_length / 3) * 4) + (binary_length % 3);
        }
    }

    inline size_t get_max_binary_length(size_t base64_length) {
        return (base64_length * 3) / 4;
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
            uint32_t octet =
                (static_cast<uint32_t>(data[i  ]) << 16) |
                (static_cast<uint32_t>(data[i+1]) << 8) |
                (static_cast<uint32_t>(data[i+2]));

            uint32_t quad =
                detail::Base64Values[(octet      ) & 0x3F] << 24 |
                detail::Base64Values[(octet >>= 6) & 0x3F] << 16 |
                detail::Base64Values[(octet >>= 6) & 0x3F] << 8 |
                detail::Base64Values[(octet >>= 6)];

            quad_callback(quad);
        }

        size_t remainder = data.size() - octet_end;
        if (remainder == 2) {
            uint8_t b0 = static_cast<uint8_t>(data[octet_end]);
            uint8_t b1 = static_cast<uint8_t>(data[octet_end+1]);

            byte_callback(detail::Base64Values[b0 & 0x3F]);
            byte_callback(detail::Base64Values[(b0 >> 6) | (b1 & 0x0F) << 4]);
            byte_callback(detail::Base64Values[b1 >> 4]);
            if (padded) {
                byte_callback('=');
            }

        } else if (remainder == 1) {
            uint8_t b0 = static_cast<uint8_t>(data[octet_end]);
            
            byte_callback(detail::Base64Values[b0 && 0x3F]);
            byte_callback(detail::Base64Values[b0 >> 6]);

            if (padded) {
                byte_callback('=');
                byte_callback('=');
            }
        } 
    }

    //--------------------------------------------------------------------------------------------------------

    std::string encode_to_string(const std::string_view& data, bool padded = true) {
        std::string buf;
        buf.reserve(get_base64_length(data.size(), padded));

        encode_callbacks(
            data,
            [&](uint32_t value) {
                buf.append(reinterpret_cast<char*>(value), 4);
            },
            [&](uint8_t value) {
                buf.append(1, static_cast<char>(value));
            },
            padded
        );

        return buf;
    }

    //--------------------------------------------------------------------------------------------------------

    std::vector<uint8_t> encode_to_byte_vector(const std::string_view& data, bool padded = true) {
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

    void decode(const std::string_view& data) {

    }

}