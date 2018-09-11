#!/usr/bin/python2
"""
Copyright 2015, Sinclair R.F., Inc.

This program is distributed under the GPL, version 2.

Demonstrate how to configure the FT230X USB UART bridge as follows:
  max_power     500 mA
  CBUS3         Drive 1 (accomodate PCB error)
"""

import sys

# Need to install libftdi for the following to work (see README.txt)
import ftdi1 as ftdi

# Define class for displaying errors.
class ErrorMsg(Exception):
  def __init__(self,message):
    self.message = message
  def __str__(self):
    return self.message

# Function to convert CBUSX values to human-readable strings
def cbush_string(value):
  if value == ftdi.CBUSX_AWAKE:
    return 'AWAKE'
  if value == ftdi.CBUSX_BAT_DETECT:
    return 'BAT_DETECT'
  if value == ftdi.CBUSX_BAT_DETECT_NEG:
    return 'BAT_DETECT_NEG'
  if value == ftdi.CBUSX_BB_RD:
    return 'BB_RD'
  if value == ftdi.CBUSX_BB_WR:
    return 'BB_WR'
  if value == ftdi.CBUSX_CLK24:
    return 'CLK24'
  if value == ftdi.CBUSX_CLK12:
    return 'CLK12'
  if value == ftdi.CBUSX_CLK6:
    return 'CLK6'
  if value == ftdi.CBUSX_DRIVE_0:
    return 'DRIVE_0'
  if value == ftdi.CBUSX_DRIVE1:
    return 'DRIVE_1'
  if value == ftdi.CBUSX_I2C_RXF:
    return 'I2C_RXF'
  if value == ftdi.CBUSX_I2C_TXE:
    return 'I2C_TXE'
  if value == ftdi.CBUSX_IOMODE:
    return 'IOMODE'
  if value == ftdi.CBUSX_PWREN:
    return 'PWREN'
  if value == ftdi.CBUSX_RXLED:
    return 'RXLED'
  if value == ftdi.CBUSX_SLEEP:
    return 'SLEEP'
  if value == ftdi.CBUSX_TIME_STAMP:
    return 'TIME_STAMP'
  if value == ftdi.CBUSX_TRISTATE:
    return 'TRISTATE'
  if value == ftdi.CBUSX_TXDEN:
    return 'TXDEN'
  if value == ftdi.CBUSX_TXLED:
    return 'TXLED'
  if value == ftdi.CBUSX_TXRXLED:
    return 'TXRXLED'
  if value == ftdi.CBUSX_VBUS_SENSE:
    return 'VBUS_SENSE'
  return 'UNKNOWN'

# Surround the program with a try ... except clause.
try:

  # Allocate and inialize an ftdi context.
  ftdic = ftdi.new()
  if ftdic == 0:
    raise ErrorMsg('ftdi.new() failed')

  # List all the FT230X devices.
  nDevices, devlist = ftdi.usb_find_all(ftdic, 0x0403, 0x6015)
  if nDevices < 0:
    raise ErrorMsg('ftdi.usb_find_all error = %s' % ftdi.get_error_string(ftdic))
  elif nDevices == 0:
    raise ErrorMsg('No FT230X devices found')
  elif nDevices != 1:
    raise ErrorMsg('More than one FT230X device found')

  # Display the identified single FT230X device.
  ret, manufacturer, description, serial = ftdi.usb_get_strings(ftdic, devlist.dev)
  if ret < 0:
    raise ErrorMsg('ftdi.usb_get_strings error = %s' % ftdi.get_error_string(ftdic))
  print 'manufacturer="%s" description="%s" serial="%s"' % (manufacturer, description, serial)

  # Open the identified single FT230X device.
  ret = ftdi.usb_open_desc(ftdic, 0x0403, 0x6015, description, serial)
  if ret < 0:
    raise ErrorMsg('ftdi.usb_open_desc error = %s' % ftdi.get_error_string(ftdic))

  # Read the chip id.
  ret, chipid = ftdi.read_chipid(ftdic)
  if ret < 0:
    raise ErrorMsg('ftdi.read_chipid error = %s' % ftdi.get_error_string(ftdic))
  print 'chip id=0x%08X' % (chipid % 2**32)

  # Read the EEPROM
  ret = ftdi.read_eeprom(ftdic)
  if ret < 0:
    raise ErrorMsg('ftdi.read_eeprom error = %s' % ftdi.get_error_string(ftdic))

  # Get a read-only copy of the EEPROM
  if True:
    eeprom_size = ftdic.eeprom.size
    ret, eeprom_buf = ftdi.get_eeprom_buf(ftdic, eeprom_size)
    if ret < 0:
      raise ErrorMsg('ftdi.get_eeprom_buf error = %s' % ftdi.get_error_string(ftdic))
    for i in range(0,eeprom_size,16):
      sys.stdout.write('%04x:  ' % i)
      for j in range(16):
        sys.stdout.write('%02x ' % ord(eeprom_buf[i+j]))
        if j in (7,15,):
          sys.stdout.write(' ')
      for j in range(16):
        x = eeprom_buf[i+j]
        if 32 <= ord(x) <= 0x7E:
          sys.stdout.write(x)
        else:
          sys.stdout.write('.')
      sys.stdout.write('\n')

  # Read and display the EEPROM (in human readable format)
  ret = ftdi.eeprom_decode(ftdic, 1)
  if ret < 0:
    raise ErrorMsg('ftdi.eeprom_decode error = %s' % ftdi.get_error_string(ftdic))

  # Set the maximum power to 500mA.
  print 'initial max_power = %dmA' % ftdic.eeprom.max_power
  ftdic.eeprom.max_power = 500
  print 'new max_power = %dmA' % ftdic.eeprom.max_power

  # Set CBUS3 to DRIVE_1 (the board needs to be reworked to use PWREN# and BCD#)
  ret, value = ftdi.get_eeprom_value(ftdic,ftdi.CBUS_FUNCTION_3)
  if ret < 0:
    raise ErrorMsg('ftdi.get_eeprom_value error = %s' % ftdi.get_error_string(ftdic))
  print 'initial CBUS3 = %d (%s)' % (value,cbush_string(value),)
  ret = ftdi.set_eeprom_value(ftdic,ftdi.CBUS_FUNCTION_3,ftdi.CBUSX_DRIVE1)
  if ret < 0:
    raise ErrorMsg('ftdi.set_eeprom_value error = %s' % ftdi.get_error_string(ftdic))
  ret, value = ftdi.get_eeprom_value(ftdic,ftdi.CBUS_FUNCTION_3)
  if ret < 0:
    raise ErrorMsg('ftdi.get_eeprom_value error = %s' % ftdi.get_error_string(ftdic))
  print 'new CBUS3 = %d (%s)' % (value,cbush_string(value),)

  # Write the new EEPROM settings.
  if False:
    ret = ftdi.eeprom_build(ftdic)
    if ret < 0:
      raise ErrorMsg('ftdi.eeprom_build error = %s' % ftdi.get_error_string(ftdic))
    ret = ftdi.write_eeprom(ftdic)
    if ret < 0:
      raise ErrorMsg('ftdi.write_eeprom error = %s' % ftdi.get_error_string(ftdic))
    print 'EEPROM write succeeded'
  else:
    print 'EEPROM write not attempted'

  # Close the ftdi context.
  ret = ftdi.usb_close(ftdic)
  if ret < 0:
    raise ErrorMsg('ftdi.usb_close error = %s' % ftdi.get_error_string(ftdic))

except ErrorMsg, msg:
  print >> sys.stderr, 'FATAL ERROR:  ' + str(msg)
  exit(1)
