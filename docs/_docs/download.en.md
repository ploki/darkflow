---
lang: en
layout: docs
title: Download
permalink: /docs/download.en/
---

{% for post in site.categories.release %}
{%   if post.lang == page.lang %}
{%     assign latest_version = post.version %}
{%     break %}
{%   endif %}
{% endfor %}

DarkFlow is free software and Cross-platform. It may be [built from source](/docs/build.{{page.lang}}/) by yourself. Or downloaded as a binary package for a your computer.

## Microsoft Windows

<div class="note info">
  <h5>MS Windows 32-bit (from Seven to 10)</h5>
  <p>Download the latest <a href="http://darkflow.org/download/windows/setup-darkflow-{{ latest_version }}-x86.exe">x86 DarkFlow Installer</a>, version {{ latest_version }}</p>
</div>

<div class="note info">
  <h5>MS Windows 64-bit (from Seven to 10)</h5>
  <p>Download the latest <a href="http://darkflow.org/download/windows/setup-darkflow-{{ latest_version }}-x64.exe">x64 DarkFlow Installer</a>, version {{ latest_version }}</p>
</div>

[Proceed to installation on your PC running Windows](/docs/installation.{{ page.lang}}/#darkflow-on-ms-windows)

## macOS

<div class="note info">
  <h5>OS X El Capitan / macOS Sierra</h5>
  <p>Download the latest <a href="http://darkflow.org/download/osx/darkflow-{{ latest_version }}.dmg">DarkFlow App</a>, version {{ latest_version }}</p>
</div>

[Proceed to installation on your mac](/docs/installation.{{ page.lang}}/#darkflow-on-macos)

## GNU/Linux

<div class="note">
  <h5>GNU/Linux: use the package manager</h5>
  <p>You may safely skip this chapter and proceed directly to the <a href="/docs/installation.en/#darkflow-on-gnulinux">installation instructions</a></p>
</div>

