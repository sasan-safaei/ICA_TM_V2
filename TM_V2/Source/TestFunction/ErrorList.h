#ifndef _ERRORLIST_H_
#define _ERRORLIST_H_
    struct ERROR {
        enum N {
            Aufruesten=1
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
        };
    };

#endif
