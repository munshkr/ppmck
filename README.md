# ppmck

Fork of [ppmck 0.9a](http://ppmck.web.fc2.com/ppmck.html)

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

* Go to [Releases]() and download the latest .zip file
* Extract .zip file on some directory, like `~/ppcmk`
* Edit the `~/.bashrc` and add the following lines at the end of the file:

```bash
# PPMCK setup
export PPMCK_BASEDIR=$HOME/ppcmk
export PATH=$PATH:$PPMCK_BASEDIR/bin
```

* Re-open your terminal or run `source .bashrc` to have changes applied

### Windows

*to-do*
