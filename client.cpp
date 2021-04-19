#include <boost/asio.hpp>
#include <iostream>
#include <string>

class Client
{
private:
    boost::asio::io_service service_;
    boost::asio::ip::tcp::socket socket_;
    boost::system::error_code error;
    boost::asio::streambuf buf;


public:
    Client() : socket_(service_) {};

    void ConnectToServer(std::string ip_address, uint16_t port)
    {
        socket_.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(ip_address), port));
        if (!error)
        {
            std::cout << "Connect to " << ip_address << ":" << port << std::endl;
            std::cout << ReadFunc();
        }
        else
        {
            std::cerr << "An error oqurred: " << error.message() << std::endl;
        }
    }

    std::string ReadFunc()
    {
        boost::asio::read_until(socket_, buf, "\n");
        std::string data = boost::asio::buffer_cast<const char*>(buf.data());
        return data;
    }

    void WriteFunc(const std::string& msg)
    {
        boost::asio::write(socket_, boost::asio::buffer(msg + "\n"));
        if (!error)
        {
            std::cout << "Message sent to " << socket_.remote_endpoint().address() << ":"
                << socket_.remote_endpoint().port() << ", with data: " << msg << std::endl;
        }
        else
        {
            std::cerr << "An error oqurred: " << error.message() << std::endl;
        }
    }
};

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

void CommandFunc(Client& client)
{
    std::string command;
    char value;
    std::string ip;
    uint16_t port;
    std::string msg;
    bool connected = false;

    while (true)
    {
        std::cout << "Client> ";
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

            client.ConnectToServer(ip, port);
            connected = 1;

        }
        else if (command == "send" && connected)
        {
            std::cin.ignore(1, ' ');
            std::getline(std::cin, msg);
            client.WriteFunc(msg);

        }
        else if (command == "exit")
        {
            return;
        }
        else
        {
            std::cerr << "If you need help enter - help" << std::endl;
            std::getline(std::cin, command);
        }
        command.erase();
    }
}

int main(int argc, char* argv[])
{
    try
    {
        Client cl;
        CommandFunc(cl);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}