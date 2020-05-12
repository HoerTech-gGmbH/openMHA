#!/bin/bash

usb_interface=$(dmesg | grep from.usb0 | cut -d: -f2 | cut -d" " -f2 | tail -1)
if test -z "$usb_interface"
then if ip a | grep -q usb0:
     then usb_interface=usb0
     fi
fi

internet_interface=$(route | grep ^default | tr ' ' '\n' | tail -1)
internet_ip=$(ifconfig $internet_interface | grep inet | awk '{print $2}' \
                | head -1)

echo This skript routes network traffic from usb interface: \"$usb_interface\"
echo to internet using the default network interface: \"$internet_interface\".
echo If this looks correct, press ENTER, else Control-c.
read
echo This skript will modify the Beaglebone Black and this PC:
echo - BBB: Install ssh key, change DNS, apt sources, network routing
echo - PC: Change NetworkManager config, restart NetworkManager
echo - PC: Install packages net-tools and sshpass
echo - PC: Modify ssh\'s known_hosts file
echo - PC: Add iptables rules to forward the BBB network traffic
echo If you want to allow this, press ENTER, else Control-c.
read

if [ -f /etc/NetworkManager/conf.d/BBB.conf ]
then
    echo File /etc/NetworkManager/conf.d/BBB.conf exists, leaving unmodified.
else
    sudo tee /etc/NetworkManager/conf.d/BBB.conf <<EOF
# Beaglebone Black USB network interface should not be managed by NetworkManager
[device-bbb]
match-device=interface-name:en*u*
managed=false
[device-usb]
match-device=interface-name:usb0*
managed=false
[device-enx]
match-device=interface-name:enx*
managed=false
EOF
    echo File /etc/NetworkManager/conf.d/BBB.conf created.
    echo Restarting NetworkManager...
    sudo systemctl restart NetworkManager
    echo Restarting NetworkManager may not be sufficient.  Reboot this PC if
    echo this skript does not work properly. For now press ENTER to continue.
    read
fi

echo Install net-tools for ifconfig and route, sshpass for ssh to beaglebone.
sudo apt install net-tools sshpass

echo set ip address of network interface
sudo ifconfig $usb_interface 192.168.7.1

echo remove ssh host key of last BBB from known_hosts
ssh-keygen -f $HOME/.ssh/known_hosts -R 192.168.7.2

echo enable ssh to BBB with ssh key
sshpass -ptoor ssh root@192.168.7.2 -o StrictHostKeyChecking=no \
        "mkdir -p .ssh && chmod 700 .ssh && cat >>.ssh/authorized_keys" \
        <$HOME/.ssh/id_rsa.pub

echo Set time on the beaglebone
ssh root@192.168.7.2 "date --set '$(date -R)'"

echo Connecting to beaglebone and overwriting resolv.conf
ssh root@192.168.7.2 "rm /etc/resolv.conf && tee /etc/resolv.conf" <<EOF
nameserver 192.168.99.30
nameserver 8.8.8.8
nameserver 134.106.49.2
EOF

echo Writing debian sources list for apt
ssh root@192.168.7.2 "tee /etc/apt/sources.list.d/debian.list" <<EOF
deb http://deb.debian.org/debian buster main contrib non-free
deb http://deb.debian.org/debian-security/ buster/updates main contrib non-free
deb http://deb.debian.org/debian buster-updates main contrib non-free
EOF
ssh root@192.168.7.2 "tee /etc/apt/sources.list.d/hoertech.list" <<EOF
deb http://aptdev.hoertech.de/ bionic universe
EOF

echo "set default route on BBB over usb0 using PC as gateway"
ssh root@192.168.7.2 "ip route add default via 192.168.7.1"

echo add NAT to network packages from BBB
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward
sudo iptables --table nat -A POSTROUTING -o $internet_interface -s 192.168.7.2 -j MASQUERADE
sudo iptables -A FORWARD -o $internet_interface -i $usb_interface -j ACCEPT
sudo iptables -A FORWARD -i $internet_interface -o $usb_interface -m state --state=ESTABLISHED,RELATED -j ACCEPT

ssh root@192.168.7.2 "apt-key adv --keyserver keyserver.ubuntu.com --recv-keys B7D6CDF547DA4ABD; apt update"
