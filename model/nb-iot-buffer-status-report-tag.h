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

#ifndef NBIOT_BSR_H
#define NBIOT_BSR_H


#include "ns3/tag.h"

namespace ns3 {

class Tag;


/**
 * Tag used to define the RNTI and LC id for each MAC packet trasmitted
 */

class BufferStatusReportTag : public Tag
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Create an empty LteRadioBearerTag
   */
  BufferStatusReportTag ();

  /**
   * Create a LteRadioBearerTag with the given RNTI and LC id
   * \param rnti the RNTI
   * \param lcId the LCID
   */
  BufferStatusReportTag (uint8_t bsrIndex);

  // ETSI 136.321 V 13.9.0 6.1.3.10
  void SetBufferStatusReportIndex(uint8_t bsrIndex); // 0-15 
  
  uint8_t GetBufferStatusReportIndex() const;


  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual uint32_t GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_bsrIndex; ///< RNTI

};



} // namespace ns3

#endif /* LTE_RADIO_BEARER_TAG_H */

