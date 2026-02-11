#include "printFunc.h"
#include <cstdlib>

#include <string>
#include <iostream>

#ifdef PrinterIncluded

    #include "ica-logo.h"
    
    //#define QR_SIZE 250 //256   // QR code image size
    #define MARGIN 2 //10     // Margin for better visibility
    #define PRINTER_NAME "EOS2"
    #define THRESHOLD 128 //128  // Adjust threshold if needed (0-255)


    // Function to send text to the printer
    void __print_text(const char *text) {
        char command[256];
        snprintf(command, sizeof(command), "echo \"%s\" | lp -d %s", text, PRINTER_NAME);
        system(command);
    }
    // Function to generate and print a QR code
    void __print_qrcode(const char *data) {
        QRcode *qrcode = QRcode_encodeString(data, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
        if (!qrcode) {
            fprintf(stderr, "Failed to generate QR code\n");
            return;
        }

        FILE *fp = fopen("./tmp/qrcode.pbm", "wb");
        if (!fp) {
            fprintf(stderr, "Failed to open file for writing\n");
            QRcode_free(qrcode);
            return;
        }

        fprintf(fp, "P1\n%d %d\n", qrcode->width, qrcode->width);
        for (int y = 0; y < qrcode->width; y++) {
            for (int x = 0; x < qrcode->width; x++) {
                fputc(qrcode->data[y * qrcode->width + x] & 1 ? '0' : '1', fp);
                fputc(' ', fp);
            }
            fputc('\n', fp);
        }
        fclose(fp);
        QRcode_free(qrcode);

        // Print the QR code
        char command[256];
        snprintf(command, sizeof(command), "lp -d %s ./tmp/qrcode.pbm", PRINTER_NAME);
        system(command);
    }

    // Function to generate and save QR code as JPEG
    gdImagePtr save_qrcode_jpeg(int _qrCodeSize, const char *data) {
        QRcode *qrcode = QRcode_encodeString(data, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
        if (!qrcode) {
            fprintf(stderr, "Failed to generate QR code\n");
            return NULL;
        }

        // Create a GD image
        int img_size = _qrCodeSize + 2;// * MARGIN;
        gdImagePtr img = gdImageCreate(img_size, img_size + 0);//40  // Extra space for text

        int white = gdImageColorAllocate(img, 255, 255, 255);
        int black = gdImageColorAllocate(img, 0, 0, 0);

        // Fill the background with white
        gdImageFilledRectangle(img, 0, 0, img_size, img_size + 0, white);//+40

        // Draw the QR code
        for (int y = 0; y < qrcode->width; y++) {
            for (int x = 0; x < qrcode->width; x++) {
                if (qrcode->data[y * qrcode->width + x] & 1) {
                    gdImageFilledRectangle(img,
                        MARGIN + x * (_qrCodeSize / qrcode->width),
                        MARGIN + y * (_qrCodeSize / qrcode->width),
                        MARGIN + (x + 1) * (_qrCodeSize / qrcode->width),
                        MARGIN + (y + 1) * (_qrCodeSize / qrcode->width),
                        black);
                }
            }
        }

        //FILE *fp = fopen(OUTPUT_FILE, "wb");
        //if (fp) {
        //    gdImageJpeg(img, fp, 100);
        //    fclose(fp);
        //    printf("QR code saved to %s\n", OUTPUT_FILE);
        //} else {
        //    fprintf(stderr, "Failed to save image\n");
        //}
        // Cleanup
        //gdImageDestroy(img);
        QRcode_free(qrcode);
        return img;
    }
    void qrcode_jpeg_output(const char* qrCode_data, const char* text1, const char* text2, const char* text3){
        
        int img_width = 378;//720;//1265;
        int img_height = 120;//250;//300;
        gdImagePtr qr = save_qrcode_jpeg(img_height,qrCode_data);

        //, const char* logo_path, const char* output_path) {
        gdImagePtr img = NULL;//logo = NULL, qr = NULL
        FILE *fp;
        int white, black;
        // Create base image
        img = gdImageCreateTrueColor(img_width, img_height);
        white = gdImageColorAllocate(img, 255, 255, 255);
        black = gdImageColorAllocate(img, 0, 0, 0);
        gdImageFilledRectangle(img, 0, 0, img_width, img_height, white);
        // Load logo
        //fp = fopen("./ica-logo.jpg", "rb");
        //if (!fp) { perror("Logo open failed"); return; }
        //logo = gdImageCreateFromJpeg(fp);
        //fclose(fp);
        // Load QR code
        //fp = fopen(OUTPUT_FILE, "rb");
        //if (!fp) { perror("QR open failed"); return; }
        //qr = gdImageCreateFromJpeg(fp);
        //fclose(fp);
        gdImagePtr logo = gdImageCreateFromJpegPtr(logo_jpg_len, logo_jpg);
        if (!logo) {
        printf("Failed to load logo image from memory\n");
            return;
        }
        // Calculate positions
        int logo_x = MARGIN;
        int logo_y = MARGIN;
        float logo_resize = .40;
        int logo_W =(int) (gdImageSX(logo)* logo_resize);
        int logo_H =  (int) (gdImageSY(logo)*logo_resize);
        int qr_x = img_width - gdImageSX(qr)+10;
        int qr_y = 0;
        gdImageCopyResampled(img, logo, logo_x, logo_y, 0, 0, logo_W,logo_H, gdImageSX(logo), gdImageSY(logo));
        
        // Draw middle text (use scalable TTF font)
        //const char* font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";  // change if needed
        const char* font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf";  // change if needed
        int brect[8];
        int fontsize = 16; //50;
        int _yy=fontsize+logo_y+10;
        gdImageStringFT(img, brect, black, font_path, fontsize, 0, logo_x+logo_W+ 7, _yy, text1);
        gdImageStringFT(img, brect, black, font_path, fontsize, 0, logo_x+logo_W+ 7, _yy+35, text2);
        gdImageStringFT(img, brect, black, font_path, fontsize+4 , 0, MARGIN, _yy+80, text3);
        // Save output
        
        int width = gdImageSX(img);
        int height = gdImageSY(img);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int color = gdImageGetPixel(img, x, y);
                int gray = (gdImageRed(img, color) + gdImageGreen(img, color) + gdImageBlue(img, color)) / 3;
                //gdImageGray(img, color);  // Works if image is grayscale
                gdImageSetPixel(img,x,y,gray < THRESHOLD ? black : white);
            }
        }

        gdImageCopy(img, qr, qr_x, qr_y, 0, 0, gdImageSX(qr), gdImageSY(qr));

        fp = fopen("./tmp/resualt.jpg", "wb");
        if (!fp) { perror("Output open failed"); return; }
        gdImageJpeg(img, fp, 100);//-1);
        fclose(fp);

        // Free
        gdImageDestroy(logo);
        gdImageDestroy(qr);
        gdImageDestroy(img);
        
    }

    int QL700_Print() {
        //const char* file = "./tmp/resualt.jpg";
        //const char* printer = "QL700"; // Use exact name from `lpstat -p`
        //const char* options = "-o PageSize=Custom.62x16mm -o ExtraMargin=0mm";
        // Print using lp
        //std::string cmd = "lp -d " + std::string(printer) + " "+ std::string(options) + " " + std::string(file);
        //int result = std::system(cmd.c_str());
        std::system("convert ./tmp/resualt.jpg -rotate 90 ./tmp/resualtR.jpg");
        int resualt = std::system("lp -d QL700 -o PageSize=12x38mm -o fit-to-page=off -o scaling=100 -o resolution=300x300dpi ./tmp/resualtR.jpg");    
        //std::cout << cmd.c_str()<<"\n";
        if (resualt == 0) {
            std::cout << "Print job sent successfully!\n";
        } else {
            std::cerr << "Failed to send print job.\n";
        }

        return resualt;
    }
    void removeJPG_PFiles_Jobs(){
        std::system("cancel -a");
        std::system("rm ./tmp/resualt.jpg");
        std::system("rm ./tmp/resualtR.jpg");
        std::system("rm ./tmp/error.jpg");
        std::system("rm ./tmp/errorR.jpg");
    }
    /*void printErrorLabel(std::string _str){
        char __strOut[64];
        sprintf(__strOut,"echo \"%s\" | lp -d QL700 -o PageSize=12x38mm",_str.c_str());
        std::system(__strOut);
    }
    */
    int printErrorLabel(std::string text1, std::string text2, std::string text3){
        
        int img_width = 378;
        int img_height = 120;
        
        gdImagePtr img = NULL;
        FILE *fp;
        int white, black;

        // Create base image
        img = gdImageCreateTrueColor(img_width, img_height);
        white = gdImageColorAllocate(img, 255, 255, 255);
        black = gdImageColorAllocate(img, 0, 0, 0);
        gdImageFilledRectangle(img, 0, 0, img_width, img_height, white);
        
        const char* font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf";  // change if needed
        int brect[8];
        int fontsize = 32; //50;
        gdImageStringFT(img, brect, black, font_path, 32, 0, 5, 52, text1.c_str());
        gdImageStringFT(img, brect, black, font_path, 20, 0, 5, 82, text2.c_str());
        gdImageStringFT(img, brect, black, font_path, 20, 0, 5, 110, text3.c_str());
        
        int width = gdImageSX(img);
        int height = gdImageSY(img);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int color = gdImageGetPixel(img, x, y);
                int gray = (gdImageRed(img, color) + gdImageGreen(img, color) + gdImageBlue(img, color)) / 3;
                gdImageSetPixel(img,x,y,gray < THRESHOLD ? black : white);
            }
        }

        fp = fopen("./tmp/error.jpg", "wb");
        if (!fp) { perror("Output open failed"); return -1; }
        gdImageJpeg(img, fp, 100);//-1);
        fclose(fp);

        // Free
        gdImageDestroy(img);
        
        std::system("convert ./tmp/error.jpg -rotate 90 ./tmp/errorR.jpg");
        int resualt = std::system("lp -d QL700 -o PageSize=12x38mm -o fit-to-page=off -o scaling=100 -o resolution=300x300dpi ./tmp/errorR.jpg");    
        if (resualt == 0) {
            std::cout << "Print job sent successfully!\n";
        } else {
            std::cerr << "Failed to send print job.\n";
        }
        return resualt;
    }
    
    #else
    void qrcode_jpeg_output(const char* qrCode_data, const char* text1, const char* text2, const char* text3){
        printf(" printFunc.qrCode_jpeg_output func is not active \n");        
    }
    int QL700_Print(){
        printf(" printFunc.QL700_Print func is not active \n");
        return -1;
    }
    void removeJPG_PFiles_Jobs(){
        printf(" printFunc.removeJPG_PFiles_Jobs func is not active \n");
    }
    int printErrorLabel(std::string text1,std::string text2){
        printf(" printFunc.printErrorLabel func is not active \n");
        return -1;
    }
#endif