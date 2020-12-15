// Secret files will crypt or decrypt upon detecting change in usb state.
static char *secret_files[] = {
    "/home/parallels/Desktop/secret_folder",
	NULL,
};

// List of all USB devices you know
static const struct usb_device_id known_devices[] = {
    // Macros used to create struct usb_device_id for device.
    { USB_DEVICE(0x058f, 0x6387) },
};
