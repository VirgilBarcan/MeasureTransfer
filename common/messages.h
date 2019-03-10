//
// Created by virgil on 10.03.2019.
//

#ifndef MEASURE_TRANSFER_MESSAGES_H
#define MEASURE_TRANSFER_MESSAGES_H

#include <boost/array.hpp>

#include "types.h"
#include "utils.h"

const std::size_t kMaxMessageSize = 65535;

const std::size_t kMessageTagSize = 1;
const std::size_t kMessageNoSize = 4;

/**
 * Messages have the following format:
 * Format: | MessageTag | MessageData |
 * Index:  |     0      |      1      |
 * Size:   |   1byte    |    Kbytes   |, k in [0, kMaxMessageSize]
 */

enum class MessageTag : int8_t
{
    kHelloMessage = 0,
    kGoodbyeMessage = 1,
    kDataMessage = 2,
    kAcknowledgeMessage = 3
};

/**
 * Hello Message format:
 * Format: | MessageTag | Protocol | CommunicationMechanism | MessageSize |
 * Index:  |     0      |    1     |           2            |     3       |
 * Size:   |   1byte    |  1byte   |         1byte          |   4byte     |
 */
struct HelloMessage
{
    static const std::size_t kSize = kMessageTagSize + kProtocolSize + kCommunicationMechanismSize + sizeof(std::size_t);
    using Buffer = boost::array<uint8_t, kSize>;

    static HelloMessage Decode(const Buffer& buffer)
    {
        if (buffer[0] != static_cast<uint8_t>(MessageTag::kHelloMessage))
        {
            throw std::invalid_argument("Buffer doesn't contain a HelloMessage");
        }

        return { static_cast<Protocol>(buffer[1]),
                 static_cast<CommunicationMechanism>(buffer[2]),
                 FromBytes(&buffer[3]) };
    }

    static Buffer Encode(const HelloMessage& message)
    {
        Buffer buffer;
        buffer[0] = static_cast<uint8_t>(MessageTag::kHelloMessage);
        buffer[1] = static_cast<uint8_t>(message.protocol);
        buffer[2] = static_cast<uint8_t>(message.communication_mechanism);

        buffer[3] = static_cast<uint8_t>((message.message_size >> 24) & 0xFF);
        buffer[4] = static_cast<uint8_t>((message.message_size >> 16) & 0xFF);
        buffer[5] = static_cast<uint8_t>((message.message_size >> 8) & 0xFF);
        buffer[6] = static_cast<uint8_t>(message.message_size & 0xFF);

        return buffer;
    }

    // data members
    Protocol protocol;
    CommunicationMechanism communication_mechanism;
    std::size_t message_size;
};

/**
 * Goodbye Messages format:
 * Format: | MessageTag | MessageData |
 * Index:  |     0      |      1      |
 * Size:   |   1byte    |    Kbytes   |
 */
struct GoodbyeMessage
{
    static const std::size_t kSize = kMessageTagSize;
    using Buffer = boost::array<uint8_t, kSize>;

    static GoodbyeMessage Decode(const Buffer& buffer)
    {
        if (buffer[0] != static_cast<uint8_t>(MessageTag::kGoodbyeMessage))
        {
            throw std::invalid_argument("Buffer doesn't contain a GoodbyeMessage");
        }

        return {};
    }

    static Buffer Encode(const GoodbyeMessage& message)
    {
        Buffer buffer;
        buffer[0] = static_cast<uint8_t>(MessageTag::kGoodbyeMessage);
        return buffer;
    }
};

/**
 * Data Messages format:
 * Format: | MessageTag | MessageNo |  Data  |
 * Index:  |     0      |     1     |   5    |
 * Size:   |   1byte    |   4bytes  | Nbytes |
 */
struct DataMessage
{
    static const std::size_t kSize = kMessageTagSize + kMessageNoSize;
    using Buffer = boost::array<uint8_t, kSize>;

    static DataMessage Decode(const Buffer& buffer)
    {
        if (buffer[0] != static_cast<uint8_t>(MessageTag::kDataMessage))
        {
            throw std::invalid_argument("Buffer doesn't contain a DataMessage");
        }

        return { FromBytes(&buffer[1]),
                 std::vector<uint8_t>{} };
    }

    static Buffer Encode(const DataMessage& message)
    {
        Buffer buffer;
        buffer[0] = static_cast<uint8_t>(MessageTag::kDataMessage);
        buffer[1] = static_cast<uint8_t>((message.message_no >> 24) & 0xFF);
        buffer[2] = static_cast<uint8_t>((message.message_no >> 16) & 0xFF);
        buffer[3] = static_cast<uint8_t>((message.message_no >> 8) & 0xFF);
        buffer[4] = static_cast<uint8_t>(message.message_no & 0xFF);

        return buffer;
    }

    // data members
    uint32_t message_no;
    std::vector<uint8_t> data;
};

/**
 * Acknowledge Messages format:
 * Format: | MessageTag | MessageNo |
 * Index:  |     0      |     1     |
 * Size:   |   1byte    |   4bytes  |
 */
struct AcknowledgeMessage
{
    static const std::size_t kSize = kMessageTagSize + kMessageNoSize;
    using Buffer = boost::array<uint8_t, kSize>;

    static AcknowledgeMessage Decode(const Buffer& buffer)
    {
        if (buffer[0] != static_cast<uint8_t>(MessageTag::kAcknowledgeMessage))
        {
            throw std::invalid_argument("Buffer doesn't contain an AcknowledgeMessage");
        }

        return { FromBytes(&buffer[1]) };
    }

    static Buffer Encode(const AcknowledgeMessage& message)
    {
        Buffer buffer;
        buffer[0] = static_cast<uint8_t>(MessageTag::kAcknowledgeMessage);
        buffer[1] = static_cast<uint8_t>((message.message_no >> 24) & 0xFF);
        buffer[2] = static_cast<uint8_t>((message.message_no >> 16) & 0xFF);
        buffer[3] = static_cast<uint8_t>((message.message_no >> 8) & 0xFF);
        buffer[4] = static_cast<uint8_t>(message.message_no & 0xFF);

        return buffer;
    }

    // data members
    uint32_t message_no;
};

#endif //MEASURE_TRANSFER_MESSAGES_H
