//
// Created by virgil on 10.03.2019.
//

#ifndef MEASURE_TRANSFER_SERVER_H
#define MEASURE_TRANSFER_SERVER_H

#include "session.h"

using boost::asio::ip::tcp;

class Server
{
public:
    Server(boost::asio::io_service& io_service)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), 4991))
    {
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    }

    void Start()
    {
        // accept a new client
        Accept();
    }

private:
    void Accept()
    {
        // create the new session
        Session::Pointer new_session = Session::Create(acceptor_.get_io_service());

        // wait for the new client to connect
        acceptor_.async_accept(new_session->Socket(),
                               boost::bind(&Server::OnAccept, this, new_session, boost::asio::placeholders::error));
    }

    void OnAccept(Session::Pointer new_session, const boost::system::error_code& error)
    {
        if (error)
        {
            std::cout << "Server::OnAccept error: " << error << std::endl;
            return;
        }

        // communicate with the client
        new_session->Start();

        // accept another client
        Accept();
    }

private:
    tcp::acceptor acceptor_;
};

#endif //MEASURE_TRANSFER_SERVER_H
