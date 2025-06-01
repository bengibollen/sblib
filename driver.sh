#! /bin/sh

export PYTHONUNBUFFERED=1

exec /usr/local/bin/ldmud -m /mud/sblib -M secure/master.c --python-script ../ldmud-python/__main__.py
