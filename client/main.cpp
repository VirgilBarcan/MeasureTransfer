//
// Created by virgil on 10.03.2019.
//

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <messages.h>
#include <thread>
#include <utility>

#include "client.h"

int main(int argc, char* argv[])
{
    std::unique_ptr<Client> client = nullptr;

    try
    {
        std::string host = "localhost";
        Protocol protocol = Protocol::kTcp;
        CommunicationMechanism communication_mechanism = CommunicationMechanism::kStreaming;
        uint32_t no_of_messages = 10;
        uint32_t message_size = 1024;

        if (argc == 6)
        {
            host = argv[1];
            protocol = static_cast<Protocol>(std::stoul(argv[2]));
            communication_mechanism = static_cast<CommunicationMechanism>(std::stoul(argv[3]));
            no_of_messages = static_cast<uint32_t>(std::stoul(argv[4]));
            message_size = static_cast<uint32_t>(std::stoul(argv[5]));
        }
        else
        {
            std::cerr << "Usage: client <host> <protocol: 0 - TCP; 1 - UDP> <communication mechanism: 0 - StopAndGo; 1 - Streaming> <no of messages> <message size>" << std::endl;
        }

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(host, "4991");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        // send Hello message
        HelloMessage hello_message = { protocol, communication_mechanism, message_size };
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


        // open new connection
        client = ClientFactory(protocol, communication_mechanism, io_service, host, static_cast<uint16_t>(response_message.message_no));
        client->TransferData(no_of_messages, message_size);

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

    auto stats = client->GetStats();

    // print stats
    auto transmission_time = std::chrono::duration_cast<std::chrono::milliseconds>(stats.end_time - stats.start_time);
    std::cout << "Transmission time: " << transmission_time.count() << " ms" << std::endl;
    std::cout << "# sent messages: " << stats.no_of_sent_messages << std::endl;
    std::cout << "# sent bytes: " << stats.no_of_sent_bytes << std::endl;

    return 0;
}