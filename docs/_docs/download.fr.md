---
lang: fr
layout: docs
title: Téléchargement
permalink: /docs/download.fr/
---

{% for post in site.categories.release %}
{%   if post.lang == page.lang %}
{%     assign latest_version = post.version %}
{%     break %}
{%   endif %}
{% endfor %}

DarkFlow est un logiciel libre et multiplate-forme. Vous pouvez le [compiler depuis les sources](/docs/build.{{page.lang}}/) vous-même. Ou télécharger de quoi l'installer directement sur votre système.

## Microsoft Windows

<div class="note info">
  <h5>MS Windows 32-bit (de Seven à 10)</h5>
  <p>Téléchargez le dernier <a href="http://darkflow.org/download/windows/setup-darkflow-{{ latest_version }}-x86.exe">Installateur DarkFlow x86</a>, version {{ latest_version }}</p>
</div>

<div class="note info">
  <h5>MS Windows 64-bit (de Seven à 10)</h5>
  <p>Téléchargez le dernier <a href="http://darkflow.org/download/windows/setup-darkflow-{{ latest_version }}-x64.exe">Installateur DarkFlow x64</a>, version {{ latest_version }}</p>
</div>

## macOS

<div class="note info">
  <h5>OS X El Capitan / macOS Sierra</h5>
  <p>Téléchargez la dernière <a href="http://darkflow.org/download/osx/darkflow-{{ latest_version }}.dmg">App DarkFlow</a>, version {{ latest_version }}</p>
</div>

## GNU/Linux

<div class="note">
  <h5>GNU/Linux: Utilisez le gestionnaire de paquets</h5>
  <p>Vous pouvez sauter sans risque cette section et aller directement aux <a href="/docs/installation.en/">instructions d'installation</a></p>
</div>

## [Procéder à l'installation](/docs/installation.{{ page.lang}}/)
