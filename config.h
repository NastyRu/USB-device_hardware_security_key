#define PASSWORD "Verydifficultpassword"
#define USB_FOLDER "/media/parallels/"
#define PASSWORD_FILE "password.txt"

    /*char path[80];
    strcpy(path, USB_FOLDER);
    strcat(path, name_device);
    strcat(path, "/");
    strcat(path, PASSWORD_FILE);
    char *data = read_file(path);
    printk(KERN_INFO "USB MODULE: %s %s\n", path, data);*/

// Secret files will crypt or decrypt upon detecting change in usb state.
static char *secret_apps[] = {
    "/home/parallels/Desktop/secret_folder",
	NULL,
};

struct known_usb_device {
    struct usb_device_id dev_id;
    char *name;
};

// List of all USB devices you know
static const struct known_usb_device known_devices[] = {
    // Macros used to create struct known_usb_device for device.
    { .dev_id = { USB_DEVICE(0x058f, 0x6387) }, .name = "SAG" },
};
