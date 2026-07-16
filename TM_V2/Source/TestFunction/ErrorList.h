#ifndef _ERRORLIST_H_
#define _ERRORLIST_H_
    struct ERROR {
        enum N {
            None=0
            ,Aufruesten=1
            , AufruestenOn
            , VCCIsLow
            , VCCIsOver
            , BoardConnection1
            , BoardConnection2
            , BoardConnection3
            //, TestDeviceConnection
            , ChargeDuration
            //, DisChargeDuration //5
            //, GPIO 
            , FlyBackEn
            , FlyBackdis
            , OutSwOff
            , waitToOutSwOff
            , EEPROMWrite
            , VCapsNotSame
            , VCapShutDownOutOfRange
            , TempSensor_IC
            , TempSensor_CapBank
            //, LoadConnection
            //, PSConnection
            //, Dongle
            , LabelPrintError
            //, StopJustForTest
            , LabPSNoAnswer
            , LabLoadNoAnswer
            , END
            ,I2CFailed
            ,uCProgramFailed
            ,ucUartFailed
            ,TM_Failed
        };
        std::string toString(N error) { 
            switch (error) {
                case 0: return "NoError";
                case Aufruesten: return "Aufruesten";
                case AufruestenOn: return "AufruestenOn";
                case VCCIsLow: return "VCCIsLow";
                case VCCIsOver: return "VCCIsOver";
                case BoardConnection1: return "BoardConnection1";
                case BoardConnection2: return "BoardConnection2";
                case BoardConnection3: return "BoardConnection3";
                case ChargeDuration: return "ChargeDuration";
                case FlyBackEn: return "FlyBackEn";
                case FlyBackdis: return "FlyBackdis";
                case OutSwOff: return "OutSwOff";
                case waitToOutSwOff: return "waitToOutSwOff";
                case EEPROMWrite: return "EEPROMWrite";
                case VCapsNotSame: return "VCapsNotSame";
                case VCapShutDownOutOfRange: return "VCapShutDownOutOfRange";
                case TempSensor_IC: return "TempSensor_IC";
                case TempSensor_CapBank: return "TempSensor_CapBank";
                case LabelPrintError: return "LabelPrintError";                
                case LabPSNoAnswer: return "LabPSNoAnswer";                
                case LabLoadNoAnswer: return "LabLoadNoAnswer";                
                case I2CFailed: return "I2CFailed";
                case uCProgramFailed: return "uCProgramFailed";
                case ucUartFailed: return "ucUartFailed";
                case TM_Failed: return "TM_Failed";
                default: return "UnknownError!!!";
            }
        }
    };

#endif
