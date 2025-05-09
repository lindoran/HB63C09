# SYSTEM ROMs

This folder contains the system ROM sources. Automation for building the SD card  
is not yet complete, but it's a high priority. This directory includes the source  
code needed to build all the system ROMs that come with the stock SD card  
distribution.

To use the ROMs, simply copy the built binary files to the SD card — that's all  
you need.

All ROMs are 16KB in size and aligned to the top of memory. In the vBIOS loader,  
use the default values:

- Start: `C000`  
- Size: `4000`  

(Just pressing Enter will use the defaults.) Once loaded, the ROM file details will  
automatically be saved to the EEPROM in the controller when settings are changed.

if the settings become corrupt or there is an issue it will drop you back to the 
default settings (BIOS.BIN).


---

## Building the Repository

Currently, the build process is manual. The old `bios.bin` automation needs to be  
fully rewritten.

TO use the computer you do not have to build the SD card resources and the newest
versions will always be stored in the <a href=https://github.com/lindoran/HB63C09/tree/main/System_Roms/ROMS> ROMS </a> location
at <repository location>HB63C09/System_Roms/ROMS/

There are a few required Git submodules:

- `beeb6809` – from my fork, mostly merged with the main repository  
- `flextools` – from my fork, customized for building media for this SBC

To clone the repo with all submodules:

```bash
git clone --recurse-submodules -j8 https://github.com/lindoran/HB63C09.git
```

(Requires Git v2.13 or newer)

More info on cloning submodules:  
https://stackoverflow.com/questions/3796927/how-do-i-git-clone-a-repo-including-its-submodules

---

## Required Tools

You'll need the following tools installed:

- `lwtools` – http://lwtools.ca  
- `asm6809` – https://www.6809.org.uk/asm6809/  
- `as` – http://john.ccac.rwth-aachen.de:8000/as/  
- `as09` – https://home.hccnet.nl/a.w.m.van.der.horst/m6809.html  
- `srecord` – https://srecord.sourceforge.net/
- `flextools` - submodlue from this repo.  you will need to build and install flexfloppy from that repo.

---

## Notes on Building

Each subfolder should contain its own build chain, except for `basic` and `mon09`.

It's clunky for now — you’ll need to build each ROM to produce a `.bin` file. Once  
done, run:

```bash
make install
```

It may not always be `make install`.  beeb6809 has its own toolchain and 
is involved.


Building FLEX is more involved:
1. Build the Makefile in the root of the `flex` folder  
2. Run `make install` in both the `root`(./Flex) and `assist09` directories

Refer to the Makefiles and scripts in each folder to understand how binaries are  
copied to the appropriate SD card folder. For FLEX, the process is especially  
complex.

---

## Final Notes

Assume the actively uploaded binary files in this repo are the latest versions  
available. I’ll update this documentation when I have more time.
