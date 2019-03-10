//
// Created by virgil on 10.03.2019.
//

#ifndef MEASURE_TRANSFER_UTILS_H
#define MEASURE_TRANSFER_UTILS_H

#include <cstdint>

uint32_t FromBytes(const uint8_t* bytes)
{
    uint32_t value = (bytes[0] << 24);
    value |= (bytes[1] << 16);
    value |= (bytes[2] << 8);
    value |= bytes[3];

    return value;
}

#endif //MEASURE_TRANSFER_UTILS_H
