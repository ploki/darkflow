---
lang: en
layout: docs
title: Configuration
permalink: /docs/configuration.en/
---

## Resources preferences

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

![Pixels preferences](/img/preferences-pixels.{{ page.lang }}.jpg)

## Path preferences

![Path preferences](/img/preferences-path.{{ page.lang }}.jpg)

## Logging preferences

![Logging preferences](/img/preferences-logging.{{ page.lang }}.jpg)

## Style preferences

![Style preferences](/img/preferences-style.{{ page.lang }}.jpg)
