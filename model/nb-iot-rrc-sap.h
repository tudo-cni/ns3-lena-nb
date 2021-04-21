#ifndef NBIOT_RRC_SAP_H
#define NBIOT_RRC_SAP_H

#include <stdint.h>
#include <list>
#include <bitset>

#include <ns3/ptr.h>
#include <ns3/simulator.h>
#include <ns3/packet.h>


namespace ns3{
class NbIotRrcSap{
    public:
        NbIotRrcSap();
        virtual ~NbIotRrcSap();

        // ETSI TS 136 331 - V15.3.0 Page 770
        struct ChannelRasterOffsetNb{
            enum {
                khz_7dot5,
                khz_2dot5,
                khz2dot5,
                khz7dot5
            } cr;
        };

        // ETSI TS 136 331 - V15.3.0 Page 731
        struct EutraNumCrsPortsNb{
            enum{
                same,
                four
            } num;
        };

        // ETSI TS 136 331 - V15.3.0 Page 731
        struct GuardbandNb  {
            bool enabled;
            ChannelRasterOffsetNb channelRasterOffset; // offset in kHz
            std::bitset<3> spare;
        };

        // ETSI TS 136 331 - V15.3.0 Page 731
        struct InbandSamePciNb {
            bool enabled;
            uint8_t eutraCrsSequenceInfo; // Integer value (0..31)
        };

        // ETSI TS 136 331 - V15.3.0 Page 731
        struct InbandDifferentPciNb {
            bool enabled;
            EutraNumCrsPortsNb eutraNumberCrsPorts; 
            ChannelRasterOffsetNb channelRasterOffset;
            std::bitset<2> spare;
        };

        // ETSI TS 136 331 - V15.3.0 Page 732
        struct StandaloneNb{
            bool enabled;
            std::bitset<5> spare;
        };

        // ETSI TS 136 331 - V15.3.0 Page 732
        struct MasterInformationBlockNb{

            // Defines the 4 MSBs of the SystemFrameNumber. The 6 LSB are acquired by decoding NPBCH
            std::bitset<4> systemFrameNumberMsb; 

            // Defines the 2 LSBs of the hyper SFN. Remaining MSBs are presented in SIB1-NB
            std::bitset<2> hyperSfnLsb; 

            // TS 36.213 Table 16.4.1.3-3
            uint8_t schedulingInfoSib1; // Integer 0..31

            //Indication of changes in SystemInformations 
            uint8_t systemInfoValueTag; // Integer 0..31

            // indicates that access barring is enabled and the the UE shall acquire SIB14-NB before initiating RRC connection establishment or resume
            bool abEnabled; 

            // NB-IoT Deployment scenario
            GuardbandNb guardBand;
            InbandSamePciNb inbandSamePci;
            InbandDifferentPciNb inbandDifferentPci;
            StandaloneNb standalone;
            // Indications that the number of NPDSCH repetitions is 16
            bool additionalTransmissionSib1;

            std::bitset<3> spare;
        };


        struct IntraFreqReselectionNb{};

        struct CellBaredNb{};

        
        struct TrackingAreaCodeNb{};

        struct PlmnIdentityListNb{};

        struct CellAccessRelatedInfoNb{
            PlmnIdentityListNb plmnidentitylist;
            TrackingAreaCodeNb trackingAreaCode;
            uint16_t cellIdentity;
            CellBaredNb CellBared;
            IntraFreqReselectionNb intraFreqReselection;
        };


        struct CellSelectionInfoNb{
            uint8_t qRxLevMin; // -70..-22
            uint8_t qQualMin; // -34..-3
        };

        struct PMaxNb {};
        struct FreqBandIndicatorNb{};
        struct FreqBandInfoNb{};
        struct NsPmaxListNb {};
        struct MultiBandInfoListNb {};
        struct DownlinkBitmapNb {};

        struct EutraControlRegionSizeNb
        {
            enum class Ecrs {
            N1,
            N2,
            N3
            }ecrs; // Number of OFDM signals
        };

        struct NrsCrsPowerOffsetNb
        {
            enum class Crs {
            dB_6,
            dB_4dot77,
            dB_3,
            dB_1dot77,
            dB0,
            dB1,
            dB1dot23,
            dB2,
            dB3,
            dB4,
            dB4dot23,
            dB5,
            dB6,
            dB7,
            dB8,
            dB9
            }crs;
        };
        struct SchedulingInfoListNb {
            uint32_t siTb;
        };

        struct SiWindowLengthNb
        {
            enum class Swl{
            ms160,
            ms320,
            ms480,
            ms640,
            ms960,
            ms1280,
            ms1600,
            spare1
            } swl;
        };

        struct SiRadioFrameOffsetNb {};
        struct SystemInfoValueTagListNb {};
        struct LateNonCriticalExtensionNb {};
        struct NonCriticalExtensionNb {};

        struct SystemInformationBlockType1Nb{
            std::bitset<8> hyperSfnMsb;
            CellAccessRelatedInfoNb cellAccessRelatedInfoNb;
            CellSelectionInfoNb cellSelectionInfo;
            PMaxNb pMax;
            FreqBandIndicatorNb freqBandIndicator;
            FreqBandInfoNb freqBandInfo;
            MultiBandInfoListNb multiBandInfoList;
            DownlinkBitmapNb downlinkBitMap;
            EutraControlRegionSizeNb eutraControlRegionSize;
            NrsCrsPowerOffsetNb nrsCrsPowerOffset;
            SchedulingInfoListNb schedulingInfoList;
            SiWindowLengthNb siWindowLength;
            SiRadioFrameOffsetNb siRadioFrameOffset;
            SystemInfoValueTagListNb systemInfoValueTagList;
            LateNonCriticalExtensionNb lateNonCriticalExtension;
            NonCriticalExtensionNb nonCriticalExtension;

        };

        struct PowerRampingParameters{
            enum class PowerRampingStep {
                dB0,
                dB2,
                dB4,
                dB6
            } powerRampingStep;
            enum class PreambleInitialReceivedTargetPower{
                dbm_120,
                dbm_118,
                dbm_116,
                dbm_114,
                dbm_112,
                dbm_110,
                dbm_108,
                dbm_106,
                dbm_104,
                dbm_102,
                dbm_100,
                dbm_98,
                dbm_96,
                dbm_94,
                dbm_92,
                dbm_90
            } preambleInitialReceivedTargetPower;
        };
        // TS 36.331 6.7.4 maxNPRACH-Resources-NB-r13 INTEGER ::= 3 -- Maximum number of NPRACH resources for NB-IoT 
        struct RachInfo{
            enum class RaResponseWindowSize{
                pp2,
                pp3,
                pp4,
                pp5,
                pp6,
                pp7,
                pp8,
                pp10
            } RaResponseWindowSize;
            enum class MacContentionResolutionTimer{
                pp1,
                pp2,
                pp3,
                pp4,
                pp8,
                pp16,
                pp32,
                pp64
            } macContentionResolutionTimer;

        };

        struct RachInfoList{
            RachInfo rachInfo1;
            RachInfo rachInfo2;
            RachInfo rachInfo3;
        };

        struct RachConfigCommon{
            uint8_t preambleTransMaxCE;
            PowerRampingParameters powerRampingParameters;
            RachInfoList rachInfoList;
            uint8_t connEstFailOffset;
        };

        struct BcchConfig{
            enum class ModificationPeriodCoeff{
                n16,
                n32,
                n64,
                n128
            } modificationPeriodCoeff;
        };
        struct PcchConfig{
            enum class DefaultPagingCycle{
                rf128,
                rf256,
                rf512,
                rf1024
            } defaultPagingCycle;
            enum class NB{
                fourT,
                twoT,
                oneT,
                halfT,
                quarterT,
                one8thT,
                one16thT,
                one32thT,
                one64thT,
                one128thT,
                one256thT,
                one512thT,
                one1024thT,
                nbspare3,
                nbspare2,
                nbspare1
            } nB;
            enum class NpdcchNumRepetitionPaging{
                r1,
                r2,
                r4,
                r8,
                r16,
                r32,
                r64,
                r128,
                r256,
                r512,
                r1024,
                r2048,
                spare4,
                spare3,
                spare2,
                spare1
            }npdcchNumRepetitionPaging;
        };

        struct NprachParametersNb{
           enum class CoverageEnhancementLevel{
               zero,
               one,
               two
           } coverageEnhancementLevel;
           enum class NprachPeriodicity{
               ms40,
               ms80,
               ms160,
               ms240,
               ms320,
               ms640,
               ms1280,
               ms2560
           } nprachPeriodicity; 
           enum class NprachStartTime{
               ms8,
               ms16,
               ms32,
               ms64,
               ms128,
               ms256,
               ms512,
               ms1024
           } nprachStartTime;
           enum class NprachSubcarrierOffset{
               n0,
               n12,
               n24,
               n36,
               n2,
               n18,
               n34,
               spare1
           } nprachSubcarrierOffset;
           enum class NprachNumSubcarriers{
               n12,
               n24,
               n36,
               n48
           } nprachNumSubcarriers;
           enum class NprachSubcarrierMsg3RangeStart{
               zero,
               oneThird,
               twoThird,
               one
           } nprachSubcarrierMsg3RangeStart;
           enum class MaxNumPreambleAttemptCE{
               n3,
               n4,
               n5,
               n6,
               n7,
               n8,
               n10,
               spare1
           } maxNumPreambleAttemptCE;
           enum class NumRepetitionsPerPreambleAttempt{
               n1,
               n2,
               n4,
               n8,
               n16,
               n32,
               n64,
               n128
           } numRepetitionsPerPreambleAttempt;
           enum class NpdcchNumRepetitionsRA{
               r1,
               r2,
               r4,
               r8,
               r16,
               r32,
               r64,
               r128,
               r256,
               r512,
               r1024,
               r2048,
               spare4,
               spare3,
               spare2,
               spare1
           } npdcchNumRepetitionsRA;
           enum class NpdcchStartSfCssRa{
               v1dot5,
               v2,
               v4,
               v8,
               v16,
               v32,
               v48,
               v64
           } npdcchStartSfCssRa;
           enum class NpdcchOffsetRa{
               zero,
               oneEighth,
               oneFourth,
               threeEighth
           } npdcchOffsetRa;
        };

        struct NprachParametersNbR14{
            NprachParametersNb::CoverageEnhancementLevel coverageEnhancementLevel;
            NprachParametersNb::NprachPeriodicity nprachPeriodicity; 
            NprachParametersNb::NprachStartTime nprachStartTime;
            NprachParametersNb::NprachSubcarrierOffset nprachSubcarrierOffset;
            NprachParametersNb::NprachNumSubcarriers nprachNumSubcarriers;
            NprachParametersNb::NprachSubcarrierMsg3RangeStart nprachSubcarrierMsg3RangeStart;
            NprachParametersNb::NpdcchNumRepetitionsRA npdcchNumRepetitionsRA;
            NprachParametersNb::NpdcchStartSfCssRa npdcchStartSfCssRa;
            NprachParametersNb::NpdcchOffsetRa npdcchOffsetRa;
            enum class NprachNumCbraStartSubcarriers{
               n8,
               n10,
               n11,
               n12,
               n20,
               n22,
               n23,
               n24,
               n32,
               n34,
               n35,
               n36,
               n40,
               n44,
               n46,
                n48
            } nprachNumCbraStartSubcarriers;
            int npdcchCarrierIndex;
        };
        struct NprachParametersList{
            NprachParametersNb nprachParametersNb0;
            NprachParametersNb nprachParametersNb1;
            NprachParametersNb nprachParametersNb2;
        };
        struct RsrpThresholdsPrachInfoList{
            int16_t ce1_lowerbound;
            int16_t ce2_lowerbound;
        };

        struct NprachConfig{
            enum NprachCpLength {
                us66dot7,
                us266dot7
            } nprachCpLength;
            RsrpThresholdsPrachInfoList rsrpThresholdsPrachInfoList;
            NprachParametersList nprachParametersList;   
        };
        struct NpdschConfigCommon{
            int8_t nrsPower;
        };
        struct NpuschConfigCommon{};
        struct DlGap{};
        struct UplinkPowerControlCommon{};
        struct NprachParametersListEdt{
            NprachParametersNbR14 nprachParametersNb0;
            NprachParametersNbR14 nprachParametersNb1;
            NprachParametersNbR14 nprachParametersNb2;
        };
        
        struct EdtTbsNb{
            bool edtSmallTbsEnabled;
            enum class EdtTbs{
                b328,
                b408,
                b504,
                b584,
                b680,
                b808,
                b936,
                b1000
            } edtTbs;
        };
        struct EdtTbsInfoList{
            EdtTbsNb edtTbsNb1;
            EdtTbsNb edtTbsNb2;
            EdtTbsNb edtTbsNb3;
        };
        struct NprachConfigR15{
            bool edtSmallTBSSubset = true;
            EdtTbsInfoList edtTbsInfoList;
            NprachParametersListEdt nprachParameterListEdt;
        };
        struct RadioResourceConfigCommonNb{
            RachConfigCommon rachConfigCommon;
            BcchConfig bcchConfig;
            PcchConfig pcchConfig;
            NprachConfig nprachConfig;
            NpdschConfigCommon npdschConfigCommon;
            NpuschConfigCommon npuschConfigCommon;
            DlGap dlGap;
            UplinkPowerControlCommon uplinkPowerControlCommon;
            NprachConfigR15 nprachConfigR15; // EDT

        };
        struct UeTimersAndConstantsNb{};
        struct FreqInfoNb{
            uint32_t ulCarrierFreq;
            uint32_t additionalSpectrumEmission;
        };
        struct TimeAlignmnentTimerCommonNb{};

        struct SystemInformationBlockType2Nb{
            RadioResourceConfigCommonNb radioResourceConfigCommon;
            UeTimersAndConstantsNb ueTimersAndConstants;
            FreqInfoNb freqInfo;
            TimeAlignmnentTimerCommonNb timeAlignmnentTimerCommon;
            MultiBandInfoListNb multiBandInfoList; 
            LateNonCriticalExtensionNb lateNonCriticalExtension;
            LateNonCriticalExtensionNb lateNonCriticalExtension1;
        };

        struct UlConfigListNbR15{

        };
        struct UlConfigCommonListNbR15{

        };

        struct UlConfigCommonNbR15{

        };

        struct NprachParametersListFmt2EdtNb{

        };

        struct SystemInformationBlockType23Nb{

        };

        /// SystemInformation structure
        struct SystemInformationNb
        {
            bool haveSib2; ///< have SIB2?
            SystemInformationBlockType2Nb sib2; ///< SIB2
            bool haveSib23;
            SystemInformationBlockType23Nb sib23; // EDT Settings
        };

        struct HarqAckResource{
            enum class TimeOffset{
                thirteen,
                fifteen,
                seventeen,
                eighteen
            } timeOffset;
            static uint8_t ConvertHarqTimeOffset2int (HarqAckResource::TimeOffset delay)
            {
                uint8_t res = 0;
                switch (delay)
                {
                case HarqAckResource::TimeOffset::thirteen:
                    res = 13;
                    break;
                case HarqAckResource::TimeOffset::fifteen:
                  res = 15;
                  break;
                case HarqAckResource::TimeOffset::seventeen:
                  res = 17;
                  break;
                case HarqAckResource::TimeOffset::eighteen:
                  res = 18;
                  break;
                default:
                    break;
                }
                return res;
            }
            enum class SubcarrierIndex {
                zero,
                one,
                two,
                three, 
                // only valid 3.75 khz 
                thirtyeight,
                thirtynine,
                forty,
                fortyone, 
                fortytwo,
                fortythree,
                fortyfour,
                fortyfive
            } subcarrierIndex;
        };

        // Liberg et al. p285
        struct DciN1{
            bool format = 1;
            bool npdcchOrderIndication;
            enum class NpdcchTimeOffset{
                ms0,
                ms4,// Only for R_max < 128
                ms8,// Only for R_max < 128
                ms12,// Only for R_max < 128
                ms16,
                ms32,
                ms64,
                ms128,
                ms256, // Only for R_max >= 128
                ms512, // Only for R_max >= 128
                ms1024 // Only for R_max >= 128
            } npdcchTimeOffset;
            enum class DciRepetitions{
               r1,
               r2,
               r4,
               r8,
               r16,
               r32,
               r64,
               r128,
               r256,
               r512,
               r1024,
               r2048,

            } dciRepetitions; 
            enum class NumNpdschSubframesPerRepetition{
                s1,
                s2,
                s3,
                s4,
                s5,
                s6,
                s8,
                s10
            } numNpdschSubframesPerRepetition;
            enum class NumNpdschRepetitions{
                r1,
                r2,
                r4,
                r8,
                r16,
                r32,
                r64,
                r128,
                r192,
                r256,
                r384,
                r512,
                r768,
                r1024,
                r1536,
                r2048
            } numNpdschRepetitions;
            enum class MCS{
                one,
                two,
                three,
                four,
                five,
                six,
                seven,
                eight,
                nine,
                ten,
                eleven,
                twelve
            } mCS;
            bool NDI;
            uint32_t tbs;
            HarqAckResource harqAckResource; 
            // Parameters to reduce simulation complexity
            std::vector<uint64_t> npdschOpportunity;
            std::vector<std::pair<uint64_t,std::vector<uint64_t>>> npuschOpportunity;
            std::vector<uint64_t> dciSubframes;
        };
        struct DciN0{
            bool format = 0;           
            uint8_t subframeIndication;
            enum class NpuschSchedulingDelay{
                ms8,
                ms16,
                ms32,
                ms64
            } npuschSchedulingDelay;
            enum class DciRepetitions{
               r1,
               r2,
               r4,
               r8,
               r16,
               r32,
               r64,
               r128,
               r256,
               r512,
               r1024,
               r2048,
            } dciRepetitions; 
            enum class NumResourceUnits{
                ru1,
                ru2,
                ru3,
                ru4,
                ru5,
                ru6,
                ru8,
                ru10
            } numResourceUnits;
            enum class NumNpuschRepetitions{
                r1,
                r2,
                r4,
                r8,
                r16,
                r32,
                r64,
                r128
            } numNpuschRepetitions;
            enum class MCS{
                one,
                two,
                three,
                four,
                five,
                six,
                seven,
                eight,
                nine,
                ten,
                eleven,
                twelve
            } mCS;
            bool redundandyVersion;
            bool NDI;
            uint32_t tbs;

            std::vector<std::pair<uint64_t,std::vector<uint64_t>>> npuschOpportunity;
            std::vector<uint64_t> dciSubframes;
        };

        struct UlGrant{
        bool Subcarrierspacing; // True = 15khz, false = 3.75khz
        
           
            // More inputs for 3.75 spacing or Multitone ETSI 136.213 16.5.1.1
            //currently limited to singletone 15 khz
        uint8_t subcarrierIndication;
        enum SchedulingDelay { //where NB-IoT DL subframe n is the last subframe in which the NPDSCH associated with the Narrowband Random Access Response Grant is transmitted 
        //16.5.1 with k0 = 12 for IDelay = 0
            ms12, // IDelay 0
            ms16, // IDelay 1 
            ms32, // IDelay 2  
            ms64  // IDelay 3
        } schedulingDelay;
        enum Msg3Repetitions{
        // 16.5.1.1
            r1,
            r2,
            r4,
            r8,
            r16,
            r32,
            r64,
            r128
        } msg3Repetitions;
        enum McsIndex {
            mcs0, // Number RUs = 4 | TBS = 88 bits 
            mcs1, // Number RUs = 2 | TBS = 88 bits 
            mcs2  // Number RUs = 1 | TBS = 88 bits 
        } mcsIndex;
        bool success;
        std::pair<uint8_t, std::vector<uint64_t>> subframes;
        uint64_t tbs_size;
        static uint8_t ConvertUlGrantSchedulingDelay2int (UlGrant::SchedulingDelay delay)
        {
            uint8_t res = 0;
            switch (delay)
            {
            case UlGrant::SchedulingDelay::ms12:
                res = 12;
                break;
            case UlGrant::SchedulingDelay::ms16:
                res = 16;
                break;
            case UlGrant::SchedulingDelay::ms32:
                res = 32;
                break;
            case UlGrant::SchedulingDelay::ms64:
                res = 64;
                break;
            default:
                break;
            }
            return res;
        }
        };
        
        struct RarPayload{
            uint16_t cellRnti;
            NbIotRrcSap::UlGrant ulGrant;
        }; 
        struct Rar

        {
            uint8_t rapId; ///< RAPID
            uint16_t cellRnti;
            //BuildRarListElement_s rarPayload; ///< RAR payload
            RarPayload rarPayload;
        };


        struct NpdcchMessage{
        enum SearchSpaceType{
            type1,
            type2,
            ueSpecific
        } searchSpaceType;
        enum NpdcchFormat {
            format1,
            format2
        } npdcchFormat;
        enum DciType{
            n0,
            n1,
            n2
        } dciType;
        NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel ce;
        NbIotRrcSap::DciN1 dciN1;
        NbIotRrcSap::DciN0 dciN0;
        std::vector<Rar> rars;
        std::vector<uint64_t> npdschOpportunity;
        std::vector<std::pair<uint64_t,std::vector<uint64_t>>> npuschOpportunity;
        std::vector<uint64_t> dciRepetitionsubframes;
        uint64_t ranti;
        uint64_t rnti;
        uint64_t tbs; // in bit
        bool isRar;
        uint64_t lcid;

        };

        struct RrcConnectionResumeRequestNb{
            uint32_t resumeIdentity; // 40 bits
            enum class EstablishmentCauseNb{
                mtAcces,
                moSignalling,
                moData,
                moExceptionData,
                delayTolerantAccess,
               // mtEdt // Not Release 13
            } establishmentCauseNb;

        };
        struct NpdcchConfigDedicatedNb{
            enum class NpdcchNumRepetitions{
                r1,
                r2,
                r4,
                r8,
                r16,
                r32,
                r64,
                r128,
                r256,
                r512,
                r1024,
                r2048,
                spare4,
                spare3,
                spare2,
                spare1
            } npdcchNumRepetitions;
            enum class NpdcchStartSfUss{
                v1dot5,
                v2,
                v4,
                v8,
                v16,
                v32,
                v48,
                v64
            } npdcchStartSfUss;
            enum class NpdcchOffsetUss{
                zero,
                oneEighth,
                oneFourth,
                threeEighth
            } npdcchOffsetUss;
        };
        struct RrcConnectionResumeNb{
            uint8_t rrcTransactionIdentifier;
            // rest is optional 
            //critical extensions possible
        };

        struct RrcConnectionResumeCompleteNb{
            uint8_t rrcTransactionIdentifier;
            Ptr<Packet> dedicatedInfoNas;

        };

        struct RrcConnectionReleaseNb{
            uint8_t rrcTransactionIdentifier;
            enum class ReleaseCauseNb{
                loadBalancingTAUrequired, 
                other, 
                rrc_Suspend,
                spare1
            } releaseCauseNb;
            uint64_t resumeIdentity;

        };
        static double ConvertNprachCpLenght2double (NprachConfig nprachconfig)
        {
            double res = 0;
            switch (nprachconfig.nprachCpLength)
            {
            case NprachConfig::NprachCpLength::us66dot7:
                res = 0.0667;
                break;

            case NprachConfig::NprachCpLength::us266dot7:
                res = 0.2666;
                break;
            default:
                break;
            }
            return res;
        }
        static uint16_t ConvertNumRepetitionsPerPreambleAttempt2int(NprachParametersNb nprachParametersNb){
            uint8_t res = 0;
            switch(nprachParametersNb.numRepetitionsPerPreambleAttempt){
                case NprachParametersNb::NumRepetitionsPerPreambleAttempt::n1:
                    res = 1;
                    break;
                case NprachParametersNb::NumRepetitionsPerPreambleAttempt::n2:
                    res = 2;
                    break;    
                case NprachParametersNb::NumRepetitionsPerPreambleAttempt::n4:
                    res = 4;
                    break;
                case NprachParametersNb::NumRepetitionsPerPreambleAttempt::n8:
                    res = 8;
                    break;
                case NprachParametersNb::NumRepetitionsPerPreambleAttempt::n16:
                    res = 16;
                    break;
                case NprachParametersNb::NumRepetitionsPerPreambleAttempt::n32:
                    res = 32;
                    break;
                case NprachParametersNb::NumRepetitionsPerPreambleAttempt::n64:
                    res = 64;
                    break;               
                case NprachParametersNb::NumRepetitionsPerPreambleAttempt::n128:
                    res = 128;
                    break;               
                default:
                    break;
             }
            return res;
        }
        static double ConvertChannelRasterOffsetNb2Double (ChannelRasterOffsetNb channelRasterOffset)
        {
            double res = 0;
            switch (channelRasterOffset.cr)
            {
            case ChannelRasterOffsetNb::khz_7dot5:
                res = -7.5;
                break;

            case ChannelRasterOffsetNb::khz_2dot5:
                res = -2.5;
                break;

            case ChannelRasterOffsetNb::khz2dot5:
                res = 2.5;
                break;

            case ChannelRasterOffsetNb::khz7dot5:
                res = 7.5;
                break;
            default:
                break;
            }
            return res;
        }
        static double ConvertSiWindowLength2Double (SiWindowLengthNb siWindowLength)
        {
            double res = 0;
            switch (siWindowLength.swl)
            {
            case SiWindowLengthNb::Swl::ms160:
                res = 160.0;
                break;

            case SiWindowLengthNb::Swl::ms320:
                res = 320.0;
                break;

            case SiWindowLengthNb::Swl::ms480:
                res = 480.0;
                break;

            case SiWindowLengthNb::Swl::ms640:
                res = 640.0;
                break;

            case SiWindowLengthNb::Swl::ms960:
                res = 960.0;
                break;

            case SiWindowLengthNb::Swl::ms1280:
                res = 1280.0;
                break;

            case SiWindowLengthNb::Swl::ms1600:
                res = 1600.0;
                break;

            default:
                break;
            }
            return res;
        }

        static uint8_t ConvertNprachNumSubcarriers2int (NprachParametersNb nprachParametersNb){
            uint8_t res = 0;
            switch(nprachParametersNb.nprachNumSubcarriers){
                case NprachParametersNb::NprachNumSubcarriers::n12:
                    res = 12;
                    break;
                case NprachParametersNb::NprachNumSubcarriers::n24:
                    res = 24;
                    break;    
                case NprachParametersNb::NprachNumSubcarriers::n36:
                    res = 36;
                    break;
                case NprachParametersNb::NprachNumSubcarriers::n48:
                    res = 48;
                    break;
                default:
                    break;
             }
            return res;
        }
        static uint16_t ConvertNprachPeriodicity2int (NprachParametersNb nprachParametersNb){
            uint16_t res = 0;
            switch(nprachParametersNb.nprachPeriodicity){
                case NprachParametersNb::NprachPeriodicity::ms40:
                    res = 40;
                    break;
                case NprachParametersNb::NprachPeriodicity::ms80:
                    res = 80;
                    break;    
                case NprachParametersNb::NprachPeriodicity::ms160:
                    res = 160;
                    break;
                case NprachParametersNb::NprachPeriodicity::ms240:
                    res = 240;
                    break;
                case NprachParametersNb::NprachPeriodicity::ms320:
                    res = 320;
                    break;
                case NprachParametersNb::NprachPeriodicity::ms640:
                    res = 640;
                    break;
                case NprachParametersNb::NprachPeriodicity::ms1280:
                    res = 1280;
                    break;
                case NprachParametersNb::NprachPeriodicity::ms2560:
                    res = 2560;
                    break;               
                default:
                    break;
             }
            return res;
        }
        static uint16_t ConvertMaxNumPreambleAttemptCE2int (NprachParametersNb nprachParametersNb){
            uint8_t res = 0;
            switch(nprachParametersNb.maxNumPreambleAttemptCE){
                case NprachParametersNb::MaxNumPreambleAttemptCE::n3:
                    res = 3;
                    break;
                case NprachParametersNb::MaxNumPreambleAttemptCE::n4:
                    res = 4;
                    break;    
                case NprachParametersNb::MaxNumPreambleAttemptCE::n5:
                    res = 5;
                    break;
                case NprachParametersNb::MaxNumPreambleAttemptCE::n6:
                    res = 6;
                    break;
                case NprachParametersNb::MaxNumPreambleAttemptCE::n7:
                    res = 7;
                    break;
                case NprachParametersNb::MaxNumPreambleAttemptCE::n8:
                    res = 8;
                    break;
                case NprachParametersNb::MaxNumPreambleAttemptCE::n10:
                    res = 10;
                    break;               
                default:
                    break;
             }
            return res;
        }
        static uint16_t ConvertNprachStartTime2int (NprachParametersNb nprachParametersNb){
            uint16_t res = 0;
            switch(nprachParametersNb.nprachStartTime){
                case NprachParametersNb::NprachStartTime::ms8:
                    res = 8;
                    break;
                case NprachParametersNb::NprachStartTime::ms16:
                    res = 16;
                    break;    
                case NprachParametersNb::NprachStartTime::ms32:
                    res = 32;
                    break;
                case NprachParametersNb::NprachStartTime::ms64:
                    res = 64;
                    break;
                case NprachParametersNb::NprachStartTime::ms128:
                    res = 128;
                    break;
                case NprachParametersNb::NprachStartTime::ms256:
                    res = 256;
                    break;
                case NprachParametersNb::NprachStartTime::ms512:
                    res = 512;
                    break;               
                case NprachParametersNb::NprachStartTime::ms1024:
                    res = 1024;
                    break;               
                default:
                    break;
             }
            return res;
        }
        static uint16_t ConvertNprachSubcarrierOffset2int (NprachParametersNb nprachParametersNb){
            uint8_t res = 0;
            switch(nprachParametersNb.nprachSubcarrierOffset){
                case NprachParametersNb::NprachSubcarrierOffset::n0:
                    res = 0;
                    break;
                case NprachParametersNb::NprachSubcarrierOffset::n2:
                    res = 2;
                    break;
                case NprachParametersNb::NprachSubcarrierOffset::n12:
                    res = 12;
                    break;
                case NprachParametersNb::NprachSubcarrierOffset::n18:
                    res = 18;
                    break;
                case NprachParametersNb::NprachSubcarrierOffset::n24:
                    res = 24;
                    break;
                case NprachParametersNb::NprachSubcarrierOffset::n34:
                    res = 34;
                    break;
                case NprachParametersNb::NprachSubcarrierOffset::n36:
                    res = 36;
                    break;
                default:
                    break;
             }
            return res;
        }
        static uint16_t ConvertNpdcchNumRepetitionsRa2int (NprachParametersNb nprachParametersNb){
            uint16_t res = 0;
            switch(nprachParametersNb.npdcchNumRepetitionsRA){
                case NprachParametersNb::NpdcchNumRepetitionsRA::r1:
                    res = 1;
                    break;
                case NprachParametersNb::NpdcchNumRepetitionsRA::r2:
                    res = 2;
                    break;    
                case NprachParametersNb::NpdcchNumRepetitionsRA::r4:
                    res = 4;
                    break;
                case NprachParametersNb::NpdcchNumRepetitionsRA::r8:
                    res = 8;
                    break;
                case NprachParametersNb::NpdcchNumRepetitionsRA::r16:
                    res = 16;
                    break;
                case NprachParametersNb::NpdcchNumRepetitionsRA::r32:
                    res = 32;
                    break;
                case NprachParametersNb::NpdcchNumRepetitionsRA::r64:
                    res = 64;
                    break;               
                case NprachParametersNb::NpdcchNumRepetitionsRA::r128:
                    res = 128;
                    break;               
                case NprachParametersNb::NpdcchNumRepetitionsRA::r256:
                    res = 256;
                    break;               
                case NprachParametersNb::NpdcchNumRepetitionsRA::r512:
                    res = 512;
                    break;               
                case NprachParametersNb::NpdcchNumRepetitionsRA::r1024:
                    res = 1024;
                    break;               
                case NprachParametersNb::NpdcchNumRepetitionsRA::r2048:
                    res = 2048;
                    break;               
                default:
                    break;
             }
            return res;
        }
        static double ConvertNpdcchStartSfCssRa2double (NprachParametersNb nprachParametersNb){
            double res = 0;
            switch(nprachParametersNb.npdcchStartSfCssRa){
                case NprachParametersNb::NpdcchStartSfCssRa::v1dot5:
                    res = 1.5;
                    break;
                case NprachParametersNb::NpdcchStartSfCssRa::v2:
                    res = 2;
                    break;
                case NprachParametersNb::NpdcchStartSfCssRa::v4:
                    res = 4;
                    break;
                case NprachParametersNb::NpdcchStartSfCssRa::v8:
                    res = 8;
                    break;
                case NprachParametersNb::NpdcchStartSfCssRa::v16:
                    res = 16;
                    break;
                case NprachParametersNb::NpdcchStartSfCssRa::v32:
                    res = 32;
                    break;
                case NprachParametersNb::NpdcchStartSfCssRa::v48:
                    res = 48;
                    break;
                case NprachParametersNb::NpdcchStartSfCssRa::v64:
                    res = 64;
                    break;              
                default:
                    break;
             }
            return res;
        }
        static double ConvertNpdcchOffsetRa2double (NprachParametersNb nprachParametersNb){
            double res = 0;
            switch(nprachParametersNb.npdcchOffsetRa){
                case NprachParametersNb::NpdcchOffsetRa::zero:
                    res = 0;
                    break;
                case NprachParametersNb::NpdcchOffsetRa::oneEighth:
                    res = 0.125;
                    break;
                case NprachParametersNb::NpdcchOffsetRa::oneFourth:
                    res = 0.25;
                    break;
                case NprachParametersNb::NpdcchOffsetRa::threeEighth:
                    res = 0.375;
                    break;
                default:
                    break;
             }
            return res;
        }
        static uint16_t ConvertNpdcchNumRepetitions2int (NpdcchConfigDedicatedNb configDedicated){
            uint16_t res = 0;
            switch(configDedicated.npdcchNumRepetitions){
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r1:
                    res = 1;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r2:
                    res = 2;
                    break;    
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r4:
                    res = 4;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r8:
                    res = 8;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r16:
                    res = 16;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r32:
                    res = 32;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r64:
                    res = 64;
                    break;               
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r128:
                    res = 128;
                    break;               
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r256:
                    res = 256;
                    break;               
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r512:
                    res = 512;
                    break;               
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r1024:
                    res = 1024;
                    break;               
                case NpdcchConfigDedicatedNb::NpdcchNumRepetitions::r2048:
                    res = 2048;
                    break;               
                default:
                    break;
             }
            return res;
        }
        static double ConvertNpdcchStartSfUss2double (NpdcchConfigDedicatedNb configDedicated){
            double res = 0;
            switch(configDedicated.npdcchStartSfUss){
                case NpdcchConfigDedicatedNb::NpdcchStartSfUss::v1dot5:
                    res = 1.5;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchStartSfUss::v2:
                    res = 2;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchStartSfUss::v4:
                    res = 4;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchStartSfUss::v8:
                    res = 8;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchStartSfUss::v16:
                    res = 16;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchStartSfUss::v32:
                    res = 32;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchStartSfUss::v48:
                    res = 48;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchStartSfUss::v64:
                    res = 64;
                    break;              
                default:
                    break;
             }
            return res;
        }
        static double ConvertNpdcchOffsetUss2double (NpdcchConfigDedicatedNb configDedicated){
            double res = 0;
            switch(configDedicated.npdcchOffsetUss){
                case NpdcchConfigDedicatedNb::NpdcchOffsetUss::zero:
                    res = 0;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchOffsetUss::oneEighth:
                    res = 0.125;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchOffsetUss::oneFourth:
                    res = 0.25;
                    break;
                case NpdcchConfigDedicatedNb::NpdcchOffsetUss::threeEighth:
                    res = 0.375;
                    break;
                default:
                    break;
             }
            return res;
        }
        static double ConvertNrsCrsPowerOffset2Double (NrsCrsPowerOffsetNb nrsCrsPowerOffset)
        {
            double res = 0;
            switch (nrsCrsPowerOffset.crs)
            {
            case NrsCrsPowerOffsetNb::Crs::dB_6:
                res = -6.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB_4dot77:
                res = -4.77;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB_3:
                res = -3.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB_1dot77:
                res = -1.77;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB0:
                res = 0.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB1:
                res = 1.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB1dot23:
                res = 1.23;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB2:
                res = 2.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB3:
                res = 3.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB4:
                res = 4.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB4dot23:
                res = 4.23;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB5:
                res = 5.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB6:
                res = 6.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB7:
                res = 7.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB8:
                res = 8.0;
                break;

            case NrsCrsPowerOffsetNb::Crs::dB9:
                res = 9.0;
                break;

            default:
                break;
            }
            return res;
        }
    static uint16_t ConvertNpdcchTimeOffset2int (DciN1 dci){
            uint16_t res = 0;
            switch(dci.npdcchTimeOffset){
                case DciN1::NpdcchTimeOffset::ms0:
                    res = 0;
                    break;
                case DciN1::NpdcchTimeOffset::ms4:
                    res = 4;
                    break;
                case DciN1::NpdcchTimeOffset::ms8:
                    res = 8;
                    break;
                case DciN1::NpdcchTimeOffset::ms12:
                    res = 12;
                    break;
                case DciN1::NpdcchTimeOffset::ms16:
                    res = 16;
                    break;
                case DciN1::NpdcchTimeOffset::ms32:
                    res = 32;
                    break;
                case DciN1::NpdcchTimeOffset::ms64:
                    res = 64;
                    break;
                case DciN1::NpdcchTimeOffset::ms128:
                    res = 128;
                    break;
                case DciN1::NpdcchTimeOffset::ms256:
                    res = 256;
                    break;
                case DciN1::NpdcchTimeOffset::ms512:
                    res = 512;
                    break;
                case DciN1::NpdcchTimeOffset::ms1024:
                    res = 1024;
                    break;
                default:
                    break;
             }
            return res;
        }
        static uint16_t ConvertDciN1Repetitions2int (DciN1 dci){
            uint16_t res = 0;
            switch(dci.dciRepetitions){
                case DciN1::DciRepetitions::r1:
                    res = 1;
                    break;
                case DciN1::DciRepetitions::r2:
                    res = 2;
                    break;
                case DciN1::DciRepetitions::r4:
                    res = 4;
                    break;
                case DciN1::DciRepetitions::r8:
                    res = 8;
                    break;
                case DciN1::DciRepetitions::r16:
                    res = 16;
                    break;
                case DciN1::DciRepetitions::r32:
                    res = 32;
                    break;
                case DciN1::DciRepetitions::r64:
                    res = 64;
                    break;
                case DciN1::DciRepetitions::r128:
                    res = 128;
                    break;
                case DciN1::DciRepetitions::r256:
                    res = 256;
                    break;
                case DciN1::DciRepetitions::r512:
                    res = 512;
                    break;
                case DciN1::DciRepetitions::r1024:
                    res = 1024;
                    break;
                case DciN1::DciRepetitions::r2048:
                    res = 2048;
                    break;
                default:
                    break;
             }
            return res;
        }
        static uint16_t ConvertNumNpdschRepetitions2int (DciN1 dci){
            uint16_t res = 0;
            switch(dci.numNpdschRepetitions){
                case DciN1::NumNpdschRepetitions::r1:
                    res = 1;
                    break;
                case DciN1::NumNpdschRepetitions::r2:
                    res = 2;
                    break;
                case DciN1::NumNpdschRepetitions::r4:
                    res = 4;
                    break;
                case DciN1::NumNpdschRepetitions::r8:
                    res = 8;
                    break;
                case DciN1::NumNpdschRepetitions::r16:
                    res = 16;
                    break;
                case DciN1::NumNpdschRepetitions::r32:
                    res = 32;
                    break;
                case DciN1::NumNpdschRepetitions::r64:
                    res = 64;
                    break;
                case DciN1::NumNpdschRepetitions::r128:
                    res = 128;
                    break;
                case DciN1::NumNpdschRepetitions::r192:
                    res = 192;
                    break;
                case DciN1::NumNpdschRepetitions::r256:
                    res = 256;
                    break;
                case DciN1::NumNpdschRepetitions::r384:
                    res = 384;
                    break;
                case DciN1::NumNpdschRepetitions::r512:
                    res = 512;
                    break;
                case DciN1::NumNpdschRepetitions::r768:
                    res = 768;
                    break;
                case DciN1::NumNpdschRepetitions::r1024:
                    res = 1024;
                    break;
                case DciN1::NumNpdschRepetitions::r1536:
                    res = 1536;
                    break;
                case DciN1::NumNpdschRepetitions::r2048:
                    res = 2048;
                    break;
                default:
                    break;
             }
            return res;
        }
    static uint8_t ConvertNumNpdschSubframesPerRepetition2int (DciN1 dci){
            uint8_t res = 0;
            switch(dci.numNpdschSubframesPerRepetition){
                case DciN1::NumNpdschSubframesPerRepetition::s1:
                    res = 1;
                    break;
                case DciN1::NumNpdschSubframesPerRepetition::s2:
                    res = 2;
                    break;
                case DciN1::NumNpdschSubframesPerRepetition::s3:
                    res = 3;
                    break;
                case DciN1::NumNpdschSubframesPerRepetition::s4:
                    res = 4;
                    break;
                case DciN1::NumNpdschSubframesPerRepetition::s5:
                    res = 5;
                    break;
                case DciN1::NumNpdschSubframesPerRepetition::s6:
                    res = 6;
                    break;
                case DciN1::NumNpdschSubframesPerRepetition::s8:
                    res = 8;
                    break;
                case DciN1::NumNpdschSubframesPerRepetition::s10:
                    res = 10;
                    break;
            
                default:
                    break;
             }
            return res;
        }
    static uint8_t ConvertRaResponseWindowSize2int(RachInfo rach){
            uint8_t res = 0;
            switch(rach.RaResponseWindowSize){
                case RachInfo::RaResponseWindowSize::pp2:
                    res = 2;
                    break;
                case RachInfo::RaResponseWindowSize::pp3:
                    res = 3;
                    break;
                case RachInfo::RaResponseWindowSize::pp4:
                    res = 4;
                    break;
                case RachInfo::RaResponseWindowSize::pp5:
                    res = 5;
                    break;
                case RachInfo::RaResponseWindowSize::pp6:
                    res = 6;
                    break;
                case RachInfo::RaResponseWindowSize::pp7:
                    res = 7;
                    break;
                case RachInfo::RaResponseWindowSize::pp8:
                    res = 8;
                    break;
                case RachInfo::RaResponseWindowSize::pp10:
                    res = 10;
                    break;
                default:
                    break;
             }
            return res;
        }

    static uint16_t ConvertDciN0Repetitions2int(DciN0 dci){
            uint16_t res = 0;
            switch(dci.dciRepetitions){
                case DciN0::DciRepetitions::r1:
                    res = 1;
                    break;
                case DciN0::DciRepetitions::r2:
                    res = 2;
                    break;
                case DciN0::DciRepetitions::r4:
                    res = 4;
                    break;
                case DciN0::DciRepetitions::r8:
                    res = 8;
                    break;
                case DciN0::DciRepetitions::r16:
                    res = 16;
                    break;
                case DciN0::DciRepetitions::r32:
                    res = 32;
                    break;
                case DciN0::DciRepetitions::r64:
                    res = 64;
                    break;
                case DciN0::DciRepetitions::r128:
                    res = 128;
                    break;
                case DciN0::DciRepetitions::r256:
                    res = 256;
                    break;
                case DciN0::DciRepetitions::r512:
                    res = 512;
                    break;
                case DciN0::DciRepetitions::r1024:
                    res = 1024;
                    break;
                case DciN0::DciRepetitions::r2048:
                    res = 2048;
                    break;
                default:
                    break;

             }
            return res;
        }
        static uint16_t ConvertNpuschSchedulingDelay2int (DciN0 dci){
            uint16_t res = 0;
            switch(dci.npuschSchedulingDelay){
                case DciN0::NpuschSchedulingDelay::ms8:
                    res = 8;
                    break;
                case DciN0::NpuschSchedulingDelay::ms16:
                    res = 16;
                    break;
                case DciN0::NpuschSchedulingDelay::ms32:
                    res = 32;
                    break;
                case DciN0::NpuschSchedulingDelay::ms64:
                    res = 64;
                    break;
                default:
                    break;
             }
            return res;
        }
        static uint16_t ConvertNumResourceUnits2int(DciN0 dci){
            uint16_t res = 0;
            switch(dci.numResourceUnits){
                case DciN0::NumResourceUnits::ru1:
                    res = 1;
                    break;
                case DciN0::NumResourceUnits::ru2:
                    res = 2;
                    break;
                case DciN0::NumResourceUnits::ru3:
                    res = 3;
                    break;
                case DciN0::NumResourceUnits::ru4:
                    res = 4;
                    break;
                case DciN0::NumResourceUnits::ru5:
                    res = 5;
                    break;
                case DciN0::NumResourceUnits::ru6:
                    res = 6;
                    break;
                case DciN0::NumResourceUnits::ru8:
                    res = 8;
                    break;
                case DciN0::NumResourceUnits::ru10:
                    res = 10;
                    break;
                default:
                    break;
            }
            return res;
        }
        static uint16_t ConvertNumNpuschRepetitions2int (DciN0 dci){
            uint16_t res = 0;
            switch(dci.numNpuschRepetitions){
                case DciN0::NumNpuschRepetitions::r1:
                    res = 1;
                    break;
                case DciN0::NumNpuschRepetitions::r2:
                    res = 2;
                    break;
                case DciN0::NumNpuschRepetitions::r4:
                    res = 4;
                    break;
                case DciN0::NumNpuschRepetitions::r8:
                    res = 8;
                    break;
                case DciN0::NumNpuschRepetitions::r16:
                    res = 16;
                    break;
                case DciN0::NumNpuschRepetitions::r32:
                    res = 32;
                    break;
                case DciN0::NumNpuschRepetitions::r64:
                    res = 64;
                    break;
                case DciN0::NumNpuschRepetitions::r128:
                    res = 128;
                    break;
                default:
                    break;
             }
            return res;
        }

 

};
}
#endif /* SRC_LTE_MODEL_NB_LTE_RRC_SAP_H_ */