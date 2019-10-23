# TestDisplay Application

## Overview

The *TestDisplay* application is an application used to display the current status of automated testing on the devices touch-screen. It is designed to run on a Raspberry Pi SBC with the **Qt** software installed.

The primary objective of *TestDisplay* is to spawn python unit-tests through shell scripts. The results of the tests are stored in a database backend (currently using `PostgreSQL`).

## Features

The *TestDisplay* application boasts the following features:

* Customizable user interface elements
    * Font Family
    * Font Size
    * Font Color
* Controlled through TCP/IP port.
    * Commands to set heading and message text.
    * Commands to inquire status.
    * Commands to start/stop test scripts.
* Web interface
    * Ability to start/stop tests.
    * Display status of running tests.
    * Display existing schedule.

## Compatability

The *TestDisplay* application has been successfully built on the following platforms:

* Ubuntu 18.04 running on x86_64
* Ubuntu 18.04 running on Arm (RasPi2 & 3)

Minimum supported Qt Version 5.10

## Configuration

*TestDisplay* uses several files for configuration. These files are listed below:

### Files

| File | Description |
| ---- | ----------- |
| `~/.config/wunderbar/TestDisplay.conf` | General configuration file setting options. |
| `~/.testdisplay/schedule.xml` | Configure which scripts are run at which times on which day of the week.|
| `~/.testdisplay/scripts.xml` | Configure which scripts can be launched by key-press. |
| `~/.testdisplay/styles.xml` | Configure available styles (fonts, colors, etc.) |

`-TBD-`

## Database Backend

`-TBD-`

## Web Interface

The web interface runs, by default, listening on port `8080`.

Here is a sample of the display of the status page of the web interface.

![Web Interface](doc/webinterface.png)
