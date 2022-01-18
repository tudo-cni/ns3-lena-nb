/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Communication Networks Institute at TU Dortmund University
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
 * Author: Tim Gebauer <tim.gebauer@tu-dortmund.de>
 */


#include "nb-iot-data-volume-and-power-headroom-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DataVolumeAndPowerHeadroomTag);

TypeId
DataVolumeAndPowerHeadroomTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DataVolumeAndPowerHeadroomTag")
    .SetParent<Tag> ()
    .SetGroupName("Lte")
    .AddConstructor<DataVolumeAndPowerHeadroomTag> ()
    .AddAttribute ("dataVolume", "",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DataVolumeAndPowerHeadroomTag::GetDataVolumeValue),
                   MakeUintegerChecker<uint16_t> ())
    
  ;
  return tid;
}

TypeId
DataVolumeAndPowerHeadroomTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

DataVolumeAndPowerHeadroomTag::DataVolumeAndPowerHeadroomTag()
  : m_dataVolumeValue(0)
{
}
DataVolumeAndPowerHeadroomTag::DataVolumeAndPowerHeadroomTag(uint8_t dataVolumeValue)
  : m_dataVolumeValue (dataVolumeValue)
{
}


uint32_t
DataVolumeAndPowerHeadroomTag::GetSerializedSize (void) const
{
  return 1;
}

void
DataVolumeAndPowerHeadroomTag::Serialize (TagBuffer i) const
{
// ETSI 36.321 defines only 4 bit for Data Volume of 1 Byte 
// but we currently dont need the remaing bits, so we can just write the whole byte
  i.WriteU8 (m_dataVolumeValue);
}

void
DataVolumeAndPowerHeadroomTag::Deserialize (TagBuffer i)
{
  m_dataVolumeValue= (uint8_t) i.ReadU8();
}

uint8_t
DataVolumeAndPowerHeadroomTag::GetDataVolumeValue () const
{
  return m_dataVolumeValue;
}
void
DataVolumeAndPowerHeadroomTag::SetDataVolumeValue (uint8_t dataVolumeValue) 
{
  m_dataVolumeValue = dataVolumeValue;
}
void
DataVolumeAndPowerHeadroomTag::Print (std::ostream &os) const
{
  os << "dataVolume=" << (uint16_t)m_dataVolumeValue;
}

} // namespace ns3

