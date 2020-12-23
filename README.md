# Operating_systems_coursework

The purpose of this work is to implement a loadable kernel module for tracking USB devices, that are the key to accessing the application.

You need to develop software for tracking USB devices, which has the following functionality:
- list of allowed devices **config.h**;
- list of paths to secret files **crypto_config.h**;
- tracking the appearance of new USB devices;
  - if the device is known, the file is decrypted;
  - if the device is not known, the file is encrypted.
