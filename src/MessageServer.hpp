/*********************************************************************************************************************/
/*                                                  /===-_---~~~~~~~~~------____                                     */
/*                                                 |===-~___                _,-'                                     */
/*                  -==\\                         `//~\\   ~~~~`---.___.-~~                                          */
/*              ______-==|                         | |  \\           _-~`                                            */
/*        __--~~~  ,-/-==\\                        | |   `\        ,'                                                */
/*     _-~       /'    |  \\                      / /      \      /                                                  */
/*   .'        /       |   \\                   /' /        \   /'                                                   */
/*  /  ____  /         |    \`\.__/-~~ ~ \ _ _/'  /          \/'                                                     */
/* /-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`                                                      */
/*                   \_|      /        _)   ;  ),   __--~~                                                           */
/*                     '~~--_/      _-~/-  / \   '-~ \                                                               */
/*                    {\__--_/}    / \\_>- )<__\      \                                                              */
/*                    /'   (_/  _-~  | |__>--<__|      |                                                             */
/*                   |0  0 _/) )-~     | |__>--<__|     |                                                            */
/*                   / /~ ,_/       / /__>---<__/      |                                                             */
/*                  o o _//        /-~_>---<__-~      /                                                              */
/*                  (^(~          /~_>---<__-      _-~                                                               */
/*                 ,/|           /__>--<__/     _-~                                                                  */
/*              ,//('(          |__>--<__|     /                  .----_                                             */
/*             ( ( '))          |__>--<__|    |                 /' _---_~\                                           */
/*          `-)) )) (           |__>--<__|    |               /'  /     ~\`\                                         */
/*         ,/,'//( (             \__>--<__\    \            /'  //        ||                                         */
/*       ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'                                          */
/*     `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/                                                     */
/*   ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~                                                       */
/*    ;'( ')/ ,)(                              ~~~~~~~~~~                                                            */
/*   ' ') '( (/                                                                                                      */
/*     '   '  `                                                                                                      */
/*********************************************************************************************************************/
#ifndef _MESSAGE_SERVER_HPP_
#define _MESSAGE_SERVER_HPP_

#include <stdint.h>
#include <memory>
#include <functional>
#include <set>
#include <memory>

#include "WarnGuard.hpp"
WARN_GUARD_ON
#include <asio.hpp>
WARN_GUARD_OFF
#include "Logger.hpp"

/// Sets the function that will be called when the message server receives a notification
void SetNotificationEventCallback(std::function<void(const std::string&, unsigned int)> cb);

///==============================================================
///= ClientConnection
///==============================================================
class ConnectionManager;

class ClientConnection : public std::enable_shared_from_this<ClientConnection>
{
    public:
        /// Constructor
        explicit ClientConnection(asio::ip::tcp::socket sock, ConnectionManager& parentConMan);

        /// Disable copy construction
        ClientConnection(const ClientConnection& rhs) = delete;
        ClientConnection& operator=(const ClientConnection& rhs) = delete;

        /// Prepares current client for send and recv operations
        void Start();

        /// Stop all asynchronous operations associated with the connection.
        void Stop();

        /// Gets, handles the message and returns the responce
        void HandleMessage(std::string);

    private:
        /// Perform an asynchronous read operation.
        void DoRecv();

        /// Perform an asynchronous write operation.
        void DoSend(std::string msg);

        /// The socket that is assosiated with the current connection
        asio::ip::tcp::socket mSocket;

        /// Buffer for incoming data
        std::array<char, 4096> mRvBuf;

        /// Buffer for outgoing data
        std::array<char, 4096> mSdBuf;

        /// The connection manager that holds this connection
        ConnectionManager& mParentConnectionManager;

        /// Stores the ip of the client, used for diagnostic messages
        std::string mIP;
};

typedef std::shared_ptr<ClientConnection> ClientConnectionPtr;


///==============================================================
///= ConnectionManager
///==============================================================

class ConnectionManager
{
    public:
        /// Add the specified connection to the manager and start it.
        void Start(ClientConnectionPtr c);

        /// Stop the specified connection.
        void Stop(ClientConnectionPtr c);

        /// Stop all connections.
        void StopAll();

    private:
        /// The managed connections.
        std::set<ClientConnectionPtr> mConnections;
};


///==============================================================
///= MessageServer
///==============================================================

class MessageServer
{
    public:
        /// Constructor, takes as argument the listen port
        explicit MessageServer(unsigned short port = 7777);

        /// Disable copy construction
        MessageServer(const MessageServer& rhs) = delete;
        MessageServer& operator=(const MessageServer& rhs) = delete;

        /// Starts the operation of the server synchronously
        void Run();

        /// Sets the callback that is called when the server exits (optional)
        void SetExitCallback(std::function<void()> cb);

    private:
        /// Perform an asynchronous accept operation.
        void DoAccept();

        /// Wait for a request to stop the server.
        void DoAwaitStop();

        /// The io_service used to perform asynchronous operations.
        asio::io_service mIOService;

        /// Acceptor used to listen for incoming connections.
        asio::ip::tcp::acceptor mAcceptor;

        /// The signal_set is used to register for process termination notifications.
        asio::signal_set mSignals;

        /// The connection manager which owns all live connections.
        ConnectionManager mConnectionManager;

        /// The next socket to be accepted.
        asio::ip::tcp::socket mAcceptSocket;

        /// The optional exit callback
        std::function<void()> mExitCallback;
};

#endif // ! _MESSAGE_SERVER_HPP_

