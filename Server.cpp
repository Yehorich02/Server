#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/enable_shared_from_this.hpp>


/// <summary>
/// After starting the server, you can connect several clients to it, but after starting the server, 
/// it can process two client requests, 
/// that is, either connecting two users, or connecting one user and receiving a message from him.
/// </summary>

class con_handler : public boost::enable_shared_from_this<con_handler>
{
private:
    boost::asio::ip::tcp::socket socket_;
    std::string message = "Successfully connected!\r\n";
    enum { max_length = 1024 };
    char data[max_length];
    bool started_ = false;
    bool sended_ = false;

public:

    typedef boost::shared_ptr<con_handler> pointer;
    con_handler(boost::asio::io_service& io_service): socket_(io_service){}

    static pointer create(boost::asio::io_service& io_service)
    {
        return pointer(new con_handler(io_service));
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        if (!started_)
        {
            do_write();
            started_ = true;
        }
        do_read();
     }


    //function for sending messages from a client
    void do_write()
    {
        auto handler = boost::bind(&con_handler::handle_write, shared_from_this(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
        socket_.async_write_some(boost::asio::buffer(message, max_length), handler);
    }

    void handle_write(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (error)
        {
            std::cerr << "An error oqurred(handle_write): " << error.message() << std::endl;
            socket_.close();
        }
    }
    //function for reading messages from a client
    void do_read()
    {
        
        socket_.async_read_some(
            boost::asio::buffer(data, max_length),
            boost::bind(&con_handler::handle_read,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
 
    void handle_read(boost::system::error_code const& error, size_t bytes_received)
    {
        if (bytes_received > 0)
        {
            std::cout << "Message received from IP: " << socket_.remote_endpoint().address()
                << ", with data: " << data;
            sended_ = true;
            socket_.close();
        }

        if (error)
        {
            std::cerr << "An error oqurred: " << error.message() << std::endl;
            socket_.close();
        }
    }
};

class Server
{
private:
    boost::asio::io_service & io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    bool port_ = false;
    bool started_ = false;
    uint16_t port_s;

    // asynchronous accept operation
    void start_accept()
    {
        con_handler::pointer connection = con_handler::create(io_service_);

        acceptor_.async_accept(connection->socket(),
        boost::bind(&Server::handle_accept, this, connection,
        boost::asio::placeholders::error));
        if (!started_)
        {
            std::cout << "Server started on " << port_s << std::endl;
            started_ = true;
        }
        io_service_.run_one();
    }
public:
        //constructor for accepting connection
        Server(boost::asio::io_service& io_service) : io_service_(io_service),
        acceptor_(io_service_){}

        void HelpBox()
        {
            std::cout << "- help\n"
                << "       This help message\n"
                << "- exit\n"
                << "       Quit the session\n"
                << "- start\n"
                << "       Start server\n"
                << "- stop\n"
                << "       Stop server\n"
                << "- SetPort <int>\n"
                << "        Set port or server\n"
                << std::endl;
        }

        //function to set the port for the server
        void SetPort(uint16_t port)
        {
            auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port);
            acceptor_.open(endpoint.protocol());
            acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            acceptor_.bind(endpoint);
            acceptor_.listen();

            std::cout << "Server will be on port: " << port << std::endl;

            port_ = true;
            port_s = port;
        }

        void start()
        {
            start_accept();
        }

        void handle_accept(con_handler::pointer connection, const boost::system::error_code& error)
        {
            if ((!error) && port_ == true)
            {
                
                connection->start();
            }
            else
            {
                std::cerr << "An error oqurred: " << error.message() << std::endl;
                connection->socket().close();
            }
            start_accept();
        }
};

int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_service io_service;
        Server srv(io_service);
        
        std::string command;
        int port;
        bool port_ = false;

        while (true)
        {
            std::cout << "Server> ";
            std::cin >> command;
            if (command == "SetPort")
            {
                std::cin >> port;
                srv.SetPort(port);
                port_ = true;
            }
            else if (command == "start" && port_==true)
            {
                srv.start();
                io_service.run_one();
            }
            else if (command == "help")
            {
                srv.HelpBox();
            }
            else if (command == "stop")
            {
                io_service.stop();
                std::cout << "Server stoped on port: " << port << std::endl;
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
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

