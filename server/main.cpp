//
// Created by virgil on 10.03.2019.
//

#include <iostream>

#include "server.h"

int main()
{
    std::cout << "Hello, Server!" << std::endl;

    try
    {
        boost::asio::io_service io_service;
        Server server(io_service);
        server.Start();
        io_service.run();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}