# ESPCAM
Linux driver for [ESP32CAM](https://github.com/intaklunik/esp32cam) device.

## Project structure
```
espcam
|
|  Makefile
|  espcam_overlay.dts
|  espcam_main.c - entry point; implements the regmap and I2C driver, registers the SPI interface
|  espcam.h - defines the context structure
|  espcam_spi.c - implements the SPI driver and registers the video driver
|  espcam_sysfs.c - implements the sysfs interface
|  regs.h - defines device register addresses
|
|__video
   | espcam_video.h - defines structures for the video driver
   | espcam_video.c - implements the video driver
   | espcam_vb.c - implements vb2 operations
   | espcam_ioctl.c - implements V4L2 ioctl operations
   | espcam_ctrl.c - implements V4L2 control interface
```

## How to use
### Load
1. Compile the Device Tree overlay file:
```
dtc -@ -I dts -O dtb -o espcam-overlay.dtbo espcam_overlay.dts
```
2. Add the Device Tree overlay to the system:
```
sudo cp espcam-overlay.dtbo /boot/overlays
```
3. To enable the overlay add this line to **/boot/firmware/config.txt**:
```
dtoverlay=espcam-overlay
```
4. Reboot:
```
sudo reboot
```
5. Compile the module:
```
make
```
6. Load the module:
```
sudo insmod espcam.ko
```

### I2C
Scan i2c-1 for connected devices:
```
i2cdetect -y 1
```
**0x11** cell in the output table should contain "0x11" if the device is found. (Or "UU" if 0x11 address is currently in use by a driver.)

### Sysfs
To see the device custom sysfs attributes:
```
cd /sys/bus/i2c/devices/1-0011/
ls
```
In the directory should be 2 folders: **app** and **camera**, which represent the device logical modules.  
- /app contains application properties and configs: id, version, camera_stream_status.  
- /camera contains camera settings: contrast, saturation, brightness.  

1. Read a sysfs attribute:
```
cd /sys/bus/i2c/devices/1-0011/module_name/
cat attribute_name
```
2. Write to a sysfs attribute:
```
cd /sys/bus/i2c/devices/1-0011/module_name/
echo value | sudo tee attribute_name
```
Where *value* is an integer.


