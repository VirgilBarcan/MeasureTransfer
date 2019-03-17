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

std::ostream& operator<<(std::ostream& os, const Protocol& protocol)
{
    if (protocol == Protocol::kTcp)
    {
        os << "TCP";
        return os;
    }

    if (protocol == Protocol::kUdp)
    {
        os << "UDP";
        return os;
    }

    os << "Unknown Protocol";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CommunicationMechanism& communication_mechanism)
{
    if (communication_mechanism == CommunicationMechanism::kStreaming)
    {
        os << "Streaming";
        return os;
    }

    if (communication_mechanism == CommunicationMechanism::kStopAndGo)
    {
        os << "StopAndGo";
        return os;
    }

    os << "Unknown CommunicationMechanism";
    return os;
}

#endif //MEASURE_TRANSFER_TYPES_H
