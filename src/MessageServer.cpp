#include "MessageServer.hpp"
#include <sstream>
#include "NotificationWindow.hpp"

// A simple logger
static Logger<ConsoleAppender, SimpleFormatter> CLogger;

// The notification callback holder
static std::function<void(const std::string&, unsigned int)> notificationCallback;

void SetNotificationEventCallback(std::function<void(const std::string&, unsigned int)> cb)
{
    notificationCallback = cb;
}

///==============================================================
///= ClientConnection
///==============================================================

ClientConnection::ClientConnection(asio::ip::tcp::socket socket, ConnectionManager& parentConMan) 
	: mSocket(std::move(socket)),
	  mParentConnectionManager(parentConMan)
{
    std::fill(std::begin(mRvBuf), std::end(mRvBuf), 0);
}

void ClientConnection::Start()
{
	mIP = mSocket.remote_endpoint().address().to_string();
	DoRecv();
}

void ClientConnection::Stop()
{
	mSocket.close();
}

void ClientConnection::DoRecv()
{
	auto self(shared_from_this());
	mSocket.async_read_some(asio::buffer(mRvBuf),
		[this, self](const asio::error_code& ec, std::size_t bytes_transferred)
		{
			if (!mSocket.is_open())
				return;
	
			if ((ec == asio::error::eof) || (ec == asio::error::connection_reset))
			{
				// Client disconnect
				CLogger.Info("Client with ip " + mIP + " has disconnected.");
				mParentConnectionManager.Stop(shared_from_this());
			}
			else
			{
				CLogger.Info("Received " + std::to_string(bytes_transferred) + " bytes of data from client with ip " + mIP);
				
				// Do staph with the data
				std::string request(&mRvBuf[0]);
				
				// Clear the old buffer
				mRvBuf.fill(0);

				// Handle the request
				HandleMessage(std::move(request));

				// Rescedule receive operation
				DoRecv();
			}
		}
	);
}

void ClientConnection::DoSend(std::string message)
{
	std::copy(message.begin(), message.end(), mSdBuf.begin());
	auto self(shared_from_this());
	mSocket.async_write_some(asio::buffer(mSdBuf, message.size()),
		[this, self](asio::error_code ec, std::size_t sent)
		{
			(void) ec;
			if (sent != 0)
				CLogger.Info("Sent " + std::to_string(sent) + " bytes of data to the IP: " + mIP);
		}
	);
}

void ClientConnection::HandleMessage(std::string msg)
{
    notificationCallback(msg, 3000);

	// Send back the responce
	DoSend(msg);
}

///==============================================================
///= ConnectionManager
///==============================================================

void ConnectionManager::Start(ClientConnectionPtr c)
{
	mConnections.insert(c);
	c->Start();
}

void ConnectionManager::Stop(ClientConnectionPtr c)
{
	mConnections.erase(c);
	c->Stop();
}

void ConnectionManager::StopAll()
{
	size_t aliveConnections = mConnections.size();
	for (auto& c : mConnections)
		c->Stop();
	mConnections.clear();
	CLogger.Info("Terminated " + std::to_string(aliveConnections) + " alive connections.");
}

///==============================================================
///= MessageServer
///==============================================================

MessageServer::MessageServer(unsigned short port /* = 7777 */)
	 : mAcceptor(mIOService),
	   mSignals(mIOService),
	   mAcceptSocket(mIOService)
{
    // Register to handle the signals that indicate when the server should exit.
	mSignals.add(SIGINT);
	mSignals.add(SIGTERM);

	// Create async operation that listens for interrupt signals
	DoAwaitStop();

    // Open the acceptor with the option to reuse the address
    asio::ip::tcp::endpoint endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
	mAcceptor.open(endpoint.protocol());
	mAcceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
	mAcceptor.bind(endpoint);
	mAcceptor.listen();
    
	// Create the async accept operation
	DoAccept();
}

void MessageServer::Run()
{
	CLogger.Info("Server is starting...");

	// The io_service::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
	mIOService.run();
}

void MessageServer::SetExitCallback(std::function<void()> cb)
{
    mExitCallback = cb;
}

void MessageServer::DoAccept()
{
	CLogger.Info("Preparing interface to for accept...");
	// Schedules an asyncronous accept passing the callback and storing the newly created client context as a future argument
	mAcceptor.async_accept(mAcceptSocket, 
		[this](const asio::error_code& ec)
		{
			// Check whether the server was stopped by a signal before this
			// completion handler had a chance to run.
			if (!mAcceptor.is_open())
				return;

			std::string incomingIp = mAcceptSocket.remote_endpoint().address().to_string();
			CLogger.Info("Accepted connection from " + incomingIp);

			if (!ec)
				mConnectionManager.Start(
                        std::make_shared<ClientConnection>(
                            std::move(mAcceptSocket), mConnectionManager));

			// Rechedule next accept operation
			DoAccept();
		}
	);
}

void MessageServer::DoAwaitStop()
{
	mSignals.async_wait(
		[this](const asio::error_code& ec, int sig)
		{
			// Unused parameter
			(void) ec;

			// The server is stopped by cancelling all outstanding asynchronous
			// operations. Once all operations have finished the io_service::run()
			// call will exit.
			CLogger.Info("Received signal " + std::to_string(sig) + "!");
			CLogger.Info("Stopping Accept interface...");
			mAcceptor.close();

			CLogger.Info("Terminating all current connections...");
			mConnectionManager.StopAll();

			CLogger.Info("Server is shuting down...");
            if (mExitCallback)
                mExitCallback();
		}
	);
}

