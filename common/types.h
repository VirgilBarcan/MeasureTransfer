//
// Created by virgil on 10.03.2019.
//

#ifndef MEASURE_TRANSFER_TYPES_H
#define MEASURE_TRANSFER_TYPES_H

#include <cstdint>

const std::size_t kProtocolSize = 1;
const std::size_t kCommunicationMechanismSize = 1;

enum class Protocol : int8_t
{
    kTcp = 0,
    kUdp = 1
};

enum class CommunicationMechanism : int8_t
{
    kStreaming = 0,
    kStopAndGo = 1
};

#endif //MEASURE_TRANSFER_TYPES_H
