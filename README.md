# TTPoE-Tap

Node-01
```
$ git clone https://github.com/teslamotors/ttpoe.git
$ cd ttpoe
$ make all
$ ls -l ./modttpoe/modttpoe.ko
$ sudo ip link add link enp0s2 name vl100 type vlan id 100
$ sudo ip link set dev vl100 address 98:ed:5c:00:00:01
$ sudo ip link set dev vl100 up
$ ip link show vl100
$ sudo insmod ./modttpoe/modttpoe.ko dev=vl100 verbose=2
$ echo 2 | sudo tee /sys/module/modttpoe/parameters/target
```

Node-02
```
$ git clone https://github.com/teslamotors/ttpoe.git
$ cd ttpoe
$ make all
$ ls -l ./modttpoe/modttpoe.ko
$ sudo ip link add link enp0s2 name vl100 type vlan id 100
$ sudo ip link set dev vl100 address 98:ed:5c:00:00:02
$ sudo ip link set dev vl100 up
$ ip link show vl100
$ sudo insmod ./modttpoe/modttpoe.ko dev=vl100 verbose=2
$ echo 1 | sudo tee /sys/module/modttpoe/parameters/target
```
