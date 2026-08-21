# CRC EEPROM Data Validator

A CRC of 256 element array with word data from a file is calculated. Then this data is written to EEPROM.
The data is read again from the EEPROM and its CRC calculated.
Both CRCs should match to verify that the data in EEPROM is the valid array data from file.
