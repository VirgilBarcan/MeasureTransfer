//
// Created by virgil on 10.03.2019.
//

#ifndef MEASURE_TRANSFER_COMMUNICATOR_H
#define MEASURE_TRANSFER_COMMUNICATOR_H

#include "messages.h"

struct Stats
{
    Protocol protocol;
    CommunicationMechanism communication_mechanism;
    uint32_t no_of_read_messages;
    uint32_t no_of_read_bytes;
};

class Communicator
{
public:
    virtual ~Communicator() = default;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual Stats GetStats() const = 0;
};

using boost::asio::ip::tcp;

class TcpStopAndGoCommunicator : public Communicator
{
public:
    TcpStopAndGoCommunicator(std::size_t message_size, uint16_t client_id)
        : protocol_{Protocol::kTcp}
        , communication_mechanism_{CommunicationMechanism::kStopAndGo}
        , message_size_{message_size}
        , io_service_{}
        , acceptor_{io_service_, tcp::endpoint(tcp::v4(), client_id)}
        , socket_{io_service_}
        , stats_{protocol_, communication_mechanism_, 0, 0}
    {}

    void Start() override
    {
        try
        {
            // create new socket for the data transfer
            Run();
            io_service_.run();
        }
        catch (std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void Stop() override
    {
        std::cout << "TcpStopAndGoCommunicator::Stop" << std::endl;
        io_service_.stop();
    }

    Stats GetStats() const override
    {
        return stats_;
    }

private:
    void Run()
    {
        std::cout << "TcpStopAndGoCommunicator::Start" << std::endl;
        acceptor_.async_accept(socket_,
                               boost::bind(&TcpStopAndGoCommunicator::OnAccept, this, boost::asio::placeholders::error));
    }

    void OnAccept(const boost::system::error_code& error)
    {
        if (error)
        {
            std::cout << "TcpStopAndGoCommunicator::OnAccept error: " << error << std::endl;
            return;
        }

        Communicate();
    }

    boost::system::error_code Communicate()
    {
        while (!io_service_.stopped())
        {
            // read message
            DataMessage data_message;
            auto error = ReadDataMessage(data_message);
            if (error)
            {
                std::cout << "Communicate error on reading DataMessage: " << error << std::endl;
                return error;
            }

            std::cout << "Read DataMessage " << data_message.message_no << std::endl;

            // process data message
            // TODO: put the bytes from the payload to a file

            UpdateStats();

            // send response message
            AcknowledgeMessage ack_message = { data_message.message_no };
            error = SendAckMessage(ack_message);
            if (error)
            {
                std::cout << "Communicate error on sending AcknowledgeMessage: " << error << std::endl;
                return error;
            }

            std::cout << "Sent ACK for DataMessage " << ack_message.message_no << std::endl;
        }

        return make_error_code(boost::system::errc::success);
    }

    boost::system::error_code ReadDataMessage(DataMessage& data_message)
    {
        boost::system::error_code error;

        // wait data message
        DataMessage::Buffer data_message_buffer;

        auto read_bytes = socket_.read_some(boost::asio::buffer(data_message_buffer), error);

        if (error)
        {
            std::cout << "Read DataMessage header error: " << error << std::endl;
            return error;
        }

        if (read_bytes < DataMessage::kSize)
        {
            std::cout << "Read DataMessage header of wrong size" << std::endl;
            return make_error_code(boost::system::errc::protocol_error);
        }

        // decode data message header
        data_message = DataMessage::Decode(data_message_buffer);

        // read the payload
        data_message.data.resize(message_size_);
        read_bytes = socket_.read_some(boost::asio::buffer(data_message.data), error);

        if (error)
        {
            std::cout << "Read DataMessage payload error: " << error << std::endl;
            return error;
        }

        if (read_bytes < message_size_)
        {
            std::cout << "Read DataMessage payload of wrong size" << std::endl;
            return make_error_code(boost::system::errc::protocol_error);
        }

        return make_error_code(boost::system::errc::success);
    }

    boost::system::error_code SendAckMessage(AcknowledgeMessage ack_message)
    {
        // send the ACK
        auto sent_bytes = socket_.send(boost::asio::buffer(AcknowledgeMessage::Encode(ack_message)));

        if (sent_bytes < AcknowledgeMessage::kSize)
        {
            std::cout << "Failed to send AcknowledgeMessage" << std::endl;
            return make_error_code(boost::system::errc::protocol_error);
        }

        return make_error_code(boost::system::errc::success);
    }

    void UpdateStats()
    {
        stats_.no_of_read_messages++;
        stats_.no_of_read_bytes += DataMessage::kSize + message_size_;
    }

private:
    Protocol protocol_;
    CommunicationMechanism communication_mechanism_;
    std::size_t message_size_;

    boost::asio::io_service io_service_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;

    Stats stats_;
};

class TcpStreamingCommunicator : public Communicator
{
public:
    TcpStreamingCommunicator(std::size_t message_size, uint16_t client_id)
        : protocol_{Protocol::kTcp}
        , communication_mechanism_{CommunicationMechanism::kStreaming}
        , message_size_{message_size}
        , io_service_{}
        , acceptor_{io_service_, tcp::endpoint(tcp::v4(), client_id)}
        , socket_{io_service_}
        , stats_{protocol_, communication_mechanism_, 0, 0}
    {}

    void Start() override
    {
        try
        {
            // create new socket for the data transfer
            Run();
            io_service_.run();
        }
        catch (std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void Stop() override
    {
        std::cout << "TcpStreamingCommunicator::Stop" << std::endl;
        io_service_.stop();
    }

    Stats GetStats() const override
    {
        return stats_;
    }

private:
    void Run()
    {
        std::cout << "TcpStreamingCommunicator::Start" << std::endl;
        acceptor_.async_accept(socket_,
                               boost::bind(&TcpStreamingCommunicator::OnAccept, this, boost::asio::placeholders::error));
    }

    void OnAccept(const boost::system::error_code& error)
    {
        if (error)
        {
            std::cout << "TcpStreamingCommunicator::OnAccept error: " << error << std::endl;
            return;
        }

        Communicate();
    }

    boost::system::error_code Communicate()
    {
        while (!io_service_.stopped())
        {
            // read message
            DataMessage data_message;
            auto error = ReadDataMessage(data_message);
            if (error)
            {
                std::cout << "Communicate error on reading DataMessage: " << error << std::endl;
                return error;
            }

            std::cout << "Read DataMessage " << data_message.message_no << std::endl;

            // process data message
            // TODO: put the bytes from the payload to a file

            UpdateStats();

            // send response message
            AcknowledgeMessage ack_message = { data_message.message_no };
            error = SendAckMessage(ack_message);
            if (error)
            {
                std::cout << "Communicate error on sending AcknowledgeMessage: " << error << std::endl;
                return error;
            }

            std::cout << "Sent ACK for DataMessage " << ack_message.message_no << std::endl;
        }

        return make_error_code(boost::system::errc::success);
    }

    boost::system::error_code ReadDataMessage(DataMessage& data_message)
    {
        boost::system::error_code error;

        // wait data message
        DataMessage::Buffer data_message_buffer;

        auto read_bytes = socket_.read_some(boost::asio::buffer(data_message_buffer), error);

        if (error)
        {
            std::cout << "Read DataMessage header error: " << error << std::endl;
            return error;
        }

        if (read_bytes < DataMessage::kSize)
        {
            std::cout << "Read DataMessage header of wrong size" << std::endl;
            return make_error_code(boost::system::errc::protocol_error);
        }

        // decode data message header
        data_message = DataMessage::Decode(data_message_buffer);

        // read the payload
        data_message.data.resize(message_size_);
        read_bytes = socket_.read_some(boost::asio::buffer(data_message.data), error);

        if (error)
        {
            std::cout << "Read DataMessage payload error: " << error << std::endl;
            return error;
        }

        if (read_bytes < message_size_)
        {
            std::cout << "Read DataMessage payload of wrong size" << std::endl;
            return make_error_code(boost::system::errc::protocol_error);
        }

        return make_error_code(boost::system::errc::success);
    }

    boost::system::error_code SendAckMessage(AcknowledgeMessage ack_message)
    {
        // send the ACK
        auto sent_bytes = socket_.send(boost::asio::buffer(AcknowledgeMessage::Encode(ack_message)));

        if (sent_bytes < AcknowledgeMessage::kSize)
        {
            std::cout << "Failed to send AcknowledgeMessage" << std::endl;
            return make_error_code(boost::system::errc::protocol_error);
        }

        return make_error_code(boost::system::errc::success);
    }

    void UpdateStats()
    {
        stats_.no_of_read_messages++;
        stats_.no_of_read_bytes += DataMessage::kSize + message_size_;
    }

private:
    Protocol protocol_;
    CommunicationMechanism communication_mechanism_;
    std::size_t message_size_;

    boost::asio::io_service io_service_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;

    Stats stats_;
};


std::unique_ptr<Communicator> CommunicatorFactory(Protocol protocol, CommunicationMechanism communication_mechanism, std::size_t message_size, uint16_t client_id)
{
    std::unique_ptr<Communicator> communicator = nullptr;

    switch (protocol)
    {
        case Protocol::kTcp:
        {
            switch (communication_mechanism)
            {
                case CommunicationMechanism::kStopAndGo:
                {
                    communicator = std::make_unique<TcpStopAndGoCommunicator>(message_size, client_id);
                    break;
                }
                case CommunicationMechanism::kStreaming:
                {
                    communicator = std::make_unique<TcpStreamingCommunicator>(message_size, client_id);
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
//                    communicator = std::make_unique<UdpStopAndGoCommunicator>(message_size, client_id);
                    break;
                }
                case CommunicationMechanism::kStreaming:
                {
//                    communicator = std::make_unique<UdpStreamingCommunicator>(message_size, client_id);
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

    return communicator;
}

#endif //MEASURE_TRANSFER_COMMUNICATOR_H
