#include <boost/asio.hpp>
#include <iostream>
#include <string>

bool connected = false;

void HelpBox()
{
    std::cout << "- help\n"
        << "       This help message\n"
        << "- exit\n"
        << "       Quit the session\n"
        << "- connect <string>\n"
        << "       Connect to server\n"
        << "- send <string>\n"
        << "       Send message\n"
        << std::endl;
}

//function for processing messages from the server
std::string do_read(boost::asio::ip::tcp::socket& socket)
{
    boost::asio::streambuf buf;
    boost::asio::read_until(socket, buf, "\n");
    std::string data = boost::asio::buffer_cast<const char*>(buf.data());
    return data;
}

//function to send a message to the server
void do_write(boost::asio::ip::tcp::socket& socket, const std::string& message)
{
    boost::asio::write(socket, boost::asio::buffer(message + "\n"));
    std::cout << "Message sent to " << socket.remote_endpoint().address() << ":" << socket.remote_endpoint().port() << ", with data: " << message<< std::endl;
}

//function to connect to server
void ConnectToServer(boost::asio::ip::tcp::socket& socket, std::string& ip_address, uint16_t& port, boost::system::error_code const& error)
{
    socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(ip_address), port));
    if (!error)
    {
        std::cout << "Connect to " << ip_address << ":" << port << std::endl;
        std::cout << do_read(socket);
        connected = true;
    }
    else
    {
        std::cerr << "An error oqurred: " << error.message() << std::endl;
    }
}



int main(int argc, char* argv[])
{
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket_(io_service);
    boost::system::error_code error;
    std::string command;
    std::string ip; //client ip-address
    uint16_t port; //the port number on which the server is running
    char value;
    std::string name = "Client";
    std::string str;
    while (true) {
        std::cout << name << "> ";
        std::cin >> command;
        if (command == "help")
        {
            HelpBox();
        }
        else if (command == "connect" && !connected)
        {
            do
            {
                std::cin >> value;
                ip.push_back(value);
            } while (value != ':');

            ip.pop_back();
            std::cin >> port;

            ConnectToServer(socket_, ip, port, error);
        }
        else if (command == "send" && connected)
        {
            std::cin.ignore(1,' ');
            std::getline(std::cin, str);
            do_write(socket_, str);
            
        }
        else if (command == "exit")
        {
            return 0;
        }
        else
        {
            std::cerr << "If you need help enter - help\r\n" << std::endl;
        }

        command.erase();
    }

}