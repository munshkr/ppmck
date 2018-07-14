# ppmck

This is a fork of [ppmck 9a ex11.3 am1](https://github.com/AoiMoe/ppmck), by
[AoiMoe](https://github.com/AoiMoe) and
[BouKiCHi](https://github.com/BouKiCHi).  See original [ppmck
website](http://ppmck.web.fc2.com/ppmck.html) for more info.

## Differences

For now there are only minor cosmetic changes:

* Source code files were converted from Shift-JIS and EUC-JP to UTF-8
* Default Makefile is for Unix on ppmck
* Simplified compile script for Unix

## Usage

Use `mknsf` to simply generate a .nsf from an .mml file:

```
$ mknsf my_song.mml
```

## Install

### Linux / OSX

* Go to [Releases](https://github.com/munshkr/ppmck/releases) and download the
  latest .zip file.
* Extract .zip file on some directory, like `~/ppcmk`.
* Edit the `~/.bashrc` and add the following lines at the end of the file:

```bash
# PPMCK setup
export PPMCK_BASEDIR=$HOME/ppmck
export PATH=$PATH:$PPMCK_BASEDIR/bin
```

* Re-open your terminal or run `source .bashrc` to have changes applied.

### Windows

*to-do*

### Source code

* Download the source code either from Github or by cloning the repository.
* Run `make` to compile both ppmck compiler (`ppmckc`) and Nesasm assembler (`nesasm`).
* Follow the instructions for configuring your `$PATH` to your `bin/` directory
  inside the repository.
