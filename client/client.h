//
// Created by virgil on 17.03.2019.
//

#ifndef MEASURE_TRANSFER_CLIENT_H
#define MEASURE_TRANSFER_CLIENT_H

#include <chrono>
#include <cstdint>

struct Stats
{
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> end_time;
    uint32_t no_of_sent_messages;
    uint32_t no_of_sent_bytes;
};

class Client
{
public:
    virtual ~Client() = default;
    virtual void TransferData(uint32_t no_of_messages, uint32_t message_size) = 0;
    virtual Stats GetStats() const = 0;
};

using boost::asio::ip::tcp;

class TcpStopAndGoClient : public Client
{
public:
    TcpStopAndGoClient(boost::asio::io_service& io_service, std::string host, uint16_t port)
        : io_service_{io_service}
        , host_{std::move(host)}
        , port_{port}
        , stats_{}
    {}

    void TransferData(uint32_t no_of_messages, uint32_t message_size) override
    {
        boost::system::error_code error;
        tcp::resolver resolver(io_service_);
        tcp::resolver::query query(host_, std::to_string(port_));
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::socket socket(io_service_);
        boost::asio::connect(socket, endpoint_iterator);

        // send data
        std::vector<uint8_t> data(message_size, 0);

        // stats
        stats_.start_time = std::chrono::steady_clock::now();

        for (uint32_t i = 0; i < no_of_messages; ++i)
        {
            // send data message
            DataMessage data_message = { i, data };

            auto sent_bytes = socket.send(boost::asio::buffer(DataMessage::Encode(data_message)));
            stats_.no_of_sent_bytes += sent_bytes;
            if (sent_bytes < DataMessage::kSize)
            {
                std::cout << "Failed to send DataMessage message header" << std::endl;
                continue;
            }

            std::cout << "DataMessage " << data_message.message_no << " header sent" << std::endl;

            sent_bytes = socket.send(boost::asio::buffer(boost::asio::buffer(data_message.data)));
            stats_.no_of_sent_bytes += sent_bytes;
            if (sent_bytes < data_message.data.size())
            {
                std::cout << "Failed to send DataMessage message payload" << std::endl;
                continue;
            }

            // stats
            stats_.no_of_sent_messages++;
            std::cout << "DataMessage " << data_message.message_no << " payload sent" << std::endl;

            // wait ack
            AcknowledgeMessage::Buffer ack_buffer;
            auto read_bytes = socket.read_some(boost::asio::buffer(ack_buffer), error);
            if (error)
            {
                std::cout << "Receive AcknowledgeMessage error: " << error << std::endl;
                continue;
            }

            if (read_bytes < AcknowledgeMessage::kSize)
            {
                std::cout << "Read AcknowledgeMessage of wrong size" << std::endl;
                continue;
            }

            auto ack_message = AcknowledgeMessage::Decode(ack_buffer);
            std::cout << "ACK message received for " << ack_message.message_no << std::endl;
        }

        // stats
        stats_.end_time = std::chrono::steady_clock::now();

        // disconnect
        socket.close();
    }

    Stats GetStats() const override
    {
        return stats_;
    }

private:
    boost::asio::io_service& io_service_;
    std::string host_;
    uint16_t port_;
    Stats stats_;
};

std::unique_ptr<Client> ClientFactory(Protocol protocol, CommunicationMechanism communication_mechanism, boost::asio::io_service& io_service, std::string host, uint16_t port)
{
    std::unique_ptr<Client> client = nullptr;

    switch (protocol)
    {
        case Protocol::kTcp:
        {
            switch (communication_mechanism)
            {
                case CommunicationMechanism::kStopAndGo:
                {
                    client = std::make_unique<TcpStopAndGoClient>(io_service, host, port);
                    break;
                }
                case CommunicationMechanism::kStreaming:
                {
//                    client = std::make_unique<TcpStreamingClient>(io_service, host, port);
                    break;
                }
                default:
                {
                    std::cerr << communication_mechanism << std::endl;
                    break;
                }
            }
            break;
        }
        case Protocol::kUdp:
        {
            switch (communication_mechanism)
            {
                case CommunicationMechanism::kStopAndGo:
                {
//                    client = std::make_unique<UdpStopAndGoClient>(io_service, host, port);
                    break;
                }
                case CommunicationMechanism::kStreaming:
                {
//                    client = std::make_unique<UdpStreamingClient>(io_service, host, port);
                    break;
                }
                default:
                {
                    std::cerr << communication_mechanism << std::endl;
                    break;
                }
            }
        }
        default:
        {
            std::cerr << protocol << std::endl;
            break;
        }
    }

    return client;
}

#endif //MEASURE_TRANSFER_CLIENT_H
