
// ======================================================================== 
// ICA Group
//
// Walter-Welp Str. 25-27
// 44149 Dortmund
//
// +49-231-22248-0
//
// ========================================================================
// global function
// ========================================================================
// Project   ICA2315
// Filename  main.h
// Author Sasan Safaei                                                                   
// Date      2025-03-27
// ======================================================================== 


//sudo apt update
//sudo apt install cups
//sudo usermod -aG lpadmin $USER
//sudo apt install printer-driver-brlaser
//sudo systemctl restart cups
//sudo apt install cups imagemagick

//sudo systemctl enable --now cups
//lpadmin -p QL700 -E -v "usb://Brother/QL-700?serial=000K4G297526" -m raw
//lpoptions -d QL700

//sudo apt install python3-pip
//pip3 install brother_ql


#ifndef _ICA_PRINTFUNC_H_
#define _ICA_PRINTFUNC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#define PrinterIncluded

#ifdef PrinterIncluded
    #include <qrencode.h>
    #include <gd.h>
    #include <gdfonts.h>
    
    //#define QR_SIZE 256   // QR code image size
    //#define MARGIN 0 //10     // Margin for better visibility
    //#define PRINTER_NAME "EOS2"
    //#define THRESHOLD 128  // Adjust threshold if needed (0-255)
    // Function to send text to the printer
    void __print_text(const char *text);
    // Function to generate and print a QR code
    void __print_qrcode(const char *data);
    // Function to generate and save QR code as JPEG
    gdImagePtr save_qrcode_jpeg(const char *data);  
    #endif
    void qrcode_jpeg_output(const char* qrCode_data, const char* text1, const char* text2, const char* text3);
    int QL700_Print();
    void removeJPG_PFiles_Jobs();    
    int printErrorLabel(std::string text1, std::string text2, std::string text3);
#endif // _ICA_PRINTFUNC_H_
