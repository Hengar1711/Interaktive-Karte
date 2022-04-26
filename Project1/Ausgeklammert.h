#pragma once

//
//tcp::socket tcp_socket{ ioservice };
//std::array<char, 4096> bytes;

//tcp::endpoint tcp_endpoint{ tcp::v4(), 8000 };
//tcp::acceptor tcp_acceptor{ ioservice };// , tcp_endpoint };
//tcp::socket tcp_socket2{ ioservice };

#ifdef SERVER_DATEN_ABRUF
/* Client seitig */
void read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
	if (!ec)
	{
		std::cout.write(bytes.data(), bytes_transferred);
		tcp_socket.async_read_some(buffer(bytes), read_handler);
	}
}
void connect_handler(const boost::system::error_code &ec)
{
	if (!ec)
	{
		std::string r = "GET / HTTP/1.1\r\nHost: theboostcpplibraries.com\r\n\r\n"; //virtualberater.com
		write(tcp_socket, buffer(r));
		tcp_socket.async_read_some(buffer(bytes), read_handler);
	}
}
void resolve_handler(const boost::system::error_code &ec, tcp::resolver::iterator it)
{
	if (ec)
	{
		cout << "Resolve failed with msg:" << ec.message();
	}
	else
	{
		tcp::endpoint tcp_endpoints = *it;

		tcp_socket.async_connect(*it, connect_handler);
		cout << "Verbunden mit " << tcp_endpoints.address() << ":" << tcp_endpoints.port() << endl << endl;
	}
}
#endif
#ifdef LISTEN_ALLGEMEIN_AUFRUF
/* Server Seitig */
void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
	if (ec)
	{
		//write(tcp_socket2, buffer(test));
		cout << ec.message();
	}
	else
	{
		tcp_socket2.shutdown(tcp::socket::shutdown_send);
	}
}
void accept_handler(const boost::system::error_code &ec)
{
	if (ec)
	{
		cout << " Failed to Accept with Msg: " << ec.message() << endl;
	}
	else
	{
		std::stringstream response;
		response << "HTTP/1.1 200 OK" << HTML_END_COLUM;
		response << HTML_END_COLUM;
		response << "<html>" << HTML_END_COLUM;
		response << "<body>" << HTML_END_COLUM;
		response << "<h1>Hallo Welt </h1>" << HTML_END_COLUM;
		response << "</body>" << HTML_END_COLUM;
		response << "</html>" << HTML_END_COLUM;

		//std::time_t now = std::time(nullptr);
		//data2 = std::ctime(&now);
		//async_write(tcp_socket2, buffer(response.str()), write_handler);
		try {
			//auto temp = boost::asio::buffer(response.str().data(), response.str().size());//
			//string test = "test";
			//ConstBufferSequenz z;
			tcp_socket2.send(boost::asio::buffer(response.str()));
			//tcp_socket2.write_some(temp);
			//write(tcp_socket2, buffer(test));
			//async_write(tcp_socket2, temp, write_handler);
		}
		catch (boost::system::error_code &e)
		{
			cout << e.message();
			;
		}
		cout << response.str();
	}
}
#endif
#ifdef DAY_TIME_LISTEN_ASYNC_SERVER
/* Daytime asynchron */
std::string make_daytime_string()
{
	using namespace std; // For time_t, time and ctime;
	time_t now = time(0);
	return ctime(&now);
}
class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
{
	tcp::socket socket_;
	std::string message_;

	tcp_connection(boost::asio::io_context& io_context) : socket_(io_context)
	{
	}

	void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/)
	{
	}

public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(boost::asio::io_context& io_context)
	{
		return pointer(new tcp_connection(io_context));
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		message_ = make_daytime_string();

		boost::asio::async_write(socket_, boost::asio::buffer(message_), boost::bind(&tcp_connection::handle_write, shared_from_this(),
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
};
class tcp_server
{
	boost::asio::io_context& io_context_;
	tcp::acceptor acceptor_;

	void start_accept()
	{
		tcp_connection::pointer new_connection = tcp_connection::create(io_context_);

		acceptor_.async_accept(new_connection->socket(), boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error));
	}

	void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
	{
		if (!error)
		{
			new_connection->start();
		}

		start_accept();
	}
public:
	tcp_server(boost::asio::io_context& io_context) : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
	{
		start_accept();
	}
};
#endif
#ifdef HEARTHBEAT_SERVER
void write_data(boost::asio::ip::tcp::socket& sock, std::string data)
{
	boost::system::error_code error;
	std::string msg;
	int ch = data[0] - '0';
	switch (ch)
	{
	case 1: msg = "Case 1\n"; break;
	case 2: msg = "Case 2\n"; break;
	case 3: msg = "Case 3\n"; break;
	case 4: msg = "Case 4\n"; break;
	default: msg = "Case default\n"; break;
	}
	boost::asio::write(sock, boost::asio::buffer(msg + "\n"), error);
	if (!error)
	{
		std::cout << "Server sent " << msg << std::endl;
	}
	else
	{
		std::cout << "send failed: " << error.message() << std::endl;
	}
};
std::string read_data(boost::asio::ip::tcp::socket& sock)
{
	boost::asio::streambuf buf;
	boost::asio::read_until(sock, buf, "\n");
	std::string data = boost::asio::buffer_cast<const char*>(buf.data());
	return data;
};
void process(boost::asio::ip::tcp::socket& sock)
{
	while (1)
	{
		std::string data = read_data(sock);
		std::cout << "Client's request is: " << data << std::endl;
		if (data[0] == 'H')
		{
			std::string msg = "HEARTBEAT";
			//data='5';
			boost::asio::write(sock, boost::asio::buffer(msg + '\n'));
			std::cout << "******************HEARTBEAT sent to clinet********************\n";
		}
		else
			write_data(sock, data);
	}
};
#endif
#ifdef HEARTHBEAT_CLIENT
void read_data(boost::asio::ip::tcp::socket &sock)// for reading data
{
	boost::system::error_code error;
	// 	getting response from server
	boost::asio::streambuf receive_buffer;
	//boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
	boost::asio::read_until(sock, receive_buffer, "\n");
	if (error && error != boost::asio::error::eof)
	{
		std::cout << "receive failed: " << error.message() << std::endl;
	}
	else
	{
		const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
		std::cout << "Received " << data << std::endl;
	}
};
void write_data(boost::asio::ip::tcp::socket &sock)
{
	//while(1){
	//Message to sent for server
	std::string msg;
	std::cout << "Enter the message to sent\n";
	//std::cin>>msg;
	std::getline(std::cin, msg); //for getting complete line
	boost::system::error_code error;

	//Write message to socket
	boost::asio::write(sock, boost::asio::buffer(msg + "\n"), error);
	if (!error)
	{
		std::cout << "Client sent " << msg << std::endl;
	}
	else
	{
		std::cout << "send failed: " << error.message() << std::endl;
	}
	//}
};
void IOthread(boost::asio::ip::tcp::socket &sock)
{
	write_data(sock);
	read_data(sock);
}
void sleepThread(boost::asio::ip::tcp::socket & sock)
{
	std::this_thread::sleep_for(std::chrono::seconds(5)); //sleep thread for 5 seconds
	//boost::system::error_code error;
	std::string msg = "HEARTBEAT";
	std::cout << "Trying to write\n";
	boost::asio::write(sock, boost::asio::buffer(msg + '\n'));
	std::cout << "###### Requested for HEARTBEAT#########\n";
	//read_data(sock);
	//std::cout<<"\nReceived "<<str<<std::endl;
	boost::asio::streambuf receive_buffer;
	boost::asio::read_until(sock, receive_buffer, "\n");
	const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
	std::cout << "******************Received   " << data << "  *************  " << std::endl;
	sleepThread(sock); //call recursively 
}
#endif
#ifdef CHAT_SERVER
void write_data(boost::asio::ip::tcp::socket & sock)
{
	boost::system::error_code error;
	std::string msg;
	std::cout << "Enter the message \n";
	//std::cin>>msg;
	std::getline(std::cin, msg); //For getting complete line 
	boost::asio::write(sock, boost::asio::buffer(msg + "\n"), error);
	if (!error)
	{
		std::cout << "Server sent hello message!" << std::endl;
	}
	else
	{
		std::cout << "send failed: " << error.message() << std::endl;
	}
};
std::string read_data(boost::asio::ip::tcp::socket & sock)
{
	boost::asio::streambuf buf;
	boost::asio::read_until(sock, buf, "\n");
	std::string data = boost::asio::buffer_cast<const char*>(buf.data());
	return data;
};
void process(boost::asio::ip::tcp::socket & sock)
{
	while (1)
	{
		std::string data = read_data(sock);
		std::cout << "Client's request is: " << data << std::endl;

		write_data(sock);
	}
};
#endif
#ifdef CHAT_CLIENT
std::string read_data(boost::asio::ip::tcp::socket &sock)
{
	boost::system::error_code error;
	// 	getting response from server
	boost::asio::streambuf receive_buffer;
	//boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
	boost::asio::read_until(sock, receive_buffer, "\n");
	if (error && error != boost::asio::error::eof)
	{
		std::cout << "receive failed: " << error.message() << std::endl;
	}
	else
	{
		const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
		std::cout << "Received " << data << std::endl;
	}

};
void write_data(boost::asio::ip::tcp::socket &sock)
{
	//Message to sent for server
	std::string msg;
	std::cout << "Enter the message to sent\n";
	//std::cin>>msg;
	std::getline(std::cin, msg); //for getting complete line
	boost::system::error_code error;

	//Write message to socket
	boost::asio::write(sock, boost::asio::buffer(msg + "\n"), error);
	if (!error)
	{
		std::cout << "Client sent hello message!" << std::endl;
	}
	else
	{
		std::cout << "send failed: " << error.message() << std::endl;
	}

};
#endif


#ifdef SERVER_DATEN_ABRUF
string adress = "theboostcpplibraries.com";////"https://www.virtualberater.com/";"localhost";
tcp::resolver::query q{ adress, "80" };
resolv.async_resolve(q, resolve_handler);
#endif
#ifdef LISTEN_ALLGEMEIN_AUFRUF
if (!socket_soffen)
{
	tcp::resolver::query q{ "127.0.0.1", "8000" };
	tcp::endpoint tcp_endpoint = *(resolv.resolve(q));

	tcp_acceptor.open(tcp_endpoint.protocol());
	tcp_acceptor.bind(tcp_endpoint);
	tcp_acceptor.listen(socket_base::max_connections);
	tcp_acceptor.async_accept(tcp_socket, accept_handler);
	socket_soffen = true;
}
#endif
#ifdef DAY_TIME_LISTEN_ASYNC_SERVER
tcp_server server(ioservice);
#endif
#ifdef HEARTHBEAT_SERVER
unsigned short port_num = 3333;
boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port_num);
try
{
	boost::asio::ip::tcp::acceptor acceptor(ioservice, ep.protocol());
	acceptor.bind(ep);
	acceptor.listen();
	boost::asio::ip::tcp::socket sock(ioservice);
	acceptor.accept(sock);

	//   std::thread t([&sock]() {
	//       hearbeatSender(sock);
	//   });
	process(sock);
	//  t.join();

		}
catch (boost::system::system_error& e)
{
	std::cout << "Error occured! Error code = " << e.code()
		<< ". Message: " << e.what();

	cout << e.code().value();
}
#endif
#ifdef HEARTHBEAT_CLIENT
unsigned short port_num = 3333;

//socket creation
boost::asio::ip::tcp::socket sock(ioservice);

//connection
boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port_num);

sock.connect(ep);
std::thread t2([&sock]() { sleepThread(sock); });
while (1)
{
	IOthread(sock); //calling from main thread.
}
#endif
#ifdef CHAT_SERVER
//const int BACKLOG_SIZE = 30;

// Step 1. Here we assume that the server application has already obtained the protocol port number.
unsigned short port_num = 3333;
// Step 2. Creating a server endpoint.
boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port_num);
//we need at least one io_service instance. boost::asio uses io_service to talk with operating system's I/O services. 
//while(1){
try
{
	// Step 3. Instantiating and opening an acceptor socket.
	boost::asio::ip::tcp::acceptor acceptor(ioservice, ep.protocol());
	// Step 4. Binding the acceptor socket to the server endpint.
	acceptor.bind(ep);
	// Step 5. Starting to listen for incoming connection requests.
	acceptor.listen();

	// Step 6. Creating an active socket.
	boost::asio::ip::tcp::socket sock(ioservice);
	// Step 7. Processing the next connection request and connecting the active socket to the client.
	acceptor.accept(sock);

	//all steps for creating socket using boost::asio are done.

	//Now perform read write operations in a function.
	//while(1)
	process(sock);

}
catch (boost::system::system_error &e)
{
	std::cout << "Error occured! Error code = " << e.code()
		<< ". Message: " << e.what();

	cout << e.code().value();
}
//}
#endif
#ifdef CHAT_CLIENT
unsigned short port_num = 3333;

//socket creation
boost::asio::ip::tcp::socket socket(ioservice);

//connection
boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port_num);

socket.connect(ep);

while (1)
{

	write_data(socket);
	std::string str = read_data(socket);
	//sleep(2); //wait for 2 second if you need otherwise comment it.
}

#endif
#ifdef DAYTIME_CLIENT
try
{
	if (argc != 2)
	{
		std::cerr << "Usage: client <host>" << std::endl;
		return 1;
	}

	tcp::resolver resolver(ioservice);
	tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "daytime");

	tcp::socket socket(ioservice);
	boost::asio::connect(socket, endpoints);

	for (;;)
	{
		boost::array<char, 128> buf;
		boost::system::error_code error;

		size_t len = socket.read_some(boost::asio::buffer(buf), error);

		if (error == boost::asio::error::eof)
			break; // Connection closed cleanly by peer.
		else if (error)
			throw boost::system::system_error(error); // Some other error.

		std::cout.write(buf.data(), len);
	}
}
#endif