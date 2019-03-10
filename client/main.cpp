//
// Created by virgil on 10.03.2019.
//

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <messages.h>
#include <thread>

using boost::asio::ip::tcp;

void TransferData(boost::asio::io_service& io_service, const std::string& host, uint16_t port)
{
    boost::system::error_code error;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, std::to_string(port));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    // send data
    std::vector<uint8_t> data(4, 0);

    for (uint32_t i = 0; i < 1; ++i)
    {
        // send data message
        DataMessage data_message = { i, data };

        auto sent_bytes = socket.send(boost::asio::buffer(DataMessage::Encode(data_message)));
        if (sent_bytes < DataMessage::kSize)
        {
            std::cout << "Failed to send DataMessage message header" << std::endl;
            return;
        }

        std::cout << "DataMessage " << data_message.message_no << " header sent" << std::endl;

        sent_bytes = socket.send(boost::asio::buffer(boost::asio::buffer(data_message.data)));
        if (sent_bytes < data_message.data.size())
        {
            std::cout << "Failed to send DataMessage message payload" << std::endl;
            return;
        }

        std::cout << "DataMessage " << data_message.message_no << " payload sent" << std::endl;

        // wait ack
        AcknowledgeMessage::Buffer ack_buffer;
        auto read_bytes = socket.read_some(boost::asio::buffer(ack_buffer), error);
        if (error)
        {
            std::cout << "Receive AcknowledgeMessage error: " << error << std::endl;
            return;
        }

        if (read_bytes < AcknowledgeMessage::kSize)
        {
            std::cout << "Read AcknowledgeMessage of wrong size" << std::endl;
            return;
        }

        auto ack_message = AcknowledgeMessage::Decode(ack_buffer);
        std::cout << "ACK message received for " << ack_message.message_no << std::endl;
    }

    // disconnect
    socket.close();
}

int main(int argc, char* argv[])
{
    try
    {
        std::string host = "localhost";

        if (argc == 2)
        {
            host = argv[1];
        }
        else
        {
            std::cerr << "Usage: client <host>" << std::endl;
        }

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(host, "4991");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        // send Hello message
        HelloMessage hello_message = { Protocol::kTcp, CommunicationMechanism::kStopAndGo, 4 };
        HelloMessage::Buffer buf = HelloMessage::Encode(hello_message);
        boost::system::error_code error;

        auto sent_bytes = socket.send(boost::asio::buffer(buf));
        if (sent_bytes != HelloMessage::kSize)
        {
            std::cout << "Failed to send Hello message" << std::endl;
            return -1;
        }

        std::cout << "Hello Message sent" << std::endl;

        // wait Response message
        AcknowledgeMessage::Buffer response_message_buffer;
        auto read_bytes = socket.read_some(boost::asio::buffer(response_message_buffer), error);
        if (error)
        {
            std::cout << "Receive ResponseMessage error: " << error << std::endl;
            return -2;
        }

        if (read_bytes < AcknowledgeMessage::kSize)
        {
            std::cout << "Read ResponseMessage of wrong size" << std::endl;
            return -3;
        }

        auto response_message = AcknowledgeMessage::Decode(response_message_buffer);
        std::cout << "Response message received, new port = " << response_message.message_no << std::endl;


        std::this_thread::sleep_for(std::chrono::seconds(10));

        // open new connection
        TransferData(io_service, host, response_message.message_no);

        // send Goodbye message
        GoodbyeMessage goodbye_message = {};
        GoodbyeMessage::Buffer buffer = GoodbyeMessage::Encode(goodbye_message);

        sent_bytes = socket.send(boost::asio::buffer(buffer));
        if (sent_bytes != GoodbyeMessage::kSize)
        {
            std::cout << "Failed to send Goodbye message" << std::endl;
            return -1;
        }

        std::cout << "Goodbye Message sent" << std::endl;

        // disconnect
        socket.close();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}