#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

#ifdef BASE64_USE_SSSE3
#include <tmmintrin.h>
#endif

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

#ifdef BASE64_USE_SSSE3
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

        size_t loop_count = (source_data_length / 12);
        size_t loop_end = (loop_count * 12);
        auto dest_ptr = dest_data;

        // Code based on work by Wojciech Muła
        // Ref: http://0x80.pl/notesen/2016-01-12-sse-base64-encoding.html
        const __m128i preshuffle_128 = _mm_set_epi8(10, 11, 9, 10, 7, 8, 6, 7, 4, 5, 3, 4, 1, 2, 0, 1);
        const __m128i t0Mask   = _mm_set_epi32(0x0fc0fc00, 0x0fc0fc00, 0x0fc0fc00, 0x0fc0fc00);
        const __m128i t1Values = _mm_set_epi32(0x04000040, 0x04000040, 0x04000040, 0x04000040);
        const __m128i t2Mask   = _mm_set_epi32(0x003f03f0, 0x003f03f0, 0x003f03f0, 0x003f03f0);
        const __m128i t3Values = _mm_set_epi32(0x01000010, 0x01000010, 0x01000010, 0x01000010);
        const __m128i _51_128  = _mm_set_epi32(0x33333333, 0x33333333, 0x33333333, 0x33333333);
        const __m128i _26_128  = _mm_set_epi32(0x1a1a1a1a, 0x1a1a1a1a, 0x1a1a1a1a, 0x1a1a1a1a);
        const __m128i _13_128  = _mm_set_epi32(0x0d0d0d0d, 0x0d0d0d0d, 0x0d0d0d0d, 0x0d0d0d0d);
        const __m128i shiftLUT = _mm_setr_epi8(
            'a' - 26, '0' - 52, '0' - 52, '0' - 52, '0' - 52, '0' - 52,
            '0' - 52, '0' - 52, '0' - 52, '0' - 52, '0' - 52, '+' - 62,
            '/' - 63, 'A', 0, 0
        );

        for (size_t i = 0; i < loop_end; i += 12, dest_ptr += 16) {
            // Load four sets of octets at once.
            // [????|dddc|ccbb|baaa]
            __m128i b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&source_data[i]));
            b = _mm_shuffle_epi8(b, preshuffle_128);

            // t0 = [0000cccc|CC000000|aaaaaa00|00000000]
            // t1 = [00000000|00cccccc|00000000|00aaaaaa]
            // t2 = [00000000|00dddddd|000000bb|bbbb0000]
            // t3 = [00dddddd|00000000|00bbbbbb|00000000]
            // unpacked = [00dddddd|00cccccc|00bbbbbb|00aaaaaa]
            const __m128i t0 = _mm_and_si128(b, t0Mask);
            const __m128i t2 = _mm_and_si128(b, t2Mask);
            const __m128i t1 = _mm_mulhi_epu16(t0, t1Values);
            const __m128i t3 = _mm_mullo_epi16(t2, t3Values);
            const __m128i unpacked = _mm_or_si128(t1, t3);

            // Convert to base64 characters without lookup tables
            const __m128i reduced = _mm_or_si128(
                _mm_subs_epu8(unpacked, _51_128),
                _mm_and_si128(
                    _mm_cmpgt_epi8(_26_128, unpacked),
                    _13_128
                )
            );
            const __m128i result = _mm_add_epi8(
                _mm_shuffle_epi8(shiftLUT, reduced),
                unpacked
            );

            // Output
            _mm_storeu_si128(
                reinterpret_cast<__m128i*>(dest_ptr),
                result
            );
        }

        size_t remainder = source_data_length - loop_end;
        size_t octet_count = (remainder / 3);
        size_t octet_end = loop_end + (octet_count * 3);

        // Process three source values at a time.
        for (size_t i = loop_end; i < octet_end; i += 3, dest_ptr += 4) {
            char b0 = source_data[i  ];
            char b1 = source_data[i+1];
            char b2 = source_data[i+2];

            dest_ptr[0] = detail::Base64LUT[b0 >> 2];
            dest_ptr[1] = detail::Base64LUT[(b0 & 0x03) << 4 | b1 >> 4];
            dest_ptr[2] = detail::Base64LUT[(b1 & 0x0F) << 2 | b2 >> 6];
            dest_ptr[3] = detail::Base64LUT[b2 & 0x3F];
        }

        // Handle the remaining values separately to avoid branches the main loop.
        remainder = source_data_length - octet_end;
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
#else
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
#endif

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

#ifdef BASE64_USE_SSSE3
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

        size_t loop_count = (source_data_length / 16);
        size_t loop_end = (loop_count * 16);
        auto dest_ptr = dest_data;

        // Code based on work by Wojciech Muła
        // Ref: http://0x80.pl/notesen/2016-01-17-sse-base64-decoding.html
        const __m128i _0f_128 = _mm_set1_epi8(0x0f);
        const __m128i _2f_128 = _mm_set1_epi8(0x2f);
        const __m128i _n3_128 = _mm_set1_epi8(-3);
        const __m128i lower_bound_LUT = _mm_setr_epi8(1, 1, 0x2b, 0x30, 0x41, 0x50, 0x61, 0x70, 1, 1, 1, 1, 1, 1, 1, 1);
        const __m128i upper_bound_LUT = _mm_setr_epi8(0, 0, 0x2b, 0x39, 0x4f, 0x5a, 0x6f, 0x7a, 0, 0, 0, 0, 0, 0, 0, 0);
        const __m128i shiftLUT = _mm_setr_epi8(
            /* 0 */ 0x00,        /* 1 */ 0x00,        /* 2 */ 0x3e - 0x2b, /* 3 */ 0x34 - 0x30,
            /* 4 */ 0x00 - 0x41, /* 5 */ 0x0f - 0x50, /* 6 */ 0x1a - 0x61, /* 7 */ 0x29 - 0x70,
            /* 8 */ 0x00,        /* 9 */ 0x00,        /* a */ 0x00,        /* b */ 0x00,
            /* c */ 0x00,        /* d */ 0x00,        /* e */ 0x00,        /* f */ 0x00
        );
        const __m128i packValues1 = _mm_set_epi32(0x01400140, 0x01400140, 0x01400140, 0x01400140);
        const __m128i packValues2 = _mm_set_epi32(0x00011000, 0x00011000, 0x00011000, 0x00011000);
        const __m128i unshuffle_128 = _mm_setr_epi8(2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, -1, -1, -1, -1);
        const __m128i write_mask_128 = _mm_set_epi32(0x00000000, 0xffffffff, 0xffffffff, 0xffffffff);

        for (size_t i = 0; i < loop_end; i += 16, dest_ptr += 12) {
            // Load four sets of octets at once.
            __m128i b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&source_data[i]));

            // Base64 characters -> 6-bit unpacked
            const __m128i higher_nibble = _mm_and_si128(_mm_srli_epi32(b, 4), _0f_128);
            const __m128i upper_bound = _mm_shuffle_epi8(upper_bound_LUT, higher_nibble);
            const __m128i lower_bound = _mm_shuffle_epi8(lower_bound_LUT, higher_nibble);

            const __m128i below = _mm_cmplt_epi8(b, lower_bound);
            const __m128i above = _mm_cmpgt_epi8(b, upper_bound);
            const __m128i eq_2f = _mm_cmpeq_epi8(b, _2f_128);

            const __m128i shift  = _mm_shuffle_epi8(shiftLUT, higher_nibble);
            const __m128i t0     = _mm_add_epi8(b, shift);
            const __m128i unpacked = _mm_add_epi8(t0, _mm_and_si128(eq_2f, _n3_128));

            // 6-bit unpacked -> 8-bit packed
            const __m128i packed = _mm_madd_epi16(
                _mm_maddubs_epi16(unpacked, packValues1),
                packValues2
            );

            // 8-bit packed -> original order
            const __m128i unshuffled = _mm_shuffle_epi8(packed, unshuffle_128);

			// Output
            _mm_maskmoveu_si128(
				unshuffled,
                write_mask_128,
                reinterpret_cast<char*>(dest_ptr)
            );
        }

        size_t binary_remainder = binary_length - (loop_count * 12);
        size_t octet_count = binary_remainder / 3;
        size_t octet_end = loop_end + (octet_count * 4);

        // Process four source values at a time.
        for (size_t i = loop_end; i < octet_end; i += 4, dest_ptr += 3) {
            uint8_t b0 = detail::Base64InverseLUT[source_data[i  ]];
            uint8_t b1 = detail::Base64InverseLUT[source_data[i+1]];
            uint8_t b2 = detail::Base64InverseLUT[source_data[i+2]];
            uint8_t b3 = detail::Base64InverseLUT[source_data[i+3]];

            dest_ptr[0] = b0 << 2 | b1 >> 4;
            dest_ptr[1] = b1 << 4 | b2 >> 2;
            dest_ptr[2] = b2 << 6 | b3;
        }

        // Handle the remaining values separately to avoid branches the main loop.
		binary_remainder -= (octet_count * 3);
        if (binary_remainder == 2) {
            uint8_t b0 = detail::Base64InverseLUT[source_data[octet_end  ]];
            uint8_t b1 = detail::Base64InverseLUT[source_data[octet_end+1]];
            uint8_t b2 = detail::Base64InverseLUT[source_data[octet_end+2]];

            dest_ptr[0] = b0 << 2 | b1 >> 4;
            dest_ptr[1] = b1 << 4 | b2 >> 2;

        } else if (binary_remainder == 1) {
            uint8_t b0 = detail::Base64InverseLUT[source_data[octet_end  ]];
            uint8_t b1 = detail::Base64InverseLUT[source_data[octet_end+1]];

            dest_ptr[0] = b0 << 2 | b1 >> 4;
        }
    }
#else
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
#endif

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