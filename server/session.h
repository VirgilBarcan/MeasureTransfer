//
// Created by virgil on 10.03.2019.
//

#ifndef MEASURE_TRANSFER_SESSION_H
#define MEASURE_TRANSFER_SESSION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "communicator.h"

using boost::asio::ip::tcp;

class Session : public boost::enable_shared_from_this<Session>
{
public:
    using Pointer = boost::shared_ptr<Session>;

    static Pointer Create(boost::asio::io_service& io_service)
    {
        static uint16_t client_id = 5000;
        return Pointer(new Session(io_service, client_id++));
    }

    ~Session()
    {
        // print the stats
        auto stats = communicator_->GetStats();
        std::cout << "Protocol: " << stats.protocol << std::endl;
        std::cout << "Communication mechanism: " << stats.communication_mechanism << std::endl;
        std::cout << "# read messages: " << stats.no_of_read_messages << std::endl;
        std::cout << "# read bytes: " << stats.no_of_read_bytes << std::endl;
    }

    void Start()
    {
        std::cout << "Session " << client_id_ << " started" << std::endl;

        do
        {
            // wait hello message from the client
            auto error = HandleHello();

            if (error)
            {
                std::cout << "Session::Start hello error: " << error << std::endl;
                break;
            }

            // wait goodbye message from the client
            error = HandleGoodbye();

            if (error)
            {
                std::cout << "Session::Start goodbye error: " << error << std::endl;
                break;
            }
        } while (false);

        // end the session
        communicator_->Stop();
        std::cout << "Session " << client_id_ << " finished" << std::endl;
    }

    tcp::socket& Socket()
    {
        return socket_;
    }

private:
    Session(boost::asio::io_service& io_service, uint16_t client_id)
        : client_id_(client_id)
        , socket_(io_service)
        , communicator_(nullptr)
    {}

    boost::system::error_code HandleHello()
    {
        boost::system::error_code error;

        // wait hello message from the client
        HelloMessage::Buffer buffer;

        auto read_bytes = socket_.read_some(boost::asio::buffer(buffer), error);

        if (error)
        {
            std::cout << "Read HelloMessage error: " << error << std::endl;
            return error;
        }

        if (read_bytes < HelloMessage::kSize)
        {
            std::cout << "Read HelloMessage of wrong size" << std::endl;
            return make_error_code(boost::system::errc::protocol_error);
        }

        // parse message
        HelloMessage hello_message = HelloMessage::Decode(buffer);

        // build communicator
        communicator_ = CommunicatorFactory(hello_message.protocol, hello_message.communication_mechanism, hello_message.message_size, client_id_);
        boost::thread communication_thread(boost::bind(&Communicator::Start, communicator_));
        communication_thread.detach();

        // send response
        AcknowledgeMessage response_message = { client_id_ };
        auto response_buffer = AcknowledgeMessage::Encode(response_message);

        auto sent_bytes = socket_.send(boost::asio::buffer(response_buffer));
        if (sent_bytes < AcknowledgeMessage::kSize)
        {
            std::cout << "Failed to send AcknowledgeMessage" << std::endl;
            return make_error_code(boost::system::errc::protocol_error);
        }

        return make_error_code(boost::system::errc::success);
    }

    boost::system::error_code HandleGoodbye()
    {
        boost::system::error_code error;

        // wait goodbye message from the client
        GoodbyeMessage::Buffer buffer;

        auto read_bytes = socket_.read_some(boost::asio::buffer(buffer), error);

        if (error)
        {
            std::cout << "Read GoodbyeMessage error: " << error << std::endl;
            return error;
        }

        if (read_bytes < GoodbyeMessage::kSize)
        {
            std::cout << "Read GoodbyeMessage of wrong size" << std::endl;
            return make_error_code(boost::system::errc::protocol_error);
        }

        return make_error_code(boost::system::errc::success);
    }

private:
    uint16_t client_id_;
    tcp::socket socket_;
    std::shared_ptr<Communicator> communicator_;
};

#endif //MEASURE_TRANSFER_SESSION_H
