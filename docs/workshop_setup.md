# Workshop Setup

## TLDR
Ultimately, what you need is a Linux machine with a Bluetooth 4.0 module/dongle and `gatttool` installed (via bluez tools). Each attendee will be supplied with a Bluetooth device which they will connect to via their Linux machine.  These devices will take attendees though a series of exercises.  For attendees new to Bluetooth, they will be supplied with exercises for beginners.  For attendees experienced with Bluetooth, they will be supplied with exercises which are much more challenging =)

## Testing your Setup
As mentioned above, you can do almost all exercises for this workshop with the standard `gatttool` application. If you can run the following without error, then you are good to go.

First make sure your main Bluetooth interface is up.
```
hciconfig hci0 up
hciconfig -a
```

Scan for some Bluetooth low energy devices.  Press Ctr-c to stop.
```
hcitool lescan
```

Try connecting to a few of the mac addresses from the output of the above command with the following command.  Make sure to replace the sample mac address 11:22:33:44:55:66 with one from your results from above.  Also note, not all devices are connectable.  However, if you get at least one to work then your setup is ready to roll.
```
gatttool -b 11:22:33:44:55:66 --characteristics
```

If you can do all of the above, you are ready to go for the workshop.  If you want to go above and beyond with your setup, check out the "Optional Tool Setup" section at the bottom of this document.

## System setup

### Raspberry Pi
By far, the easiest way to get the hardware and software setup for this workshop is to install stock Raspbian on a Raspberry Pi with Bluetooth (Zero W, 3 A+, 3 B+, 4, etc).  A default install of Raspbian on one of these devices gives you everything you need out of the box.  The only complex task with this setup is "how you will access the machine" during the workshop.  This can be done remotely via:
- direct access to the pi via keyboard and monitor.  This method will require you to bring your own equipment =)
- connecting your laptop and to the ras pi over a usb network and accessing it via ssh.  Instructions on doing this can be found here: https://raspberrypi.stackexchange.com/questions/66431/headless-pi-zero-ssh-access-over-usb
- connecting your laptop and the ras pi to the same wifi network and accessing the ras pi via ssh over wifi.  Wifi will be available during the workshop so this is an option.  If you go this route, please test it before the workshop.  Instructions for doing so can be found here: https://styxit.com/2017/03/14/headless-raspberry-setup.html . For this setup, I use the following wpa config file:
boot/wpa_supplicant.conf
```
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US

network={
        ssid="SSID_NAME"
        psk="PASSWORD"
        key_mgmt=WPA-PSK
}

```
and I enable ssh with:
```
touch boot/ssh.txt
```

### Direct Linux Install
Most standard Linux distributions (Ubuntu, Fedora, etc) will work for this workshop out of the box. At most, you will likely just have to install bluez-tools.  For distros such as Kali Linux, you will have to do some extra steps to make your Bluetooth device accessible.  For Kali, the steps are as follows:

```rfkill unblock all```

```btmgmt le on```

```hciconfig hci0 up```

### Linux via Vagrant
This repository has a vagrant file with all the tools you need complete the workshop.  The only setup you will have to do is to install the VirtualBox usb extension [here](https://www.virtualbox.org/wiki/Downloads).  Then you must also find the vendor id and product id of your bluetooth module/dongle and enter it into the filter section of the vagrant file.  In Linux, just run `lsusb`.  In OSX you can get this info from the Apple Menu -> About System -> Devices -> Bluetooth.  Your vendor/product id will be available there.  For Windows, I have no clue, but I’m sure its simple to find.

Feel free to just swap out the vendor id and product id from this section of the vagrant file with your info:
```
vb.customize ["usbfilter", "add", "0", 
      "--target", :id, 
      "--name", "ud100",
      "--vendorid","0x0a12",
      "--productid","0x0001"
      ]

```

### Linux Running on a VM (VMware, Virtualbox, etc)
This should work just fine with most VM software.  Just remember to do USB/Device pass-through on the VM settings so your guest VM will have access to your Bluetooth device.

## Alternative Systems

### Iphone/Android with NFRConnect
You can do a large portion of the BLE_CTF exercise with the Nordic NRFConnect app on a cell phone.  While this is a possible solution, I wouldn't attempt it unless you are very patient =)

### Windows
The workshop can probably be done with some existing tools or scripting code libs in Windows, but I wouldn't recommend it =)

### OSX
The workshop can be done with various code libraries in OSX.  Keep in mind it will be a lot more challenging and you likely wont get much assistance in the workshop if you get stuck.

## Optional Tool Setup
You don't need any of the following tools to complete the workshop.  Many of them will make exercises easier (Bleah & Bettercap)  and some will just provide you with experience for more advanced Bluetooth interaction (Ubertooth & NRF Sniffer).

### Bleah
The original project is now depricated, but my fork of it can be found here:
https://github.com/hackgnar/bleah

### Ghetto Gatttool Enumeration Script
Running the following gatttool/bash script will get you output much like Bleah.  Its nice in case you have issues installing Bleah.
https://gist.github.com/hackgnar/c9fd9bbf5a96fdd0d43f9f3a8c4e7aeb

### Bettercap
The Bettercap project has some bluetooth support.  Much like Bleah, its useful for visualizing GATT characteristics.
https://www.bettercap.org/

### Wireshark/Tshark
Useful for reading advertisement data payloads and outputs from Ubertooth of NRF Sniffer.  I personally prefer tshark.

### Ubertooth
Not needed for the class, but the exercises provide a great opportunity to learn how to sniff traffic with an ubertooth hardware device.
https://github.com/greatscottgadgets/ubertooth/

### NRF Sniffer
Not needed for the class but the exercises provide a great opportunity to learn how to sniff traffic with an NFR Sniffer
https://github.com/NordicSemiconductor/nRF-Sniffer-for-802.15.4

## Common Problems
The following are the most common problems people attending my Bluetooth workshops have had:
- Not having a Bluetooth Low Energy capable device.  Some participants use older laptops where the onboard Bluetooth devices is not a Bluetooth 4.0 module.  Please check before the workshop.  If you have this problem, you can easily use a USB Bluetooth 4.0 device with this type of setup.
- Not being able to bring up Bluetooth devices in Kali Linux.  This is typically because of the rfkill default config in Kali.  The above documentation on this should resolve the issue.
