/* Copyright (c) 2012 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <gtest/gtest.h>

#include "Core/Debug.h"
#include "RPC/Server.h"
#include "RPC/ServerRPC.h"
#include "RPC/Service.h"

namespace LogCabin {
namespace RPC {
namespace {

class MyService : public Service {
    MyService()
        : lastRPC()
    {
    }
    void handleRPC(ServerRPC serverRPC) {
        lastRPC.reset(new ServerRPC(std::move(serverRPC)));
    }
    std::unique_ptr<ServerRPC> lastRPC;
};

class RPCServerTest : public ::testing::Test {
    RPCServerTest()
        : loop()
        , address("127.0.0.1", 61023)
        , service()
        , server(loop, 1024, service)
        , fd1(-1)
        , fd2(-1)
    {
        EXPECT_EQ("", server.bind(address));
        int fds[2];
        EXPECT_EQ(0, pipe(fds));
        fd1 = fds[0];
        fd2 = fds[1];
    }
    ~RPCServerTest() {
        if (fd1 != -1)
            EXPECT_EQ(0, close(fd1));
        if (fd2 != -1)
            EXPECT_EQ(0, close(fd2));
    }
    Event::Loop loop;
    Address address;
    MyService service;
    Server server;
    int fd1;
    int fd2;
};

TEST_F(RPCServerTest, TCPListener_handleNewConnection) {
    server.listener.handleNewConnection(fd1);
    fd1 = -1;
    ASSERT_EQ(1U, server.sockets.size());
    Server::ServerMessageSocket& socket = *server.sockets.at(0);
    EXPECT_EQ(&server, socket.server);
    EXPECT_EQ(0U, socket.socketsIndex);
    EXPECT_FALSE(socket.self.expired());

    server.listener.server = NULL;
    server.listener.handleNewConnection(fd2);
    fd2 = -1;
    EXPECT_EQ(1U, server.sockets.size());
}

TEST_F(RPCServerTest, MessageSocket_onReceiveMessage) {
    server.listener.handleNewConnection(fd1);
    Server::ServerMessageSocket& socket = *server.sockets.at(0);
    socket.onReceiveMessage(1, Buffer(NULL, 3, NULL));
    ASSERT_TRUE(service.lastRPC);
    EXPECT_EQ(3U, service.lastRPC->request.getLength());
    EXPECT_EQ(0U, service.lastRPC->response.getLength());
    EXPECT_EQ(&socket, service.lastRPC->messageSocket.lock().get());
    EXPECT_EQ(1U, service.lastRPC->messageId);
}

TEST_F(RPCServerTest, MessageSocket_onReceiveMessage_ping) {
    server.listener.handleNewConnection(fd1);
    Server::ServerMessageSocket& socket = *server.sockets.at(0);
    socket.onReceiveMessage(0, Buffer());
    ASSERT_FALSE(service.lastRPC);
    EXPECT_EQ(1U, socket.outboundQueue.size());
}

TEST_F(RPCServerTest, MessageSocket_onDisconnect) {
    server.listener.handleNewConnection(fd1);
    fd1 = -1;
    server.listener.handleNewConnection(fd2);
    fd2 = -1;
    EXPECT_EQ(2U, server.sockets.size());
    server.sockets.at(0)->onDisconnect();
    EXPECT_EQ(1U, server.sockets.size());
    EXPECT_EQ(0U, server.sockets.at(0)->socketsIndex);
}

TEST_F(RPCServerTest, MessageSocket_close) {
    // tested by onDisconnect
}

TEST_F(RPCServerTest, constructor) {
    // tested sufficiently in other tests
}

TEST_F(RPCServerTest, destructor) {
    // difficult to test
}

} // namespace LogCabin::RPC::<anonymous>
} // namespace LogCabin::RPC
} // namespace LogCabin
