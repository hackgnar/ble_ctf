SRC_DIR=/home/vagrant/src
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y git python-setuptools libbluetooth-dev build-essential libglib2.0-dev bluez bluez-tools cmake libusb-1.0-0-dev make gcc g++ libbluetooth-dev pkg-config libpcap-dev python-numpy tshark python-serial wireshark-dev gcc-arm-none-eabi libnewlib-arm-none-eabi

mkdir $SRC_DIR

cd $SRC_DIR
git clone https://github.com/hackgnar/bleah.git
git clone https://github.com/IanHarvey/bluepy.git
git clone https://github.com/greatscottgadgets/libbtbb.git
git clone https://github.com/greatscottgadgets/ubertooth.git
git clone https://github.com/adafruit/Adafruit_BLESniffer_Python.git
git clone https://github.com/ambrice/nordic_ble

cd $SRC_DIR/bluepy
python setup.py build
python setup.py install

cd $SRC_DIR/bleah
python setup.py build
python setup.py install

cd $SRC_DIR/libbtbb
mkdir build
cd build
cmake ..
make
make install
ldconfig

cd $SRC_DIR/ubertooth/host
mkdir build
cd build
cmake ..
make
make install
ldconfig

cd $SRC_DIR/nordic_ble
mkdir build
cd build
cmake ..
make
make install

chown -R vagrant:vagrant $SRC_DIR
