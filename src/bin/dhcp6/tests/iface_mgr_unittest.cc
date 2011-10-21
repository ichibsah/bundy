// Copyright (C) 2011  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <config.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <arpa/inet.h>
#include <gtest/gtest.h>

#include "io_address.h"
#include "dhcp/pkt6.h"
#include "dhcp6/iface_mgr.h"

using namespace std;
using namespace isc;
using namespace isc::asiolink;
using namespace isc::dhcp;

// name of loopback interface detection
char LOOPBACK[32] = "lo";

namespace {
const char* const INTERFACE_FILE = TEST_DATA_BUILDDIR "/interfaces.txt";

class NakedIfaceMgr: public IfaceMgr {
    // "naked" Interface Manager, exposes internal fields
public:
    NakedIfaceMgr() { }
    IfaceLst & getIfacesLst() { return ifaces_; }
    void setSendSock(int sock) { sendsock_ = sock; }
    void setRecvSock(int sock) { recvsock_ = sock; }

    int openSocket(const std::string& ifname,
                   const isc::asiolink::IOAddress& addr,
                   int port) {
        return IfaceMgr::openSocket(ifname, addr, port);
    }

};

// dummy class for now, but this will be expanded when needed
class IfaceMgrTest : public ::testing::Test {
public:
    IfaceMgrTest() {
    }
};

// We need some known interface to work reliably. Loopback interface
// is named lo on Linux and lo0 on BSD boxes. We need to find out
// which is available. This is not a real test, but rather a workaround
// that will go away when interface detection is implemented.

// NOTE: At this stage of development, write access to current directory
// during running tests is required.
TEST_F(IfaceMgrTest, loDetect) {

    unlink("interfaces.txt");

    ofstream interfaces("interfaces.txt", ios::ate);
    interfaces << "lo ::1";
    interfaces.close();

    NakedIfaceMgr * ifacemgr = new NakedIfaceMgr();
    IOAddress loAddr("::1");
    IOAddress mcastAddr("ff02::1:2");

    // bind multicast socket to port 10547
    int socket1 = ifacemgr->openSocket("lo", mcastAddr, 10547);
    // this fails on BSD (there's no lo interface there)

    // poor man's interface dection
    // it will go away as soon as proper interface detection
    // is implemented
    if (socket1>0) {
        cout << "This is Linux, using lo as loopback." << endl;
        close(socket1);
    } else {
        // this fails on Linux and succeeds on BSD
        socket1 = ifacemgr->openSocket("lo0", mcastAddr, 10547);
        if (socket1>0) {
            sprintf(LOOPBACK, "lo0");
            cout << "This is BSD, using lo0 as loopback." << endl;
            close(socket1);
        } else {
            cout << "Failed to detect loopback interface. Neither "
                 << "lo or lo0 worked. I give up." << endl;
            ASSERT_TRUE(false);
        }
    }

    delete ifacemgr;
}

// uncomment this test to create packet writer. It will
// write incoming DHCPv6 packets as C arrays. That is useful
// for generating test sequences based on actual traffic
//
// TODO: this potentially should be moved to a separate tool
//

#if 0
TEST_F(IfaceMgrTest, dhcp6Sniffer) {
    // testing socket operation in a portable way is tricky
    // without interface detection implemented

    unlink("interfaces.txt");

    ofstream interfaces("interfaces.txt", ios::ate);
    interfaces << "eth0 fe80::21e:8cff:fe9b:7349";
    interfaces.close();

    NakedIfaceMgr * ifacemgr = new NakedIfaceMgr();

    Pkt6 * pkt = 0;
    int cnt = 0;
    cout << "---8X-----------------------------------------" << endl;
    while (true) {
        pkt = ifacemgr->receive();

        cout << "// Received " << pkt->data_len_ << " bytes packet:" << endl;
        cout << "Pkt6 *capture" << cnt++ << "() {" << endl;
        cout << "    Pkt6* pkt;" << endl;
        cout << "    pkt = new Pkt6(" << pkt->data_len_ << ");" << endl;
        cout << "    pkt->remote_port_ = " << pkt-> remote_port_ << ";" << endl;
        cout << "    pkt->remote_addr_ = IOAddress(\""
             << pkt->remote_addr_.toText() << "\");" << endl;
        cout << "    pkt->local_port_ = " << pkt-> local_port_ << ";" << endl;
        cout << "    pkt->local_addr_ = IOAddress(\""
             << pkt->local_addr_.toText() << "\");" << endl;
        cout << "    pkt->ifindex_ = " << pkt->ifindex_ << ";" << endl;
        cout << "    pkt->iface_ = \"" << pkt->iface_ << "\";" << endl;

        // TODO it is better to declare an array and then memcpy it to
        // packet.
        for (int i=0; i< pkt->data_len_; i++) {
            cout << "    pkt->data_[" << i << "]="
                 << (int)(unsigned char)pkt->data_[i] << "; ";
            if (!(i%4))
                cout << endl;
        }
        cout << endl;
        cout << "    return (pkt);" << endl;
        cout << "}" << endl << endl;

        delete pkt;
    }
    cout << "---8X-----------------------------------------" << endl;

    // never happens. Infinite loop is infinite
    delete pkt;
    delete ifacemgr;
}
#endif

TEST_F(IfaceMgrTest, basic) {
    // checks that IfaceManager can be instantiated

    IfaceMgr & ifacemgr = IfaceMgr::instance();
    ASSERT_TRUE(&ifacemgr != 0);
}

TEST_F(IfaceMgrTest, ifaceClass) {
    // basic tests for Iface inner class

    IfaceMgr::Iface * iface = new IfaceMgr::Iface("eth5", 7);

    EXPECT_STREQ("eth5/7", iface->getFullName().c_str());

    delete iface;

}

// TODO: Implement getPlainMac() test as soon as interface detection
// is implemented.
TEST_F(IfaceMgrTest, getIface) {

    cout << "Interface checks. Please ignore socket binding errors." << endl;
    NakedIfaceMgr * ifacemgr = new NakedIfaceMgr();

    // interface name, ifindex
    IfaceMgr::Iface iface1("lo1", 1);
    IfaceMgr::Iface iface2("eth5", 2);
    IfaceMgr::Iface iface3("en3", 5);
    IfaceMgr::Iface iface4("e1000g0", 3);

    // note: real interfaces may be detected as well
    ifacemgr->getIfacesLst().push_back(iface1);
    ifacemgr->getIfacesLst().push_back(iface2);
    ifacemgr->getIfacesLst().push_back(iface3);
    ifacemgr->getIfacesLst().push_back(iface4);

    cout << "There are " << ifacemgr->getIfacesLst().size()
         << " interfaces." << endl;
    for (IfaceMgr::IfaceLst::iterator iface=ifacemgr->getIfacesLst().begin();
         iface != ifacemgr->getIfacesLst().end();
         ++iface) {
        cout << "  " << iface->name_ << "/" << iface->ifindex_ << endl;
    }


    // check that interface can be retrieved by ifindex
    IfaceMgr::Iface * tmp = ifacemgr->getIface(5);
    // ASSERT_NE(NULL, tmp); is not supported. hmmmm.
    ASSERT_TRUE( tmp != NULL );

    EXPECT_STREQ( "en3", tmp->name_.c_str() );
    EXPECT_EQ(5, tmp->ifindex_);

    // check that interface can be retrieved by name
    tmp = ifacemgr->getIface("lo1");
    ASSERT_TRUE( tmp != NULL );

    EXPECT_STREQ( "lo1", tmp->name_.c_str() );
    EXPECT_EQ(1, tmp->ifindex_);

    // check that non-existing interfaces are not returned
    EXPECT_EQ(static_cast<void*>(NULL), ifacemgr->getIface("wifi0") );

    delete ifacemgr;
}

TEST_F(IfaceMgrTest, detectIfaces) {

    // test detects that interfaces can be detected
    // there is no code for that now, but interfaces are
    // read from file
    fstream fakeifaces(INTERFACE_FILE, ios::out|ios::trunc);
    fakeifaces << "eth0 fe80::1234";
    fakeifaces.close();

    // this is not usable on systems that don't have eth0
    // interfaces. Nevertheless, this fake interface should
    // be on list, but if_nametoindex() will fail.

    NakedIfaceMgr * ifacemgr = new NakedIfaceMgr();

    ASSERT_TRUE( ifacemgr->getIface("eth0") != NULL );

    IfaceMgr::Iface * eth0 = ifacemgr->getIface("eth0");

    // there should be one address
    EXPECT_EQ(1, eth0->addrs_.size());

    IOAddress * addr = &(*eth0->addrs_.begin());
    ASSERT_TRUE( addr != NULL );

    EXPECT_STREQ( "fe80::1234", addr->toText().c_str() );

    delete ifacemgr;
}

// TODO: disabled due to other naming on various systems
// (lo in Linux, lo0 in BSD systems)
// Fix for this is available on 1186 branch, will reenable
// this test once 1186 is merged
TEST_F(IfaceMgrTest, DISABLED_sockets) {
    // testing socket operation in a portable way is tricky
    // without interface detection implemented

    NakedIfaceMgr * ifacemgr = new NakedIfaceMgr();

    IOAddress loAddr("::1");

    // bind multicast socket to port 10547
    int socket1 = ifacemgr->openSocket(LOOPBACK, loAddr, 10547);
    EXPECT_GT(socket1, 0); // socket > 0

    // bind unicast socket to port 10548
    int socket2 = ifacemgr->openSocket(LOOPBACK, loAddr, 10548);
    EXPECT_GT(socket2, 0);

    // expect success. This address/port is already bound, but
    // we are using SO_REUSEADDR, so we can bind it twice
    int socket3 = ifacemgr->openSocket(LOOPBACK, loAddr, 10547);

    // rebinding succeeds on Linux, fails on BSD
    // TODO: add OS-specific defines here (or modify code to
    // behave the same way on all OSes, but that may not be
    // possible
    // EXPECT_GT(socket3, 0); // socket > 0

    // we now have 3 sockets open at the same time. Looks good.

    close(socket1);
    close(socket2);
    close(socket3);

    delete ifacemgr;
}

// TODO: disabled due to other naming on various systems
// (lo in Linux, lo0 in BSD systems)
TEST_F(IfaceMgrTest, DISABLED_socketsMcast) {
    // testing socket operation in a portable way is tricky
    // without interface detection implemented

    NakedIfaceMgr * ifacemgr = new NakedIfaceMgr();

    IOAddress loAddr("::1");
    IOAddress mcastAddr("ff02::1:2");

    // bind multicast socket to port 10547
    int socket1 = ifacemgr->openSocket(LOOPBACK, mcastAddr, 10547);
    EXPECT_GT(socket1, 0); // socket > 0

    // expect success. This address/port is already bound, but
    // we are using SO_REUSEADDR, so we can bind it twice
    int socket2 = ifacemgr->openSocket(LOOPBACK, mcastAddr, 10547);
    EXPECT_GT(socket2, 0);

    // there's no good way to test negative case here.
    // we would need non-multicast interface. We will be able
    // to iterate thru available interfaces and check if there
    // are interfaces without multicast-capable flag.

    close(socket1);
    close(socket2);

    delete ifacemgr;
}

// TODO: disabled due to other naming on various systems
// (lo in Linux, lo0 in BSD systems)
// Fix for this is available on 1186 branch, will reenable
// this test once 1186 is merged
TEST_F(IfaceMgrTest, DISABLED_sendReceive) {
    // testing socket operation in a portable way is tricky
    // without interface detection implemented

    fstream fakeifaces(INTERFACE_FILE, ios::out|ios::trunc);
    fakeifaces << LOOPBACK << " ::1";
    fakeifaces.close();

    NakedIfaceMgr * ifacemgr = new NakedIfaceMgr();

    // let's assume that every supported OS have lo interface
    IOAddress loAddr("::1");
    int socket1 = ifacemgr->openSocket(LOOPBACK, loAddr, 10547);
    int socket2 = ifacemgr->openSocket(LOOPBACK, loAddr, 10546);

    ifacemgr->setSendSock(socket2);
    ifacemgr->setRecvSock(socket1);

    boost::shared_ptr<Pkt6> sendPkt(new Pkt6(128) );

    // prepare dummy payload
    for (int i=0;i<128; i++) {
        sendPkt->data_[i] = i;
    }

    sendPkt->remote_port_ = 10547;
    sendPkt->remote_addr_ = IOAddress("::1");
    sendPkt->ifindex_ = 1;
    sendPkt->iface_ = LOOPBACK;

    boost::shared_ptr<Pkt6> rcvPkt;

    EXPECT_EQ(true, ifacemgr->send(sendPkt));

    rcvPkt = ifacemgr->receive();

    ASSERT_TRUE( rcvPkt ); // received our own packet

    // let's check that we received what was sent
    EXPECT_EQ(sendPkt->data_len_, rcvPkt->data_len_);
    EXPECT_EQ(0, memcmp(&sendPkt->data_[0], &rcvPkt->data_[0],
                        rcvPkt->data_len_) );

    EXPECT_EQ(sendPkt->remote_addr_.toText(), rcvPkt->remote_addr_.toText());
    EXPECT_EQ(rcvPkt->remote_port_, 10546);

    delete ifacemgr;
}

}