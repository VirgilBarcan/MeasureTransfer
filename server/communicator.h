//
// Created by virgil on 10.03.2019.
//

#ifndef MEASURE_TRANSFER_COMMUNICATOR_H
#define MEASURE_TRANSFER_COMMUNICATOR_H

#include "messages.h"

using boost::asio::ip::tcp;

class Communicator
{
public:
    Communicator(Protocol protocol, CommunicationMechanism communication_mechanism, std::size_t message_size, uint16_t client_id)
        : protocol_(protocol)
        , communication_mechanism_(communication_mechanism)
        , message_size_(message_size)
        , io_service_()
        , acceptor_(io_service_, tcp::endpoint(tcp::v4(), client_id))
        , socket_(io_service_)
    {}

    void Run()
    {
        try
        {
            // create new socket for the data transfer
            Start();
            io_service_.run();
        }
        catch (std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void Stop()
    {
        io_service_.stop();
    }

private:
    void Start()
    {
        acceptor_.async_accept(socket_,
                               boost::bind(&Communicator::OnAccept, this, boost::asio::placeholders::error));
    }

    void OnAccept(const boost::system::error_code& error)
    {
        if (error)
        {
            std::cout << "Communicator::OnAccept error: " << error << std::endl;
            return;
        }

        Communicate();
    }

    boost::system::error_code Communicate()
    {
        // read message
        DataMessage data_message;
        auto error = ReadDataMessage(data_message);
        if (error)
        {
            std::cout << "Communicate error on reading DataMessage: " << error << std::endl;
            return error;
        }

        // process data message
        // TODO: put the bytes from the payload to a file

        // send response message
        AcknowledgeMessage ack_message = { data_message.message_no };
        error = SendAckMessage(ack_message);
        if (error)
        {
            std::cout << "Communicate error on sending AcknowledgeMessage: " << error << std::endl;
            return error;
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

private:
    Protocol protocol_;
    CommunicationMechanism communication_mechanism_;
    std::size_t message_size_;

    boost::asio::io_service io_service_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

#endif //MEASURE_TRANSFER_COMMUNICATOR_H
