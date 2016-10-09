---
lang: en
layout: docs
title: Installation
permalink: /docs/installation.en/
---

## DarkFlow on MS Windows

## DarkFlow on OS X/macOS

## DarkFlow on GNU/Linux

Installation process on GNU/Linux needs your user to be a sudoer. If it is not already the case, you can fix it by issuing the following command.

```bash
$ su -c "adduser $(id -un) sudo"
```

And then, logout and login again, not only from your terminal but also from your graphical user environment. This step is needed by the way group additions works on GNU/Linux.

### Ubuntu and Debian

Currently supported distributions
 - Debian stretch amd64 and i386
 - Ubuntu xenial amd64 and i386
 - Ubuntu wily amd64

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
