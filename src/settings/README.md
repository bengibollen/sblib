# LDMud Settings for Silver Bucket Mudlib

This directory contains the settings file used to configure the LDMud driver for the Silver Bucket mudlib.

## Files

- `sblib` - Main settings file for the mudlib

## Usage

The settings file is automatically copied to the LDMud driver during the Docker build process and used with the `--with-setting=sblib` configure option.

## Customization

To modify the driver configuration:

1. Edit the `sblib` settings file
2. Rebuild the Docker container with `docker-compose build`

## Documentation

For more information about LDMud settings, see the LDMud documentation at:
https://github.com/ldmud/ldmud/tree/master/src/settings
