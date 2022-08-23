/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ns3/application-5g-helper.h"
#include "ns3/application-5g-server.h"
#include "ns3/application-5g-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

Application5GServerHelper::Application5GServerHelper (uint16_t port)
{
  m_factory.SetTypeId (Application5GServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void 
Application5GServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
Application5GServerHelper::SetTrafficModel (Ptr<Application> app, std::vector<Application5GHelper::Application5GMessage> messages)
{
  Ptr<Application5GClient> test = app->GetObject<Application5GClient>();
  test->SetTrafficModel (messages);
}

ApplicationContainer
Application5GServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
Application5GServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
Application5GServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
Application5GServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application5GServer> ();
  node->AddApplication (app);

  return app;
}

Application5GClientHelper::Application5GClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (Application5GClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

Application5GClientHelper::Application5GClientHelper (Address address)
{
  m_factory.SetTypeId (Application5GClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void 
Application5GClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
Application5GClientHelper::SetTrafficModel (Ptr<Application> app, std::vector<Application5GHelper::Application5GMessage> messages)
{
  app->GetObject<Application5GClient>()->SetTrafficModel (messages);
}

void
Application5GClientHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<Application5GClient>()->SetFill (fill);
}

void
Application5GClientHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<Application5GClient>()->SetFill (fill, dataLength);
}

void
Application5GClientHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<Application5GClient>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
Application5GClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
Application5GClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
Application5GClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
Application5GClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application5GClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
