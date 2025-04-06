# Retro Rewind Launcher

This is an experimental channel for Wii, Wii U and Dolphin to launch and manage the Retro Rewind Mario Kart Wii distribution. It is still heavily a work in progress. All contributions are welcome; however, the current codebase is fast-moving and it will be easier to contribute once the foundations are laid.

### Motivation

This channel is set to replace the existing Riivolution-based launcher. It solves a number of issues with that launcher:

- It is very overengineered and contains many unused features,
- It takes a long time to initially load,
- It is diffiult and time consuming to maintain,
- It doesn't work with Dolphin.

In addition, a new channel would allow us to add novel features such as a quick launch to skip menus and get right into playing.

### Technical Details

The best way to understand this channel on a technical level is to read the source code, as a lot of it is documented. Wiibrew is also a fantastic resource regarding all things Wii homebrew: https://wiibrew.org/

### Building

In order to build this project, you need:

- [DevkitPro](https://devkitpro.org/wiki/Getting_Started), specifically all packages in the `wii-dev` and `ppc-dev` groups.
- Additional libraries: libcurl (the `install-libs.sh` script can install them for you)

This project uses a `Makefile` for building the project: running `make` in the root directory will build the project and produce a `RR-Launcher.dol` file.

### License

This project is licenced via the GPLv3. Refer to LICENSE, or go to https://www.gnu.org/licenses/, for more information.
