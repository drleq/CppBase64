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
            uint32_t octet =
                (static_cast<uint32_t>(data[i  ]) << 16) |
                (static_cast<uint32_t>(data[i+1]) << 8) |
                (static_cast<uint32_t>(data[i+2]));

            uint32_t quad = detail::Base64Values[(octet      ) & 0x3F] << 24;
            quad |= detail::Base64Values[(octet >>= 6) & 0x3F] << 16;
            quad |= detail::Base64Values[(octet >>= 6) & 0x3F] << 8;
            quad |= detail::Base64Values[(octet >>= 6)];

            quad_callback(quad);
        }

        size_t remainder = data.size() - octet_end;
        if (remainder == 2) {
            uint8_t b0 = static_cast<uint8_t>(data[octet_end]);
            uint8_t b1 = static_cast<uint8_t>(data[octet_end+1]);

            byte_callback(detail::Base64Values[b0 & 0x3F]);
            byte_callback(detail::Base64Values[(b0 >> 6) | (b1 & 0x0F) << 2]);
            byte_callback(detail::Base64Values[b1 >> 2]);
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
        size_t octet_count = (data.size() / 4);

        size_t octet_end = (octet_count * 4);
        for (size_t i = 0; i < octet_end; i += 4) {
            uint8_t b0 = static_cast<uint8_t>(data[i  ]      | data[i+1] << 6);
            uint8_t b1 = static_cast<uint8_t>(data[i+1] >> 2 | data[i+2] << 4);
            uint8_t b2 = static_cast<uint8_t>(data[i+2] >> 4 | data[i+3] << 2);

            triple_callback(b0, b1, b2);
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