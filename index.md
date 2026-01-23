<!-- SPDX-FileCopyrightText: 2025 Benedikt Spranger <b.spranger@linutronix.de> -->
<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->


# Welcome to the sertest-NG project

Serial interfaces like RS-232, RS-422/485 or CAN bus are even used in this
modern times. While reworking the 8250 driver in the Linux kernel developer
[John Ogness](<john.ogness@linutronix.de>) faced plethora of nasty hardware
dedication with modern serial hardware. Single tasks overtop the banks in
getting functional hardware. This sucks and reliable full featured serial
testing hardware is needed.

sertest-ng tries to fill that gap. It has a USB interface to control a
CAN/CAN-FD bus interface, a full featured RS-232 interface and a RS-422/485
interface. On top a RX/TX only RS-232 is equipped.

## Documentation

### Schematic

Link to the [Schematic](https://eurovibes.github.io/sertest-ng/Fabrication/sertest-ng-schematic_0.1.pdf).

### Layout

#### Top
![Top Layer](https://eurovibes.github.io/sertest-ng/Fabrication/PCB/blue/sertest-ng-top_0.1.jpg)

#### Bottom
![Bottom Layer](https://eurovibes.github.io/sertest-ng/Fabrication/PCB/blue/sertest-ng-bottom_0.1.jpg)

### BoM
sertest-ng provides am [interactive BoM](https://eurovibes.github.io/sertest-ng/Fabrication/BoM/sertest-ng-ibom_0.1.html).

## Fabrication
**sertest-ng** provides generic gerber files and fabrication data for JLCPCB and
Seeed Fusion PCB.

### JLCPCB
Fabrication files for [JLCPCB](https://eurovibes.github.io/sertest-ng/Fabrication/JLCPCB/sertest-ng-JLCPCB_0.1.zip).

### Seeed Fusion PCB
Fabrication files for [Seeed Fusion PCB](https://eurovibes.github.io/sertest-ng/Fabrication/FusionPCB/sertest-ng-FusionPCB_0.1.zip).

## License rules

**sertest-ng** is provided under the terms of the CERN-OHL-S license version 2 as
provided in the LICENSES/CERN-OHL-S-2.0 file.

Instead of adding license boilerplates to the individual files, **sertest-ng**
uses SPDX license identifiers, which are machine parseable and considered
legaly equivalent.

The SPDX license identifier in sertest-ng shall be added at the first possible
line in a file which can contain a comment. This is normally the first line
except for scripts. Scripts require the #!PATH_TO_INTERPRETER tag in the
first line; place the SPDX identifier into the second line.

The SPDX license identifier is added in form of a comment.

## Copyright and License

> Copyright Benedikt Spranger 2025.  
> This source describes Open Hardware and is licensed under the CERNOHL-S v2.  
> You may redistribute and modify this source and make products using it
> under the terms of the [CERN-OHL-S v2](https://ohwr.org/cern_ohl_s_v2.txt).  
> This source is distributed WITHOUT ANY EXPRESS OR IMPLIED
> WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY
> QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please see
> the CERN-OHL-S v2 for applicable conditions.  
> Source location: (https://github.com/eurovibes/sertest-ng)  
> As per CERN-OHL-S v2 section 4, should You produce hardware based
> on this source, You must where practicable maintain the Source Location
> visible on the external case of the **sertest-ng** or other products you make
> using this source.