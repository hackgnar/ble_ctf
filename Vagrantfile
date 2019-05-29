# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.network "forwarded_port", guest: 8888, host: 8889
  config.vm.box = "bento/ubuntu-16.04"
  config.vm.provider "virtualbox" do |vb|
    vb.cpus=2
    vb.memory=4096
    vb.customize ["modifyvm", :id, "--usb", "on"]
    vb.customize ["modifyvm", :id, "--usbehci", "on"]
    vb.customize ["usbfilter", "add", "0", 
      "--target", :id, 
      "--name", "ud100",
      "--vendorid","0x0a12",
      "--productid","0x0001"
      ]
    #vb.customize ["usbfilter", "add", "0", 
    #  "--target", :id, 
    #  "--name", "ubertooth",
    #  "--vendorid","0x1d50",
    #  "--productid","0x6002"
    #  ]
    #vb.customize ["usbfilter", "add", "0", 
    #  "--target", :id, 
    #  "--name", "ubertooth_bootloader",
    #  "--vendorid","0xffff",
    #  "--productid","0x0004"
    #  ]

    #vb.customize ["usbfilter", "add", "0", 
    #  "--target", :id, 
    #  "--name", "adafruit sniffer",
    #  "--vendorid","0x0403",
    #  "--productid","0x6015"
    #  ]
    #vb.customize ["usbfilter", "add", "0", 
    #  "--target", :id, 
    #  "--name", "redbearlabs_nrf5",
    #  "--vendorid","0x0d28",
    #  "--productid","0x0204"
    #  ]
    #vb.customize ["usbfilter", "add", "0", 
    #  "--target", :id, 
    #  "--name", "Jlink",
    #  "--vendorid","0x1366",
    #  "--productid","0x0101"
    #  ]

  end
  config.vm.provision "shell", path: "vagrant_bluetooth_dev_setup.sh"
  config.vm.synced_folder ".", "/vagrant", group: "staff", type: "rsync",
          rsync__exclude: [
            "build",
            ".git/objects",
            ".git/modules/third-party/objects"
          ]          
  end
