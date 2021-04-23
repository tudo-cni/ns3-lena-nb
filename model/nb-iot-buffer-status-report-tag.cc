/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 */


#include "nb-iot-buffer-status-report-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BufferStatusReportTag);

TypeId
BufferStatusReportTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BufferStatusReportTag")
    .SetParent<Tag> ()
    .SetGroupName("Lte")
    .AddConstructor<BufferStatusReportTag> ()
    .AddAttribute ("bsrIndex", "",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BufferStatusReportTag::GetBufferStatusReportIndex),
                   MakeUintegerChecker<uint16_t> ())
    
  ;
  return tid;
}

TypeId
BufferStatusReportTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

BufferStatusReportTag::BufferStatusReportTag()
  : m_bsrIndex(0)
{
}
BufferStatusReportTag::BufferStatusReportTag(uint8_t bsrIndex)
  : m_bsrIndex(bsrIndex)
{
}


uint32_t
BufferStatusReportTag::GetSerializedSize (void) const
{
  return 1;
}

void
BufferStatusReportTag::Serialize (TagBuffer i) const
{
// ETSI 36.321 defines only 4 bit for Data Volume of 1 Byte 
// but we currently dont need the remaing bits, so we can just write the whole byte
  i.WriteU8 (m_bsrIndex);
}

void
BufferStatusReportTag::Deserialize (TagBuffer i)
{
  m_bsrIndex= (uint8_t) i.ReadU8();
}

uint8_t
BufferStatusReportTag::GetBufferStatusReportIndex() const
{
  return m_bsrIndex;
}
void
BufferStatusReportTag::SetBufferStatusReportIndex(uint8_t bsrIndex) 
{
  m_bsrIndex= bsrIndex;
}
void
BufferStatusReportTag::Print (std::ostream &os) const
{
  os << "BSR Index=" << (uint16_t)m_bsrIndex;
}

} // namespace ns3


