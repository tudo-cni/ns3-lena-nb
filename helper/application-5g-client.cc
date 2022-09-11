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
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/application-5g-client.h"
#include "ns3/application-5g-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Application5GClientApplication");

NS_OBJECT_ENSURE_REGISTERED (Application5GClient);

TypeId
Application5GClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Application5GClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<Application5GClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&Application5GClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&Application5GClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&Application5GClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&Application5GClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&Application5GClient::SetDataSize,
                                         &Application5GClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&Application5GClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&Application5GClient::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&Application5GClient::m_txTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&Application5GClient::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

Application5GClient::Application5GClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
  m_count_per_run = 0;
  //m_messages.push_back(0);
}

Application5GClient::~Application5GClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void 
Application5GClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
Application5GClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
Application5GClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
Application5GClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&Application5GClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  ScheduleTransmit (Seconds (0.));
}

void 
Application5GClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}

void 
Application5GClient::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so 
  // neither will we.
  //
  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t 
Application5GClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
Application5GClient::SetFill (std::string fill)
{
  NS_LOG_FUNCTION (this << fill);

  uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_data, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
Application5GClient::SetFill (uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset (m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
Application5GClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
Application5GClient::SetTrafficModel (std::vector<Application5GHelper::Application5GMessage> messages)
{
  //NS_LOG_FUNCTION (this << messages);
  m_messages = messages;
  bool remoteAddressfound = false;
  while (!remoteAddressfound){
    for (std::vector<Application5GHelper::Application5GMessage>::iterator it = m_messages.begin (); it != m_messages.end (); ++it){
      if (it->isUL){
        m_peerAddress = it->remoteAddr;
        m_peerPort = it->remotePort;
        remoteAddressfound = true;
      }
    }
  }
}

void 
Application5GClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &Application5GClient::Send, this);
}

void 
Application5GClient::Send (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;

  if (m_messages.size () > 0)
  {
    m_count_per_run = m_count_per_run + 1;
    Application5GHelper::Application5GMessage firstMsg = m_messages[0];
    if (firstMsg.isUL){
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
      if (Ipv4Address::IsMatchingType (m_peerAddress))
        {
          m_txTraceWithAddresses (p, localAddress, InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType (m_peerAddress))
        {
          m_txTraceWithAddresses (p, localAddress, Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      m_socket->Send (p);
      ++m_sent;

      if (Ipv4Address::IsMatchingType (m_peerAddress))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " << Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
        }
      else if (Ipv6Address::IsMatchingType (m_peerAddress))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " << Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " << InetSocketAddress::ConvertFrom (m_peerAddress) << " port " << m_peerPort << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " << Inet6SocketAddress::ConvertFrom (m_peerAddress) << " port " << m_peerPort << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
        }

      // if (m_sent < m_count) 
      //   {
      //     ScheduleTransmit (m_interval);
      //   }
      }
    else{
    //TBD: What if first message is DL?
    }
  }
  else
  {
    std::cout << "Application5GClient::Send: m_messages is empty. No message will be transmitted (for now)." << std::endl;
  }
  
  if (m_count_per_run >= m_messages.size()){
    m_count_per_run = 0; // All messages have been transmitted
  } 
}

void 
Application5GClient::NextSend (void){
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;

  m_count_per_run = m_count_per_run + 1;
    Application5GHelper::Application5GMessage nextMsg = m_messages[m_count_per_run-1];
    if (nextMsg.isUL){
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
      Address localAddress;
      m_socket->GetSockName (localAddress);
      // call to the trace sinks before the packet is actually sent,
      // so that tags added to the packet can be sent as well
      m_txTrace (p);
      if (Ipv4Address::IsMatchingType (m_peerAddress))
        {
          m_txTraceWithAddresses (p, localAddress, InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType (m_peerAddress))
        {
          m_txTraceWithAddresses (p, localAddress, Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      m_socket->Send (p);
      ++m_sent;

      if (Ipv4Address::IsMatchingType (m_peerAddress))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " << Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
        }
      else if (Ipv6Address::IsMatchingType (m_peerAddress))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " << Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " << InetSocketAddress::ConvertFrom (m_peerAddress) << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort () << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
        {
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " << Inet6SocketAddress::ConvertFrom (m_peerAddress) << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort () << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client sent " << m_size << " bytes to " <<
                      Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
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
Application5GClient::HandleRead (Ptr<Socket> socket)
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
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client received " << packet->GetSize () << " bytes from " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (from).GetPort () << std::endl;;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          m_count_per_run = m_count_per_run + 1; // Server message received
          std::cout << "At time " << Simulator::Now ().As (Time::S) << " client received " << packet->GetSize () << " bytes from " << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort () << std::endl;
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
      socket->GetSockName (localAddress);
      m_rxTrace (packet);
      m_rxTraceWithAddresses (packet, from, localAddress);
    }
  if (m_count_per_run < m_messages.size()) // There are additional messages that need to be transmitted
  {
    NextSend();
  }
  else
  {
    m_count_per_run = 0; // All messages have been transmitted
  }
   
}

} // Namespace ns3
