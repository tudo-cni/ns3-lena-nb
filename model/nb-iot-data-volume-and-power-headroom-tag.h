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
 * Author: Author: Marco Miozzo  <marco.miozzo@cttc.es>
 */

#ifndef NBIOT_DPR_H
#define NBIOT_DPR_H


#include "ns3/tag.h"

namespace ns3 {

class Tag;


/**
 * Tag used to define the RNTI and LC id for each MAC packet trasmitted
 */

class DataVolumeAndPowerHeadroomTag : public Tag
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
  DataVolumeAndPowerHeadroomTag ();

  /**
   * Create a LteRadioBearerTag with the given RNTI and LC id
   * \param rnti the RNTI
   * \param lcId the LCID
   */
  DataVolumeAndPowerHeadroomTag (uint8_t dataVoluemValue);

  // ETSI 136.321 V 13.9.0 6.1.3.10
  void SetDataVolumeValue(uint8_t dataVolumeValue); // 0-15 
  
  uint8_t GetDataVolumeValue() const;


  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual uint32_t GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_dataVolumeValue; ///< RNTI

};



} // namespace ns3

#endif /* LTE_RADIO_BEARER_TAG_H */
