# lcd-daemon

[![lcd-daemon C/C++ CI](https://github.com/mbhangui/lcd-daemon/actions/workflows/lcd-daemon-c-cpp.yml/badge.svg)](https://github.com/mbhangui/lcd-daemon/actions/workflows/lcd-daemon-c-cpp.yml)

lcd-daemon package is a package consisting of lcdDaemon and pilcd. lcdDaemon reads a local named pipe (FIFO) and reads data from pilcd and prints text supplied by pilcd on a LCD display. It works only for the Hitachi 4480 controller based 16x2 and 20x4 displays.

lcd-daemon uses libwiringpi and libwiringpidev from https://github.com/mbhangui/wiringPi. The original Author of Wiring Pi is Gordon Henderson. Wiring Pi was what made me love the Raspberry Pis.

The wiring scheme used by lcdDaemon is as below. If you wire it differently you don't have to rebuild the package. Instead you can set environment variables PIN\_RS, PIN\_EN, PIN\_D0 to PIN\_D7.

Pin Name|WiringPI Number|RPI Board Number|LCD Pin Number
--------|--------|---------|------------------------------
-|-|-|01 Ground 
-|-|-|02 +5v 
-|-|-|03 Contrast Adjustment 
PIN\_RS|6|22|04 Register Select 
-|-|-|05 Read Write Pin (connected to Ground)
PIN\_EN|5|18|06 Enable Pulse
PIN\_D0|-|-|07 data pin 0 Not connected
PIN\_D1|-|-|08 data pin 1 Not connected
PIN\_D2|-|-|09 data pin 2 Not connected
PIN\_D3|-|-|10 data pin 3 Not connected
PIN\_D4|4|16|11 data pin 4
PIN\_D5|0|11|12 data pin 5
PIN\_D6|2|13|13 data pin 6
PIN\_D7|3|15|14 data pin 7
-|-|-|15 Anode of backlight LED
-|-|-|16 Cathode of backlight LED

lcdDaemon reads a named pipe in line mode and expects the line to be in a simple format white space arguments as detailed below.

<u>rownum</u> <u>scroll</u> <u>clear</u>:<u>message</u>

where
 * rownum - value can be 0 or 1 for 16x2 display or 0, 1, 2 or 3 for 20x4 display.
 * scroll - value can be 0 or 1. 1 turns on scrolling
 * clear - The following values are supported for \fIclear\fR
   1. clear the screen
   2. clear and initialize
   3. initializing LCD display
   4. clear screen without displaying text
   5. clear and initialize screen without displaying text
   6. initialize screen without displaying text

The <b>pilcd</b> is one client that writes to the named pipe in the above format. You can use any program/script to display text on the LCD display by using the above format. Expanding this further you can also have a simple server application that listens on a socket and write to the named pipe using the same format and thus allow the LCD display to be accessed from the network. Here is one such example that uses <b>nc</b> client from the <b>netcat</b> package to send a message over UDP to be displayed on the LCD screen.

First let's write a simple script <u>/usr/bin/recvlcd</u> that just reads descriptor 0 and writes the line to /run/lcd-daemon/lcdfifo

**Script /usr/libexec/lcd-daemon/recvlcd**

```
#!/bin/sh
# run lcdDaemon in the background on /run/lcd-daemon/lcdfifo 
# mkdir -p /run/lcd-daemon
# lcdDaemon -f /run/lcd-daemon -b 4 -c 20 -r 4 &
# run nc command
# server: nc -u --exec /usr/libexec/lcd-daemon/recvlcd -l 7777 --keep-open
# or
# server: nc -k -u -l 7777 > /run/lcd-daemon/lcdfifo
# client: echo testing | nc -u localhost 7777
read line
if [ -d /run ] ; then
	echo "$line" > /run/lcd-daemon/lcdfifo
elif [ -d /var/run ] ; then
	echo "$line" > /var/run/lcd-daemon/lcdfifo
else
	echo "$line" > /tmp/lcd-daemon/lcdfifo
fi
```

Now on the server which has the LCD display connected, let us run <b>lcdDaemon</b> and a simple script that will receive packets over UDP.

**Start lcdDaemon and run a UDP server on port 7777**

```
Start the lcdDaemon

$ sudo /usr/sbin/lcdDaemon -b 4 -c 20 -r 4 &

Send message locally using picd on the first row

$ pilcd -R 0 Hi from Localhost

Send message locally using echo command on the second row

$ echo  1 0 0:Hi from local > /run/lcd-daemon/lcdfifo

Use the nc command to listen over port 7777 and pass descriptor 0 to the script /usr/bin/recvlcd. The form below can be used for nc that support the -exec argument

$ nc -u -exec /usr/libexec/lcd-daemon/bin/recvlcd -l 7777 --keep-open

Use the nc command to listen over port 7777 and redirect output to /run/lcd-daemon/lcdfifo. The form below can be used for nc that do not support the -exec argument

$ nc -k -u -l 7777 > /run/lcd-daemon/lcdfifo
```

**Send text to be displayed on LCD**

We now have a LCD display to which messages can be sent over UDP to port 7777 from any host from the network. We will use the <b>nc</b> command to send the message.

```
Scroll message to a remote server 192.168.2.101 having the LCD display on row 3

$ echo 2 1 0:Hi from overseas | nc -u 192.168.2.101 7777
```

# Installation

## Source Installation

Before building lcd-daemon, you need to build and install libwiringpi and libwiringpidev from https://github.com/mbhangui/libwiringPi.

```
$ cd /usr/local/src
$ git clone --no-tags --no-recurse-submodules --depth=1 https://github.com/mbhangui/lcd-daemon
$ cd lcd-daemon
$ ./default.configure
$ make && sudo make install-strip
```

## Create RPM/Debian packages

This is done by running the create\_rpm / create\_debian scripts. (Here `version` refers to the existing version of lcd-daemon package)

### Create RPM package

```
To create the RPM package execute create_rpm script
$ pwd
/usr/local/src/lcd-daemon
$ ./create_rpm
$ ls -l $HOME/rpmbuild/RPMS/x86_64/*lcd-daemon*
-rw-r--r--. 1 pi pi 36584 Jun 19 17:41 /home/pi/rpmbuild/RPMS/x86_64/lcd-daemon-1.0-1.1.fc38.x86_64.rpm
-rw-r--r--. 1 pi pi 40876 Jun 19 17:40 /home/pi/rpmbuild/RPMS/x86_64/lcd-daemon-debuginfo-1.0-1.1.fc38.x86_64.rpm
-rw-r--r--. 1 pi pi 80848 Jun 19 17:40 /home/pi/rpmbuild/RPMS/x86_64/lcd-daemon-debugsource-1.0-1.1.fc38.x86_64.rpm

To install use the rpm command

$ sudo rpm -ivh /home/pi/rpmbuild/RPMS/x86_64/lcd-daemon-1.0-1.1.fc38.x86_64.rpm

```

### Create debian package

```
To create the debian package execute create_debian script
$ pwd
/usr/local/src/lcd-daemon
$ ./create_debian
$ ls -l $HOME/stage
total 44
drwxr-xr-x  3 pi pi 4096 Jun 19 18:35 .
drwxr-xr-x 24 pi pi 4096 Jun 19 18:24 ..
-rw-r--r--  1 pi pi 6059 Jun 19 18:35 lcd-daemon.0-1.1_arm64.buildinfo
-rw-r--r--  1 pi pi 2552 Jun 19 18:35 lcd-daemon.0-1.1_arm64.changes
-rw-r--r--  1 pi pi 2344 Jun 19 18:35 lcd-daemon.0-1.1_arm64.deb

To install use the debian command

$ sudo dpkg -i /home/pi/stage/lcd-daemon-1.0-1.1_arm64.deb
```

## Running lcdDaemon

You can run lcdDaemon on the command line or put it in rc.local or create a systemd script or use your own favourite method. I use [daemontools](https://cr.yp.to/daemontools.html) and use [svscan](https://github.com/mbhangui/indimail-mta/wiki/svscan.8). The RPM/debian installation creates a service for lcdDaemon as <u>/service/lcd-daemon</u>. My fork of Dan's daemontools package is [here](https://github.com/mbhangui/indimail-mta/tree/master/daemontools-x). svscan runs [supervise](https://github.com/mbhangui/indimail-mta/wiki/supervise.8) which can run lcdDaemon continuously. If you have built lcdDaemon using the source, you can run the <u>create\_service</u> to create the service. Run the <u>create\_service</u> command to create a service for lcdDeamon in <u>/service/lcd-daemon</u>. Once you install the daemontools package, lcdDaemon will start automatically on boot. You can manually start or stop using the [svc](https://github.com/mbhangui/indimail-mta/wiki/svc.8) command.

**Start lcdDaemon**

```
$ sudo svc -u /service/lcd-daemon
```

**Stop lcdDaemon**

```
$ sudo svc -d /service/lcd-daemon
```

To install daemontools package run the command. This will require you to install the OBS repo. See the next chapter for details.

```
$ sudo apt-get update && sudo apt-get install daemontools
```

**TODO**

lcdDaemon creates a fifo <u>/tmp/lcdfifo</u>. The location will change in the next release to <u>/run/lcd-daemon/lcdfifo</u>. Currently the permission of this file is <b>0644</b> owned by root. If you want any user to be able to print message on the lcd display, change the permission to <b>0666</b>.

Next release will also allow the pilcd command to write message to the LCD display from anywhere in the network. I will be using my fork of the [ucspi-tcp](http://cr.yp.to/ucspi-tcp.html) to implement this feature. My fork of Dan's ucspi-tcp package is [here](https://github.com/mbhangui/indimail-mta/tree/master/ucspi-tcp-x).

I need to upload the man pages for lcdDaemon and pilcd here. Will do it sometime later.

## Prebuilt Binaries

[![build result](https://build.opensuse.org/projects/home:mbhangui:raspi/packages/lcd-daemon/badge.svg?type=default)](https://build.opensuse.org/package/show/home:mbhangui:raspi/lcd-daemon)

Prebuilt binaries using openSUSE Build Service are available for lcd-daemon for

* Debian 10 (including ARM images for Debian 10 which work (and tested) for RaspberryPI (model 2,3))
* Raspbian 10 and 11 for RaspberryPI (arm and aarch64 images)
* Ubuntu 20, 22, 23 (arm and aarch64 images)

Use the below url for installation

[download](https://software.opensuse.org//download.html?project=home%3Ambhangui%3Araspi&package=lcd-daemon)

The above instructions will help you add the OBS repo in /etc/apt/sources.list.d for debian systems or /etc/yum.repos.d for rpm based systems. Once you have the repo can can install using the apt-get or the yum/dnf command

**For debian based systems**

```
$ sudo apt-get update && sudo apt-get install lcd-daemon
```

**For RPM based systems**

```
$ sudo dnf update && sudo dnf install lcd-daemon
```

## IMPORTANT NOTE for binary builds on debian

You don't need to read this if you are going to have your own method to run lcdDaemon on startup. But if you want to use daemontools to run lcdDaemon you need to read this. debian/ubuntu repositories already has daemontools which is far behind in terms of feature list than the daemontools repo provides. When you install daemontools, apt-get may pull the wrong version with limited features. Also `apt-get install daemontools` or `apt-get install daemontools` will get installed with errors, leading to an incomplete setup. You need to ensure that the two packages get installed from the daemontools repository instead of the official debian repository.

All you need to do is set a higher preference for the daemontools repository by creating /etc/apt/preferences.d/preferences with the following contents

```
$ sudo /bin/bash
# cat > /etc/apt/preferences.d/preferences <<EOF
Package: *
Pin: origin download.opensuse.org
Pin-Priority: 1001
EOF
```

You can verify this by doing

```
$ apt policy daemontools ucspi-tcp
daemontools:
  Installed: 2.11-1.1+1.1
  Candidate: 2.11-1.1+1.1
  Version table:
     1:0.76-7 500
        500 http://raspbian.raspberrypi.org/raspbian buster/main armhf Packages
 *** 2.11-1.1+1.1 1001
       1001 http://download.opensuse.org/repositories/home:/indimail/Debian_10  Packages
        100 /var/lib/dpkg/status
ucspi-tcp:
  Installed: 2.11-1.1+1.1
  Candidate: 2.11-1.1+1.1
  Version table:
     1:0.88-6 500
        500 http://raspbian.raspberrypi.org/raspbian buster/main armhf Packages
 *** 2.11-1.1+1.1 1001
       1001 http://download.opensuse.org/repositories/home:/indimail/Debian_10/ Packages
        100 /var/lib/dpkg/status
```

# Support

Feel free to create an [issue](https://github.com/mbhangui/lcd-daemon/issues/new/choose) on github. I'm also available on [matrix](https://matrix.to/#/#Manvendra:matrix.org) where you can leave a message if you have any issue.
