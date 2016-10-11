---
lang: en
layout: docs
title: Installation
permalink: /docs/installation.en/
---


Follow the installation instructions for your paltform:

* [Installation on MS Windows](#darkflow-on-ms-windows)
* [Installation on OS X / macOS](#darkflow-on-os-xmacos)
* [Installation on GNU/Linux](#darkflow-on-gnulinux)

Then proceed to the [configuration](/docs/configuration.{{ page.lang }})

## DarkFlow on MS Windows

Follow this simple procedure:

* Double click the ```setup-darkflow-YYYY.MM.DD-x64.exe``` to execute it.
* authorize the setup program to modify the system by clicking the ```Yes``` button
* Choose the path where to install DarkFlow (default should be fine), then click ```Next```
* check the box if you want a desktop shortcut, then click ```Next```
* Review the installation parameters and click ```Install```

Things get weird at this point, but don't worry. The DarkFlow installer contains the ImageMagick installer which is launched on this step. So another Installation program is launched over the first one and asks for another set of questions:

* Accept the license Agreement (which tells you that it is free software) and click ```Next```
* You may read the ```Welcome to ImageMagick!``` message and click ```Next```
* Choose the path where to install ImageMagick (default should be fine). then click ```Next```
* Choose a Start Menu Folder (default should be fine). then click ```Next```
* Make sure the ```Add Application directory to your system path``` **is checked** and 
click ```Next```
* Proceed by clicking ```Install```
* You just installed ImageMagick, so ckick ```Next```
* Choose to read the ImageMagick's index.html or not and click ```Finish```

At this point, the ImageMagic installer exits and give back the hand to the DarkFlow installer.

* Click ```Finish``` and you're done!


Execute the setup program and answer to the asked questions regarding the path

## DarkFlow on macOS

Open the DMG disk image file and drag-n-drop the DarkFlow App into your
Applications Folder. Since the App is not signed, you will need to answer some
cryptic question in order to be able to launch DarkFlow.
[This support page](https://support.apple.com/kb/PH21769) may help you in case
of difficulties to launch the program.

## DarkFlow on GNU/Linux

Installation process on GNU/Linux needs your user to be a sudoer. If it is not already the case, you can fix it by issuing the following command.

```bash
$ su -c "adduser $(id -un) sudo"
```

And then, logout and login again, not only from your terminal but also from your graphical user environment. This step is needed by the way group additions works on GNU/Linux.

### Ubuntu and Debian

Currently supported distributions

* Debian stretch amd64 and i386
* Ubuntu xenial amd64 and i386
* Ubuntu wily amd64

```bash
$ sudo apt-get install lsb-release
$ echo deb http://darkflow.org/$(lsb_release -si).$(dpkg --print-architecture)/ $(lsb_release -sc) main | sudo tee /etc/apt/sources.list.d/darkflow.list
$ sudo apt-get update
$ sudo apt-get install darkflow
```

### Other distros using docker

If your distro is not listed in the previous section, you're not out of luck. There is a handy way to get DarkFlow Up and Running using [docker](https://www.docker.com/).

The following command will pull the required image from [docker hub](https://hub.docker.com/r/ggim/darkflow/) and run a container.

All you need is to expose your photos to the docker container running DarkFlow, this is done by copying your assets in the ```~/DARKFLOW/``` directory.

```bash
$ curl -s [http://darkflow.org/live.sh](http://darkflow.org/live.sh) | bash
```

<div class="note">
  <h5>"In God we trust. All other we audit."</h5>
  <p>Feel free to review this script as <a href="http://darkflow.org/live.sh">live.sh</a> sudo's operations and open your display to local connections</p>
</div>

<div class="note warning">
  <h5>Running DarkFlow in a container</h5>
  <p>From within the container, save all your data in the ~/DARKFLOW/ directory, otherwise you will lose data for good.</p>
</div>
