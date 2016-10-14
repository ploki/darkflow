---
lang: en
layout: docs
title: Configuration
permalink: /docs/configuration.en/
---

You'll find in this page information about how to configure DarkFlow to get the most of it. Make sure to check at least the [resources](#resources-preferences) and [path](#path-preferences) preferences in order to prevent from falling into nasty traps.

* [Resources preferences](#resources-preferences)
* [Pixels preferences](#pixels-preferences)
* [Path preferences](#path-preferences)
* [Logging preferences](#logging-preferences)
* [Style preferences](#style-preferences)

## Resources preferences

This settings tab tells Darkflow how and how much of your computer resources it will be allowed to use.

![Resources preferences](/img/preferences-resources.{{ page.lang }}.jpg)

### Memory managment

DarkFlow uses ImageMagick as a backing store for pixels. Here are the parameters of the ImageMagick architecture. For a detailed explanation, take a look at the [ImageMagick: Architecture](http://www.imagemagick.org/script/architecture.php) page.

*Highly recommanded values are indicated in the corresponding tables.*

#### Area

Maximum area in bytes of any one image that can reside in the pixel cache memory. If this limit is exceeded, the image is automagically cached to disk and optionally memory-mapped.

| **32-bit** | **64-bit** | **unit** |
|          1 |          1 |     GiB  |

#### Memory

Maximum amount of memory in bytes to allocate for the pixel cache from the heap.

| **32-bit** | **64-bit** | **unit** |
|          0 |          0 |     GiB  |

#### Map

Maximum amount of memory map in bytes to allocate for the pixel cache.

| **32-bit** | **64-bit**                | **unit** |
|          0 | 2 times your RAM, or more |     GiB  |

#### Disk

Maximum amount of disk space in bytes permitted for use by the pixel cache. If this limit is exceeded, the pixel cache is not created and a fatal exception is thrown.

| **32-bit** and **64-bit**                | **unit** |
|  The more, the better, but not less than the ```Map``` parameter |     GiB  |

*The ```Disk``` parameters includes the quantity set with the ```Map``` parameter, hence should be the greater of both.*

The difference between both parameter is the data access method:

* ```Map``` uses virtual memory and let the operating system manage data freshness on the behalf of the application. This access method ***permits parallelism*** in the image processing.
* ```Disk``` uses conventionnal file access to read and write image data to disk and will benefit of the buffer cache of your operating system. This access method ***does not permit parallelism*** in the image processing.


<div class="note warning">
  <h5>Memory and Map parameters on 32-bit systems</h5>
  <p>Be aware of that Darkflow will consume all the memory you permit
  the use and it will exhaust the address space of your 32-bit system
  pretty quickly. So, it is important to set these parameters to
  <strong>0</strong>!</p>
</div>

<div class="note warning">
  <h5>Memory parameter on 64-bit systems</h5>
  <p>Note that even on 64-bit systems, this parameter should be set to
  <strong>0</strong> because of the ImageMagick architecture that doesn't
  recycle memory used by already allocated images for newer instances. And
  keeping images stuck in memory is not really what you want. A better usage
  of this memory is to let your operating system manage it by setting the Map
  parameter to a higher value</p>
</div>

<div class="note">
  <h5>Prefer the Map parameter</h5>
  <p>In order to not waste memory and to have good parallelism in the image processing</p>
</div>


### Multi-threading

#### Workers

This is the number of operators allowed to run in parallel will playing the whole process. Operators are connected to each others and form some sort of dependency graph.

A safe default is ```1```. If your system has plenty of bandwidth you may increase this parameter to ```2```. A greater value may not give better performances.

#### ImageMagick Threads

This is the maximum number of threads that are permitted to run in parallel within the ImageMagick builtin operations.

*Should be set to the number of cores of your system, or the double if hyperthreading is enabled*

<div class="note warning">
  <h5>Do not exceed the default number of threads</h5>
  <p>There is a limitation in the way ImageMagick handles threading and DarkFlow
  will die stupidly if the thread number exceed the number of cores in your system.</p>
</div>

#### DarkFlow Threads

This is the maximum number of threads that are permitted to run in parallel within the DarkFlow builtin operations.

*Should be set to the number of cores of your system, or the double if hyperthreading is enabled*

<div class="note unreleased">
  <h5>Better default resources settings are coming soon!</h5>
  <p>DarkFlow will compute better values for these critical settings.</p>
</div>


## Pixels preferences

This settings tab is related to various pixels settings. *Default values are OK in most cases*.

![Pixels preferences](/img/preferences-pixels.{{ page.lang }}.jpg)

### Display target

This parameter sets the default rendering transfer function of the colors of linear space images.
This is used by the [Visualization window](/docs/visualization.{{ page.lang }}/) to set the initial value when and is used for all but non-linear pixel formats. It only affects on screen display and it does not affect how images are processed. The default ```sRGB``` value is probably what you what here and will render images in concordance with your perception of light.

### Incompatible scale

Some operators do not support (HDR color scaling)[/docs/philosophy.{{ page.lang}}/#hdr) and this parameter define the behavior of operator when facing an incompatible color scale.

* ```Ignore and convert``` the operator will convert silently the color scale of the image to something that it can handle.
* ```Warning and convert``` the operator will convert the color scale of the image to something that it can handle and it will issue a warning message in the [console](/docs/console.{{ page.lang }}/).
* ```Error``` the operator will refuse to process the image and it will issue an error message in the [console](/docs/console.{{ page.lang }}/). It will also mark the Image as being in error.

### Lab selection size

This parameter is used by the CIE LAB color selector. It defines the width and the height of the color gamut view. The greater, the slower to interact with.

*The CIE LAB color selector is used by the [Selective Lab Filter](/docs/cosmetic.{{ page.lang }}/#selective-lab-filter)*

## Path preferences

![Path preferences](/img/preferences-path.{{ page.lang }}.jpg)

### Base Directory

This directory defines the default place in your computer from where DarkFlow starts when the project does not define a base directory.

<div class="note info">
  <h5>Keep things all together</h5>
  <p>Give DarkFlow a default base folder, then create sub directories in this base folder for each project.
  And finally create sub directories within the project for your <em>lights</em>, <em>darks</em>, <em>flats</em> and <em>artifacts</em> photos. This will permit you to move your projects from a computer to another without pain</p>
</div>

### Temporary Directory

DarkFlow will use this directory to store transcient files. These files are actual pixels stored to vacate the memory of your computer. The amount per instance of DarkFlow will not exceed the [Disk](#disk) parameter.

<div class="note">
  <h5>Keep an eye on the temporary directory</h5>
  <p>You may want to put this directory within the base folder to check from time to time if leaks occured. In this case you can remove all the <code>magick-*</code> (<em>only if DarkFlow is not running</em>).</p>
</div>

## Logging preferences

This settings tab defines how DarkFlow logs messages and when the messages console is raised.

![Logging preferences](/img/preferences-logging.{{ page.lang }}.jpg)

### Log Level

This parameter permit to select the type of messages which are logged into the console. ```Debug``` being the more verbose option.

### Raise console on

This parameter tells DarkFlow to raise the console in the event of a message with a priority higher or equal to the value defined with this option.

### Trap on (debug)

**For developers only**. This permit to set break points in the code when a message of a certain class is issued. DarkFlow must run within a debugger, otherwise it will die abruptly.

## Style preferences

Want to change the appearence of DarkFlow? Here you can tune the colors of the interface and the workspace.

![Style preferences](/img/preferences-style.{{ page.lang }}.jpg)


### Workspace only

If this option is checked, the color theme will only be applied on the workspace. All other components of the program will inherit there style from the system.

### Colors

Color theme is defined with [Hex triplets](https://en.wikipedia.org/wiki/Web_colors#Hex_triplet), the same as used through the web and defined with the #RRGGBB syntax. Three-digit shorthand triplets like #RGB may be used.
