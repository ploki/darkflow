---
lang: en
layout: docs
title: Philosophy
permalink: /docs/philosophy.en/
---


DarkFlow looks like an interactive diagram editor until you open a visualization window. Here is the processing with DarkFlow of the Messier 8 region from very good LRGB source images. [Image Acquisition by Jim Misti](http://www.mistisoftware.com/astronomy/index_fits.htm).

*Don't pay attention to the jerks and jolts as they are caused by the screencast program and the small resolution used for acquisition purpose.*
<div class="videoWrapper">
    <iframe width="420" height="315" src="https://www.youtube.com/embed/N8G3rAnTi_g" frameborder="0" allowfullscreen></iframe>
</div>

## What DarkFlow aims to be

 - DarkFlow aims to be an all in one comprehensive solution for astrophotography processing.
 - DarkFlow aims to offer fundamental operators that let you craft methods we never envisionned.
 - DarkFlow aims to be intuitive and easy to use. It tries to not disturb your flow of ideas.

## What DarkFlow is

 - DarkFlow is about creating pipelines of image processing operators.
 - DarkFlow creates new images from your original photos by applying operators during the process.
 - DarkFlow applies global and reproducible operations on images.

## What DarkFlow is not

 - DarkFlow is not a program that requires you to manage intermediate renderings or temporary files.
 - DarkFlow is not an image editor that modifies your images.
 - DarkFlow is not a Raster Graphics Editor with brushes nor a freehand drawing tool.

## DarkFlow is Project oriented

You are encouraged to work iteratively on the processing of your images. Save the project, and go back to it once your eyes are rested. Things are better built gradually.

You can even use [git](https://en.wikipedia.org/wiki/Git) (in association with [git-lfs](https://git-lfs.github.com/) to store your raw photos) to track all the versions of your image processing project and host it for free on [GitHUB](https://github.com). Take a look at the [NGC7023](https://github.com/ploki/NGC7023) repository.

### Non destructive approach

DarkFlow doesn't modify your source material. It's left untouched while your projects evolve with your skills.

### Project artifacts

As you wish, you can generate output images from your projects. It depends on whether you added or not one or many ```Save``` operators in your workflow.

## Efficient

By its design, DarkFlow will leverage every single bit of potency from your computer.

### Parallelism

Algorithms implementation behind each operator have been developped to deliver maximum parallelism. Thanks to *[Grand Central Dispatch](https://en.wikipedia.org/wiki/Grand_Central_Dispatch)* on macOS and to *[OpenMP](https://en.wikipedia.org/wiki/OpenMP)* on MS Windows and GNU/Linux.

### Pixels streaming

If DarkFlow is [configured as suggested](/docs/configuration.{{ page.lang }}/#resources-preferences), the operating system will stream the images from the hard drive to the operators using access patterns friendly to both SSD and spinning hard drives.

### 48-bit Pixel format

16-bit per channel RGB permits an acceptable memory footprint in addition to providing good performance algorithms. And even better performances when the usage of a [LUT](https://en.wikipedia.org/wiki/Lookup_table) is possible.

For many operators, floating point numbers are internaly used in order to keep a good precision in computation.

The precision of 16-bit per channel images, especialy in the dark parts, may be enhanced using a logarithmic scale that guaranties 4096 steps per [EV](https://en.wikipedia.org/wiki/Exposure_value) on a 16EV dynamic range. This logarithmic scale is named *HDR* within DarkFlow.
