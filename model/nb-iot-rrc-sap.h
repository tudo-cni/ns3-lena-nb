#ifndef NBIOT_RRC_SAP_H
#define NBIOT_RRC_SAP_H

#include <stdint.h>
#include <list>
#include <bitset>

#include <ns3/ptr.h>
#include <ns3/simulator.h>


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
           enum class nprachSubcarrierMsg3RangeStart{
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
        struct NprachParametersList{
            NprachParametersNb nprachParametersNb0;
            NprachParametersNb nprachParametersNb1;
            NprachParametersNb nprachParametersNb2;
        };
        struct RsrpThresholdsPrachInfoList{
            int16_t ce1_lowerbound;
            int16_t ce2_lowerbound;
            int16_t ce3_lowerbound;
        };

        struct NprachConfig{
            enum {
                us66dot7,
                us266dot7
            } nprachCpLength;
            RsrpThresholdsPrachInfoList raspThresholdsPrachInfoList;
            NprachParametersList nprachParametersList;   
        };
        struct NpdschConfigCommon{
            int8_t nrsPower;
        };
        struct NpuschConfigCommon{};
        struct DlGap{};
        struct UplinkPowerControlCommon{};
        struct RadioResourceConfigCommonNb{
            RachConfigCommon rachConfigCommon;
            BcchConfig bcchConfig;
            PcchConfig pcchConfig;
            NprachConfig nprachConfig;
            NpdschConfigCommon npdschConfigCommon;
            NpuschConfigCommon npuschConfigCommon;
            DlGap dlGap;
            UplinkPowerControlCommon uplinkPowerControlCommon;
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

        /// SystemInformation structure
        struct SystemInformationNb
        {
            bool haveSib2; ///< have SIB2?
            SystemInformationBlockType2Nb sib2; ///< SIB2
        };

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


};
}
#endif /* SRC_LTE_MODEL_NB_LTE_RRC_SAP_H_ */