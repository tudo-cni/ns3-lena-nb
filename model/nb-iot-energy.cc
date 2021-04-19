#include "nb-iot-energy.h"
namespace ns3{

NS_OBJECT_ENSURE_REGISTERED (NbiotEnergyModel);

void NbiotChip::SetDownlinkPower(double power){
    m_downlinkPower = power;
}
void NbiotChip::SetUplinkPower(double power){
    m_uplinkPower = power;
}
void NbiotChip::SetEdrxPower(double power){
    m_edrxPower = power;
}
void NbiotChip::SetDrxPower(double power){
    m_drxPower = power;
}
void NbiotChip::SetPsmPower(double power){
    m_psmPower = power;
}
void NbiotChip::SetIdlePower(double power){
    m_idlePower = power;
}
double NbiotChip::GetDownlinkPower(){
    return m_downlinkPower;
}
double NbiotChip::GetUplinkPower(){
    return m_uplinkPower;
}
double NbiotChip::GetEdrxPower(){
    return m_edrxPower;
}
double NbiotChip::GetDrxPower(){
    return m_drxPower;
}
double NbiotChip::GetPsmPower(){
    return m_psmPower;
}
double NbiotChip::GetIdlePower(){
    return m_idlePower;
}
NbiotEnergyModel::~NbiotEnergyModel(){
    DoNotifyStateChange(PowerState::OFF);
}

NbiotEnergyModel::PowerState 
NbiotEnergyModel::DoGetState(){
    return m_lastState;
}

void NbiotEnergyModel::SetImsi(uint32_t imsi){
    m_imsi = imsi;
}
void NbiotEnergyModel::DoNotifyStateChange(PowerState newState){
    Time stateTime = (Simulator::Now()-m_lastStateChange); // subframes are [ms] and we need [s]
    double lostEnergy = 0; // Energy in [Ws] or [J]
    m_states.push_back(std::pair<PowerState,uint32_t>(newState,Simulator::Now().GetMilliSeconds()));
    switch (m_lastState)
    {
    case PowerState::RRC_CONNECTED_IDLE:
        lostEnergy = m_module.GetIdlePower()*stateTime.GetSeconds(); // Energy in [Ws] or [J]
        break;
    case PowerState::RRC_CONNECTED_RECEIVING_NPDCCH:
    case PowerState::RRC_CONNECTED_RECEIVING_NPDSCH:
        lostEnergy = m_module.GetDownlinkPower()*stateTime.GetSeconds();
        break;
    case PowerState::RRC_CONNECTED_SENDING_NPRACH:
    case PowerState::RRC_CONNECTED_SENDING_NPUSCH:
    case PowerState::RRC_CONNECTED_SENDING_NPUSCH_F2:
        lostEnergy = m_module.GetUplinkPower()*stateTime.GetSeconds();
        break;
    case PowerState::RRC_SUSPENDED_EDRX:
        lostEnergy = m_module.GetEdrxPower()*stateTime.GetSeconds();
        break;
    case PowerState::RRC_SUSPENDED_DRX:
        lostEnergy = m_module.GetDrxPower()*stateTime.GetSeconds();
        break;
    case PowerState::RRC_SUSPENDED_PSM:
        lostEnergy = m_module.GetPsmPower()*stateTime.GetSeconds();
    default:
        break;
    }
    m_lastStateChange = Simulator::Now();
    m_battery->DecreaseRemainingEnergy(lostEnergy);
    m_timeSpendInState[m_lastState] += stateTime.GetMilliSeconds();
    m_energySpendInState[m_lastState] += lostEnergy;
    m_lastState = newState;

}
double NbiotEnergyModel::GetEnergyRemaining(){
    return m_battery->GetRemainingEnergy();
}
double NbiotEnergyModel::GetEnergyRemainingFraction(){
    return m_battery->GetEnergyFraction();
}
}
