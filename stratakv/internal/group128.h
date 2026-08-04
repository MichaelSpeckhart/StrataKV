//
// Created by Michael Speckhart on 8/3/26.
//


#ifndef STRATAKV_GROUP128_H
#define STRATAKV_GROUP128_H
#include <cstddef>
#include <cstdint>

#include "control_bytes.h"
#include "hardware.h"

namespace stratakv {
    template <typename Key = uint64_t, typename Value = uint64_t>
    struct alignas(CompileTimeCacheLine) Group128 {
        static constexpr std::size_t GROUP_WIDTH = 16;
        static constexpr std::size_t SLOTS_PER_GROUP = 7;
        static constexpr uint16_t valid_lanes = 0x7F;
        ctrl_t ctrl[GROUP_WIDTH]{};

        struct Slot {
            Key key;
            Value value;
        } slots[SLOTS_PER_GROUP];

        // Takes a h2 and checks if the
         [[nodiscard]] uint16_t match(ctrl_t h2) const {
            uint16_t mask = 0;
            for (std::size_t i = 0; i < SLOTS_PER_GROUP; ++i)
                if (ctrl[i] == h2) {
                    mask |= static_cast<uint16_t>(1) << i;
                    std::cout << "Mask: " << mask << "\n";
                }
            return mask & valid_lanes;
        }
        [[nodiscard]] uint16_t match_empty() const {
            uint16_t mask = 0;
            for (std::size_t i = 0; i < SLOTS_PER_GROUP; ++i)
                if (IsEmpty(ctrl[i])) mask |= static_cast<uint16_t>(1) << i;
            return mask & valid_lanes;
        }
        [[nodiscard]] uint16_t match_empty_or_deleted() const {
             uint16_t mask = 0;
             for (std::size_t i = 0; i < SLOTS_PER_GROUP; ++i)
                 if (IsEmpty(ctrl[i]) || IsDeleted(ctrl[i])) mask |= static_cast<uint16_t>(1) << i;
             return mask & valid_lanes;
         }
    };

    static_assert(sizeof(Group128<uint64_t, uint64_t>) == 128,
                  "Group must be exactly 128 bytes to fill one Apple Silicon cache line!");
    static_assert(alignof(Group128<uint64_t, uint64_t>) == 128,
                  "Group must be aligned to 128 bytes!");
}



#endif //STRATAKV_GROUP128_H
