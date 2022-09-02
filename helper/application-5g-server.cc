/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/application-5g-server.h"
#include "ns3/application-5g-helper.h"
#include "ns3/vector.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Application5GServerApplication");

NS_OBJECT_ENSURE_REGISTERED (Application5GServer);

TypeId
Application5GServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Application5GServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<Application5GServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&Application5GServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&Application5GServer::m_remote),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&Application5GServer::m_remote_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&Application5GServer::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&Application5GServer::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

Application5GServer::Application5GServer ()
{
  NS_LOG_FUNCTION (this);
}

Application5GServer::~Application5GServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
}

void 
Application5GServer::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_remote = ip;
  m_remote_port = port;
}

void
Application5GServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
Application5GServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      if (m_socket6->Bind (local6) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&Application5GServer::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&Application5GServer::HandleRead, this));
}

void 
Application5GServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket6 != 0) 
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void 
Application5GServer::SetTrafficModel (std::vector<Application5GHelper::Application5GMessage> messages)
{
  //NS_LOG_FUNCTION (this << messages);

  m_messages = messages;
  bool remoteAddressfound = false;
  while (!remoteAddressfound){
    for (std::vector<Application5GHelper::Application5GMessage>::iterator it = m_messages.begin (); it != m_messages.end (); ++it){
      if (it->isUL){
        m_port = it->remotePort;
        remoteAddressfound = true;
      }
    }
  }
}

void 
Application5GServer::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &Application5GServer::Send, this);
}

void 
Application5GServer::Send (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;

  if (m_messages.size () > 0)
  {
    m_count_per_run = m_count_per_run + 1;
    Application5GHelper::Application5GMessage firstMsg = m_messages[0];
    if (firstMsg.isUL == false){
      m_size = firstMsg.sizeBytes;
      if (m_dataSize)
        {
          //
          // If m_dataSize is non-zero, we have a data buffer of the same size that we
          // are expected to copy and send.  This state of affairs is created if one of
          // the Fill functions is called.  In this case, m_size must have been set
          // to agree with m_dataSize
          //
          NS_ASSERT_MSG (m_dataSize == m_size, "Application5GClient::Send(): m_size and m_dataSize inconsistent");
          NS_ASSERT_MSG (m_data, "Application5GClient::Send(): m_dataSize but no m_data");
          p = Create<Packet> (m_data, m_dataSize);
        }
      else
        {
          //
          // If m_dataSize is zero, the client has indicated that it doesn't care
          // about the data itself either by specifying the data size by setting
          // the corresponding attribute or by not calling a SetFill function.  In
          // this case, we don't worry about it either.  But we do allow m_size
          // to have a value different from the (zero) m_dataSize.
          //
          p = Create<Packet> (m_size);
        }
      Address localAddress;
      m_socket->GetSockName (localAddress);
      // call to the trace sinks before the packet is actually sent,
      // so that tags added to the packet can be sent as well
      m_txTrace (p);
      if (Ipv4Address::IsMatchingType (m_remote))
        {
          m_txTraceWithAddresses (p, localAddress, InetSocketAddress (Ipv4Address::ConvertFrom (m_remote), m_remote_port));
        }
      else if (Ipv6Address::IsMatchingType (m_remote))
        {
          m_txTraceWithAddresses (p, localAddress, Inet6SocketAddress (Ipv6Address::ConvertFrom (m_remote), m_remote_port));
        }
      m_socket->Send (p);
      ++m_sent;

      if (Ipv4Address::IsMatchingType (m_remote))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      Ipv4Address::ConvertFrom (m_remote) << " port " << m_remote_port);
        }
      else if (Ipv6Address::IsMatchingType (m_remote))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      Ipv6Address::ConvertFrom (m_remote) << " port " << m_remote_port);
        }
      else if (InetSocketAddress::IsMatchingType (m_remote))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      InetSocketAddress::ConvertFrom (m_remote).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_remote).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (m_remote))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      Inet6SocketAddress::ConvertFrom (m_remote).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (m_remote).GetPort ());
        }

      if (m_sent < m_count) 
        {
          ScheduleTransmit (m_interval);
        }
      }
    }
    else{
      //TBD: What if first message is UL?
    }
    
    if (m_count_per_run >= m_messages.size()){
      m_count_per_run = 0; // All messages have been transmitted
    }
    
}

void 
Application5GServer::NextSend (Ptr<Socket> socket){
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;

    Application5GHelper::Application5GMessage nextMsg = m_messages[m_count_per_run];
    if (!nextMsg.isUL){
      m_size = nextMsg.sizeBytes;
      if (m_dataSize)
        {
          //
          // If m_dataSize is non-zero, we have a data buffer of the same size that we
          // are expected to copy and send.  This state of affairs is created if one of
          // the Fill functions is called.  In this case, m_size must have been set
          // to agree with m_dataSize
          //
          NS_ASSERT_MSG (m_dataSize == m_size, "Application5GClient::Send(): m_size and m_dataSize inconsistent");
          NS_ASSERT_MSG (m_data, "Application5GClient::Send(): m_dataSize but no m_data");
          p = Create<Packet> (m_data, m_dataSize);
        }
      else
        {
          //
          // If m_dataSize is zero, the client has indicated that it doesn't care
          // about the data itself either by specifying the data size by setting
          // the corresponding attribute or by not calling a SetFill function.  In
          // this case, we don't worry about it either.  But we do allow m_size
          // to have a value different from the (zero) m_dataSize.
          //
          p = Create<Packet> (m_size);
        }
      //Address localAddress;
      //m_socket->GetSockName (localAddress);
      // call to the trace sinks before the packet is actually sent,
      // so that tags added to the packet can be sent as well
      //m_txTrace (p);
      // if (Ipv4Address::IsMatchingType (m_remote))
      //   {
      //     m_txTraceWithAddresses (p, localAddress, InetSocketAddress (Ipv4Address::ConvertFrom (m_remote), m_remote_port));
      //   }
      // else if (Ipv6Address::IsMatchingType (m_remote))
      //   {
      //     m_txTraceWithAddresses (p, localAddress, Inet6SocketAddress (Ipv6Address::ConvertFrom (m_remote), m_remote_port));
      //   }
      socket->Send (p);
        m_count_per_run = m_count_per_run + 1;
      ++m_sent;

      if (Ipv4Address::IsMatchingType (m_remote))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " server sent " << m_size << " bytes to " << Ipv4Address::ConvertFrom (m_remote) << " port " << m_remote_port << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server sent " << m_size << " bytes to " <<
                      Ipv4Address::ConvertFrom (m_remote) << " port " << m_remote_port);
        }
      else if (Ipv6Address::IsMatchingType (m_remote))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " server sent " << m_size << " bytes to " << Ipv6Address::ConvertFrom (m_remote) << " port " << m_remote_port << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server sent " << m_size << " bytes to " <<
                      Ipv6Address::ConvertFrom (m_remote) << " port " << m_remote_port);
        }
      else if (InetSocketAddress::IsMatchingType (m_remote))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " server sent " << m_size << " bytes to " << InetSocketAddress::ConvertFrom (m_remote) << " port " << m_remote_port << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server sent " << m_size << " bytes to " <<
                      InetSocketAddress::ConvertFrom (m_remote).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_remote).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (m_remote))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " server sent " << m_size << " bytes to " << Inet6SocketAddress::ConvertFrom (m_remote) << " port " << m_remote_port << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server sent " << m_size << " bytes to " <<
                      Inet6SocketAddress::ConvertFrom (m_remote).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (m_remote).GetPort ());
        }

      // if (m_sent < m_count) 
      //   {
      //     ScheduleTransmit (m_interval);
      //   }
      }
    else{
      //TBD: What if next message is DL?
    }

    if (m_count_per_run >= m_messages.size()){
      m_count_per_run = 0; // All messages have been transmitted
    }
}

void 
Application5GServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          m_count_per_run = m_count_per_run + 1; // Server message received
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " server received " << packet->GetSize () << " bytes from " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (from).GetPort () << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
          m_remote = from;
          m_remote_port = InetSocketAddress::ConvertFrom (from).GetPort ();
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          m_count_per_run = m_count_per_run + 1; // Server message received
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " server received " << packet->GetSize () << " bytes from " << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort () << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
          m_remote = from;
          m_remote_port = InetSocketAddress::ConvertFrom (from).GetPort ();
        }
      socket->GetSockName (localAddress);
      m_txTrace (packet);
      m_txTraceWithAddresses (packet, from, localAddress);
    }
  if (m_count_per_run < m_messages.size()) // There are additional messages that need to be transmitted
  {
    NextSend(socket);
  }
  else
  {
    m_count_per_run = 0; // All messages have been transmitted
  }

  // Ptr<Packet> packet;
  // Address from;
  // Address localAddress;
  // while ((packet = socket->RecvFrom (from)))
  //   {
  //     socket->GetSockName (localAddress);
  //     m_remote = from;
  //     m_remote_port = InetSocketAddress::ConvertFrom (from).GetPort ();
  //     m_rxTrace (packet);
  //     m_rxTraceWithAddresses (packet, from, localAddress);
  //     if (InetSocketAddress::IsMatchingType (from))
  //       {
  //         NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server received " << packet->GetSize () << " bytes from " <<
  //                      InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
  //                      InetSocketAddress::ConvertFrom (from).GetPort ());
  //       }
  //     else if (Inet6SocketAddress::IsMatchingType (from))
  //       {
  //         NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server received " << packet->GetSize () << " bytes from " <<
  //                      Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
  //                      Inet6SocketAddress::ConvertFrom (from).GetPort ());
  //       }

  //     packet->RemoveAllPacketTags ();
  //     packet->RemoveAllByteTags ();

  //     NS_LOG_LOGIC ("Echoing packet");
  //     socket->SendTo (packet, 0, from);

  //     if (InetSocketAddress::IsMatchingType (from))
  //       {
  //         NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server sent " << packet->GetSize () << " bytes to " <<
  //                      InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
  //                      InetSocketAddress::ConvertFrom (from).GetPort ());
  //       }
  //     else if (Inet6SocketAddress::IsMatchingType (from))
  //       {
  //         NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " server sent " << packet->GetSize () << " bytes to " <<
  //                      Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
  //                      Inet6SocketAddress::ConvertFrom (from).GetPort ());
  //       }
  //   }
}

} // Namespace ns3
