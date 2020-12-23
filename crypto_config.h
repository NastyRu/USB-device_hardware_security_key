#define PASSWORD "Verydifficultpassword\n"

// Secret files will crypt or decrypt upon detecting change in usb state.
static char *secret_apps[] = {
    "/home/parallels/Desktop/Operating_systems_coursework/file.txt",
    "/home/parallels/Desktop/Operating_systems_coursework/xor",
    "/usr/bin/firefox",
	NULL,
};
