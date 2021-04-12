/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *         Marco Miozzo <marco.miozzo@cttc.es>
 */

#ifndef NBIOT_ENERGY_H
#define NBIOT_ENERGY_H

#include <ns3/object.h>
#include <ns3/simulator.h>
#include <ns3/li-ion-energy-source.h>
#include <cmath>

namespace ns3 {

class NbiotChip {
protected:
    double m_psmPower; 
    double m_drxPower;
    double m_edrxPower;
    double m_uplinkPower;
    double m_downlinkPower;
    double m_idlePower;
    
public:
    NbiotChip():
        m_psmPower(0),
        m_drxPower(0),
        m_edrxPower(0),
        m_uplinkPower(0),
        m_downlinkPower(0),
        m_idlePower(0){}
    NbiotChip(double psmPower, double drxPower, double edrxPower, double uplinkPower, double downlinkPower, double idlePower):
        m_psmPower(psmPower),
        m_drxPower(drxPower),
        m_edrxPower(edrxPower),
        m_uplinkPower(uplinkPower),
        m_downlinkPower(downlinkPower),
        m_idlePower(idlePower){}
    ~NbiotChip(){}

    void SetPsmPower(double Power);
    void SetDrxPower(double Power);
    void SetEdrxPower(double Power);
    void SetUplinkPower(double Power);
    void SetDownlinkPower(double Power);
    void SetIdlePower(double Power);

    double GetPsmPower();
    double GetDrxPower();
    double GetEdrxPower();
    double GetUplinkPower();
    double GetDownlinkPower();
    double GetIdlePower();
};

class BG96 : public NbiotChip{
public:
    BG96(){
        // Compare Joerke nbiot-nidd-ciotopt
        m_psmPower = 3.8* 3.9*std::pow(10,-6);
        m_drxPower = 3.8* 1.56*std::pow(10,-3);
        m_edrxPower = 3.8* 0.81*std::pow(10,-3);
        m_uplinkPower = 3.8* 155*std::pow(10,-3);
        m_downlinkPower = 80*std::pow(10,-3);
        m_idlePower = 3.8 * 0.81*std::pow(10,-3);
    };
};

class NbiotEnergyModel : public Object
{
public: 
    enum class PowerState{
        RRC_CONNECTED_IDLE,
        RRC_CONNECTED_RECEIVING_NPDSCH,
        RRC_CONNECTED_RECEIVING_NPDCCH,
        RRC_CONNECTED_SENDING_NPRACH,
        RRC_CONNECTED_SENDING_NPUSCH,
        RRC_CONNECTED_SENDING_NPUSCH_F2,
        RRC_SUSPENDED_DRX,
        RRC_SUSPENDED_EDRX,
        RRC_SUSPENDED_PSM,
        OFF
    }powerState;

    NbiotEnergyModel(NbiotChip module): 
        m_module(module),
        m_lastState(PowerState::OFF),
        m_lastStateChange(Time(0)){
        m_battery = CreateObject<LiIonEnergySource>();
        }
    ~NbiotEnergyModel();
    

    void DoNotifyStateChange(PowerState newState);
    PowerState DoGetState();

private:
    Ptr<LiIonEnergySource> m_battery; // Battery model
    NbiotChip m_module; // Current Nbiot Module
    PowerState m_lastState; // Current Powerstate
    Time m_lastStateChange;
    std::map<PowerState, double> m_timeSpendInState; // Statistics
    std::map<PowerState, double> m_energySpendInState; // Statistics
    std::vector<std::pair<PowerState,uint32_t>> m_states;

};
}
#endif /* FF_MAC_SCHEDULER_H */

