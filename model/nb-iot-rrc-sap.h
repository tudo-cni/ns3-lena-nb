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
            enum {
            N1,
            N2,
            N3
            }ecrs; // Number of OFDM signals
        };

        struct NrsCrsPowerOffsetNb
        {
            enum {
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
            enum
            {
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
            CellSelectionInfoNb CellSelectionInfo;
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

        struct RadioResourceConfigCommonNb{};
        struct UeTimersAndConstantsNb{};
        struct FreqInfoNb{};
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
            case SiWindowLengthNb::ms160:
                res = 160.0;
                break;

            case SiWindowLengthNb::ms320:
                res = 320.0;
                break;

            case SiWindowLengthNb::ms480:
                res = 480.0;
                break;

            case SiWindowLengthNb::ms640:
                res = 640.0;
                break;

            case SiWindowLengthNb::ms960:
                res = 960.0;
                break;

            case SiWindowLengthNb::ms1280:
                res = 1280.0;
                break;

            case SiWindowLengthNb::ms1600:
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
            case NrsCrsPowerOffsetNb::dB_6:
                res = -6.0;
                break;

            case NrsCrsPowerOffsetNb::dB_4dot77:
                res = -4.77;
                break;

            case NrsCrsPowerOffsetNb::dB_3:
                res = -3.0;
                break;

            case NrsCrsPowerOffsetNb::dB_1dot77:
                res = -1.77;
                break;

            case NrsCrsPowerOffsetNb::dB0:
                res = 0.0;
                break;

            case NrsCrsPowerOffsetNb::dB1:
                res = 1.0;
                break;

            case NrsCrsPowerOffsetNb::dB1dot23:
                res = 1.23;
                break;

            case NrsCrsPowerOffsetNb::dB2:
                res = 2.0;
                break;

            case NrsCrsPowerOffsetNb::dB3:
                res = 3.0;
                break;

            case NrsCrsPowerOffsetNb::dB4:
                res = 4.0;
                break;

            case NrsCrsPowerOffsetNb::dB4dot23:
                res = 4.23;
                break;

            case NrsCrsPowerOffsetNb::dB5:
                res = 5.0;
                break;

            case NrsCrsPowerOffsetNb::dB6:
                res = 6.0;
                break;

            case NrsCrsPowerOffsetNb::dB7:
                res = 7.0;
                break;

            case NrsCrsPowerOffsetNb::dB8:
                res = 8.0;
                break;

            case NrsCrsPowerOffsetNb::dB9:
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