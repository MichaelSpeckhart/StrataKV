//
// Created by Michael Speckhart on 8/3/26.
//

#ifndef STRATAKV_CONTROL_BYTES_H
#define STRATAKV_CONTROL_BYTES_H
#include <cstdint>
#include <type_traits>

/**
 * NB: Taken from absl @hash_control_bytes.h
 */
enum class ctrl_t : int8_t {
  kEmpty = -128,   // 0b10000000
  kDeleted = -2,   // 0b11111110
  kSentinel = -1,  // 0b11111111
};
static_assert(
    (static_cast<int8_t>(ctrl_t::kEmpty) &
     static_cast<int8_t>(ctrl_t::kDeleted) &
     static_cast<int8_t>(ctrl_t::kSentinel) & 0x80) != 0,
    "Special markers need to have the MSB to make checking for them efficient");
static_assert(
    ctrl_t::kEmpty < ctrl_t::kSentinel && ctrl_t::kDeleted < ctrl_t::kSentinel,
    "ctrl_t::kEmpty and ctrl_t::kDeleted must be smaller than "
    "ctrl_t::kSentinel to make the SIMD test of IsEmptyOrDeleted() efficient");
static_assert(
    ctrl_t::kSentinel == static_cast<ctrl_t>(-1),
    "ctrl_t::kSentinel must be -1 to elide loading it from memory into SIMD "
    "registers (pcmpeqd xmm, xmm)");
static_assert(ctrl_t::kEmpty == static_cast<ctrl_t>(-128),
              "ctrl_t::kEmpty must be -128 to make the SIMD check for its "
              "existence efficient (psignb xmm, xmm)");
static_assert(
    (~static_cast<int8_t>(ctrl_t::kEmpty) &
     ~static_cast<int8_t>(ctrl_t::kDeleted) &
     static_cast<int8_t>(ctrl_t::kSentinel) & 0x7F) != 0,
    "ctrl_t::kEmpty and ctrl_t::kDeleted must share an unset bit that is not "
    "shared by ctrl_t::kSentinel to make the scalar test for "
    "MaskEmptyOrDeleted() efficient");
static_assert(ctrl_t::kDeleted == static_cast<ctrl_t>(-2),
              "ctrl_t::kDeleted must be -2 to make the implementation of "
              "ConvertSpecialToEmptyAndFullToDeleted efficient");
static_assert(ctrl_t::kEmpty == static_cast<ctrl_t>(-128),
              "ctrl_t::kEmpty must be -128 to use saturated subtraction in"
              " ConvertSpecialToEmptyAndFullToDeleted");

// Helpers for checking the state of a control byte.
inline bool IsEmpty(ctrl_t c) { return c == ctrl_t::kEmpty; }
inline bool IsFull(ctrl_t c) {
  // Cast `c` to the underlying type instead of casting `0` to `ctrl_t` as `0`
  // is not a value in the enum. Both ways are equivalent, but this way makes
  // linters happier.
  return static_cast<std::underlying_type_t<ctrl_t>>(c) >= 0;
}
inline bool IsDeleted(ctrl_t c) { return c == ctrl_t::kDeleted; }
inline bool IsEmptyOrDeleted(ctrl_t c) { return c < ctrl_t::kSentinel; }

#endif //STRATAKV_CONTROL_BYTES_H
