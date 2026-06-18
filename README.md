# Media Downloader

Portuguese version: [README.pt-BR.md](/home/wbaamaral/Desenvolvimento/0-aa-cerebro-wba/00-caixa-de-entrada/workspace/c++/media-downloader/README.pt-BR.md)

This project is a Qt/C++ based graphical frontend for multiple CLI tools used to download online media.

[yt-dlp](https://github.com/yt-dlp/yt-dlp) is the default supported tool. Other tools can be added by
installing their extensions, and the list of supported extensions is maintained [here](https://github.com/mhogomchungu/media-downloader/wiki/Extensions).


## Features

1. The GUI can be used to download any media from any website supported by installed extensions.

2. The GUI offers a configurable list of preset options that can be used when media is available in multiple formats.

3. The GUI offers the ability to do an unlimited number of concurrent downloads. Be careful with this ability because doing too many concurrent
downloads may cause the host to ban you.

4. The GUI offers the ability to do batch downloads by entering individual links in the UI or telling the app to read them from a local file.

5. The GUI offers the ability to download playlists from websites that support them, such as YouTube.

6. The GUI offers the ability to manage playlist links to monitor activity more easily.

7. The GUI is offered in multiple languages. The supported languages are English, Chinese, Spanish, Polish, Turkish, Russian, Japanese, French, Italian, Portuguese, Arabic, Korean, Swedish, German, Greek and Ukrainian.

## Extensions

Media Downloader is a GUI front end to [yt-dlp](https://github.com/yt-dlp/yt-dlp), [gallery-dl](https://github.com/mikf/gallery-dl), [you-get](https://github.com/soimort/you-get), [svtplay-dl](https://github.com/spaam/svtplay-dl), [aria2c](https://aria2.github.io/), [wget](https://www.gnu.org/software/wget) and [get-sauce](https://github.com/gan-of-culture/get-sauce).

To install these extensions, go to the `Configure` tab, open the `Extensions` sub tab, click `Add An Extension`, and select the extension you want to install.

## FAQ
A Frequently Asked Questions page is [here](https://github.com/mhogomchungu/media-downloader/wiki/Frequently-Asked-Questions).

#### Before running for the first time

Make sure you have internet access before you run "Media Downloader" for the first time because it will attempt to download the latest version of yt-dlp. Installing most extensions will also cause "Media Downloader" to access the internet to download the extension executable.

## Current status

The project is currently aligned with pre-release `v5.6.1-rc.2` and includes:

- startup flow improvements;
- UI layout and responsiveness refinements;
- initial window centering on the active monitor;
- persistence of window state between runs;
- Configure tab refinements;
- fixes to engine discovery and execution flow.

## Binary packages

#### Bundle for MacOS

Bundle for the arm64 build of macOS is [here](https://github.com/mhogomchungu/media-downloader/releases/download/5.6.1/MediaDownloaderQt6-arm64-5.6.1.dmg).

Bundle for the x86_64 build of macOS is [here](https://github.com/mhogomchungu/media-downloader/releases/download/5.6.1/MediaDownloaderQt6-x86_64-5.6.1.dmg).

These bundles are not notarized and your system may report them as "corrupted". Search the internet for how to install bundles that are not notarized if you want to use this app on macOS. This bundle works on macOS 14.0 or later.

#### Installer for Microsoft Windows

Installer for Microsoft Windows that is 32 bit, built with Qt5 and has a minimum requirement of Windows 7 is [here](https://github.com/mhogomchungu/media-downloader/releases/download/5.6.1/MediaDownloaderQt5-5.6.1.setup.exe).

Installer for Microsoft Windows that is 64 bit, built with Qt6 and has a minimum requirement of Windows 10 is [here](https://github.com/mhogomchungu/media-downloader/releases/download/5.6.1/MediaDownloaderQt6-5.6.1.setup.exe).

#### Portable version for Microsoft Windows

A portable version is self-contained, keeps everything in the application folder, and does not need to be installed first.

Portable version for Microsoft Windows that is 32 bit, built with Qt5 and has a minimum requirement of Windows 7 is [here](https://github.com/mhogomchungu/media-downloader/releases/download/5.6.1/MediaDownloaderQt5-5.6.1.zip).

Portable version for Microsoft Windows that is 64 bit, built with Qt6 and has a minimum requirement of Windows 10 is [here](https://github.com/mhogomchungu/media-downloader/releases/download/5.6.1/MediaDownloaderQt6-5.6.1.zip).

You can also install the portable version for Windows using scoop with the following commands:

Add the extras bucket:
```powershell
scoop bucket add extras
```
Install Media Downloader:
```powershell
scoop install media-downloader
```

Git versions for Windows and macOS can be downloaded from [here](https://github.com/mhogomchungu/media-downloader-git/releases).

#### Problems with Windows antivirus programs

Once in a while, Windows Defender and other antivirus tools will report this application as a virus/unsafe
or Potentially unwanted. These are false positive reports and they are tracked [here](https://github.com/mhogomchungu/media-downloader/issues/481).


#### Flatpak

Media Downloader is on [Flathub](https://flathub.org/apps/io.github.mhogomchungu.media-downloader) for those who prefer to use Flatpak.

#### AUR package for Arch Linux
Arch Linux users can build the project from source using [this](https://aur.archlinux.org/packages/media-downloader) AUR package.

#### Package for Fedora
Media Downloader is in the official Fedora repositories and can be installed by running ```sudo dnf -y install media-downloader```

### Binary packages for other Linux distributions

Binary packages I maintain for a few Linux distributions are [here](https://software.opensuse.org//download.html?project=home%3Aobs_mhogomchungu&package=media-downloader).

### Packaging Status

A short list of distributions that have Media Downloader in their repositories, and the version they ship, is maintained [here](https://repology.org/project/media-downloader/badges).


# How to compile on Linux

1. Clone the repo and `cd` into it:
```console
git clone https://github.com/mhogomchungu/media-downloader && cd media-downloader
```

2. Make the helper script executable:
```console
chmod +x build_linux.sh
```

3. Run the shell script:
```console
./build_linux.sh
```

The helper scripts follow the same flow:

1. configure the build with CMake;
2. compile with `cmake --build`;
3. run the smoke tests through `ctest`;
4. optionally launch the app when `RUN_AFTER_BUILD=1`.

If you want to run the flow manually:

```console
mkdir build
cd build
cmake .. -DBUILD_TESTING=ON
cmake --build . -j"$(nproc)"
ctest --output-on-failure
```


### Fedora

Fedora users can use the following script to build from source:

```console
./build_fedora.sh
```
### Arch linux

Arch linux users can use the following script to build from source:

```console
./build_arch.sh
```

# Screenshots


![media-downloader.png](https://raw.githubusercontent.com/mhogomchungu/media-downloader/main/images/media-downloader-1.png)

![media-downloader.png](https://raw.githubusercontent.com/mhogomchungu/media-downloader/main/images/media-downloader-2.png)

![media-downloader.png](https://raw.githubusercontent.com/mhogomchungu/media-downloader/main/images/media-downloader-3.png)

![media-downloader.png](https://raw.githubusercontent.com/mhogomchungu/media-downloader/main/images/media-downloader-4.png)

![media-downloader.png](https://raw.githubusercontent.com/mhogomchungu/media-downloader/main/images/media-downloader-5.png)

![media-downloader.png](https://raw.githubusercontent.com/mhogomchungu/media-downloader/main/images/media-downloader-6.png)

![media-downloader.png](https://raw.githubusercontent.com/mhogomchungu/media-downloader/main/images/media-downloader-7.png)

# Disclaimer

This program is intended to be used  in a way that does not violate any laws that are applicable to its users.

# License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
