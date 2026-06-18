echo "Step1:--cmp"
./STM32ProgFunc --cmp ./Firmware_Folder/ICA2510V1.1_STM32.bin 
echo "Step2: Flash"
./STM32ProgFunc ./Firmware_Folder/ICA2510V1.1_STM32.bin 
echo "Step3:--cmp"
./STM32ProgFunc --cmp ./Firmware_Folder/ICA2510V1.1_STM32.bin 
echo "Step4:--ob"
./STM32ProgFunc --write-ob 0xDEFFE1AA
echo "Step5:--reset"
./STM32ProgFunc --reset
echo "Step6:--uart str"
./STM32ProgFunc --uart str "55 56 52 50"
echo "Step7:--uart str"
./STM32ProgFunc --uart str "53 A0 01 F8 53 A1 08 50"