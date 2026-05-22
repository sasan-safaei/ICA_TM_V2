# how to install ...
   ``` bash

    sudo apt update
	sudo apt install cups printer-driver-brlaser printer-driver-ptouch libqrencode4 imagemagick

	sudo usermod -aG lpadmin $USER

	lpinfo -v  //find printer
	lpinfo -m | grep -i QL-700
	sudo lpadmin -p QL700 -E -v usb://Brother/QL-700?serial=000K4G297526 -m ptouch:0/ppd/ptouch-driver/Brother-QL-700-ptouch-ql.ppd
	sudo cupsenable QL700
	sudo cupsaccept QL700
	lpstat -p -d
	lpoptions -p QL700 -l
	
	sudo lpadmin -x QL700
	sudo lpadmin -p QL700 -E -v usb://Brother/QL-700?serial=000K4G297526 -P ~/ica_usv_test_resualt/tmp/QL700.ppd
	sudo systemctl restart cups
   ```




#   Temp command and  instructions
```
lpinfo -v  //find printer
lpinfo -m | grep -i QL-700
sudo lpadmin -p QL700 -E -v usb://Brother/QL-700?serial=000K4G297526 -m ptouch:0/ppd/ptouch-driver/Brother-QL-700-ptouch-ql.ppd
sudo cupsenable QL700
sudo cupsaccept QL700
lpstat -p -d
lpoptions -p QL700 -l
cancel jobID

PRINT EXAMPLE: 
lp -d QL700 -o PageSize=62x29mm ./tmp/resualt.jpg
echo "This is a direct print!" | lp -d QL700 -o PageSize=62x29mm
echo "62x10mm test label" | lp -d QL700 -o PageSize=Custom.62x10mm

echo "Short label" | lp -d QL700 -o PageSize=Custom.62x10mm -o AutoCut=True -o PrintQuality=Fast
echo "Zero margin" | lp -d QL700 -o PageSize=Custom.62x10mm -o ExtraMargin=0mm
echo "5mm margin" | lp -d QL700 -o PageSize=Custom.62x10mm -o ExtraMargin=5mm
lp -d QL700 -o PageSize= 62x16mm -o ExtraMargin=0mm ./tmp/resualt.jpg


ROTATE  jpg file:
sudo apt install imagemagick
convert resualt.jpg -rotate 90 resualtR.jpg

EDIT PPD file:

sudo nano /home/ica/sasan/ICA_USV_Test/tmp/QL700.ppd

*HWMargins: 0 0 0 0
*PageSize 12x38mm/12mm x 38mm (0.47" x 1.50"): "<</PageSize[34 107]/ImagingBBox null>>setpagedevice"
*PageRegion 12x38mm/12mm x 38mm (0.47" x 1.50"): "<</PageSize[34 107]/ImagingBBox null>>setpagedevice"
*ImageableArea 12x38mm/12mm x 38mm (0.47" x 1.50"): "0 0 29 107"
*PaperDimension 12x38mm/12mm x 38mm (0.47" x 1.50"): "34 107"

sudo lpadmin -x QL700
sudo lpadmin -p QL700 -E -v usb://Brother/QL-700?serial=000K4G297526 -P /home/ica/sasan/ICA_USV_Test/tmp/QL700.ppd
sudo systemctl restart cups

TEST PRINT: 
lp -d QL700 -o PageSize=12x38mm -o fit-to-page=off -o scaling=100 -o resolution=300x300dpi resualtRB3.jpg
```
