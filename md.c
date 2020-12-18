#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/fs_struct.h>
#include <asm/current.h>
#include <linux/proc_fs.h>
#include <linux/mount.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/syscalls.h>
#include <linux/notifier.h>
#include "config.h"

#define USB_COUNT 4
#define NAME_USB_LEN 8

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sidenko");

// Read password from USB device
static char *read_file(char *filename)
{
    struct kstat *stat;
    struct file *fp;
    mm_segment_t fs;
    loff_t pos = 0;
    char *buf;
    int size;

    fp = filp_open(filename, O_RDWR, 0644);
    if (IS_ERR(fp))
    {
        return NULL;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    
    stat = (struct kstat *)kmalloc(sizeof(struct kstat), GFP_KERNEL);
    if (!stat)
    {
        return NULL;
    }

    vfs_stat(filename, stat);
    size = stat->size;

    buf = kmalloc(size, GFP_KERNEL);
    if (!buf) 
    {
        kfree(stat);
        return NULL;
    }

    kernel_read(fp, buf, size, &pos);

    filp_close(fp, NULL);
    set_fs(fs);
    kfree(stat);
    buf[size]='\0';
    return buf;
}

// Call decryption from user space
static int call_decryption(char *name_device) {
    printk(KERN_INFO "USB MODULE: Call_decrypt\n");

    char path[80];
    strcpy(path, USB_FOLDER);
    strcat(path, name_device);
    strcat(path, "/");
    strcat(path, PASSWORD_FILE);
    char *data = read_file(path);

    char *argv[] = {
        "/home/parallels/Desktop/Operating_systems_coursework/crypto",
        data,
        NULL };

    static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", 
        NULL };

    if (call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC) < 0) 
    {
        return -1;
    }

    return 0;
}

// Call encryption from user space
static int call_encryption(void) {
    printk(KERN_INFO "USB MODULE: Call_encrypt\n");
    char *argv[] = {
        "/home/parallels/Desktop/Operating_systems_coursework/crypto",
        NULL };

    static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", 
        NULL };

    if (call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC) < 0) 
    {
        return -1;
    }

    return 0;
}

typedef struct our_usb_device {
    struct usb_device_id dev_id;
    // This is used to link nodes together in the list.
    struct list_head list_node;
} our_usb_device_t;

// Declare and init the head node of the linked list.
LIST_HEAD(connected_devices);

// Match device with device id.
static bool device_match_device_id(struct usb_device *dev, const struct usb_device_id *dev_id)
{
    // Check idVendor and idProduct, which are used.
    if (dev_id->idVendor != dev->descriptor.idVendor)
        return false;
    if (dev_id->idProduct != dev->descriptor.idProduct)
        return false;
    return true;
}

// Match device id with device id.
static bool device_id_match_device_id(struct usb_device_id *new_dev_id, const struct usb_device_id *dev_id)
{
    // Check idVendor and idProduct, which are used.
    if (dev_id->idVendor != new_dev_id->idVendor)
        return false;
    if (dev_id->idProduct != new_dev_id->idProduct)
        return false;
    return true;
}

// Check our list of devices, if we know device.
static char *usb_device_id_is_known(struct usb_device_id *dev)
{
    unsigned long known_devices_len = sizeof(known_devices) / sizeof(known_devices[0]);
    int i = 0;
    for (i = 0; i < known_devices_len; i++)
    {
        if (device_id_match_device_id(dev, &known_devices[i].dev_id))
        {
            int size = sizeof(known_devices[i].name);
            char *name = (char *)kmalloc(size + 1, GFP_KERNEL);
            int j = 0;
            for (j = 0; j < size; j++)
                name[j] = known_devices[i].name[j];
            name[size + 1] = '\0';

            return name;
        }
    }
    return NULL;
}

static char *knowing_device(void)
{
    our_usb_device_t *temp;
    int count = 0;
    char *name;

    list_for_each_entry(temp, &connected_devices, list_node) {
        name = usb_device_id_is_known(&temp->dev_id);
        if (!name)
            return NULL;
        count++;
    }
    if (0 == count)
        return NULL;
    return name;
}

static void print_our_usb_devices(void)
{
    our_usb_device_t *temp;
    int count = 0;
    list_for_each_entry(temp, &connected_devices, list_node) {
        printk(KERN_INFO "USB MODULE: Node %d data = %x:%x\n", count++, temp->dev_id.idVendor, temp->dev_id.idProduct);
    }
}

static void add_our_usb_device(struct usb_device *dev)
{
    our_usb_device_t* new_usb_device = (our_usb_device_t *)kmalloc(sizeof(our_usb_device_t), GFP_KERNEL);
    struct usb_device_id new_id = { USB_DEVICE(dev->descriptor.idVendor, dev->descriptor.idProduct) };
    new_usb_device->dev_id = new_id;
    list_add_tail(&new_usb_device->list_node, &connected_devices);
}

static void delete_our_usb_device(struct usb_device *dev)
{
    our_usb_device_t *device, *temp;
    list_for_each_entry_safe(device, temp, &connected_devices, list_node) 
    {
        if (device_match_device_id(dev, &device->dev_id))
        {
            list_del(&device->list_node);
            kfree(device);
        }
    }
}

// If usb device inserted.
static void usb_dev_insert(struct usb_device *dev)
{   
    add_our_usb_device(dev);
    char *name = knowing_device();
    
    if (name)
    {
        if (state_encrypt)
            call_decryption(name);
        state_encrypt = false;
        printk(KERN_INFO "USB MODULE: New device we can encrypt.\n");
    }
    else
   {
        if (!state_encrypt)
            call_encryption();
        state_encrypt = true;
        printk(KERN_INFO "USB MODULE: New device, we can't encrypt.\n");
    }
}

// If usb device removed.
static void usb_dev_remove(struct usb_device *dev)
{
    delete_our_usb_device(dev);
    char *name = knowing_device();

    if (name)
    {
        if (state_encrypt)
            call_decryption(name);
        state_encrypt = false; 
        printk(KERN_INFO "USB MODULE: Delete device, we can encrypt.\n");
    }
    else
    {
        if (!state_encrypt)
            call_encryption();
        state_encrypt = true;
        printk(KERN_INFO "USB MODULE: Delete device, we can't encrypt.\n");
    }
}

// New notify.
static int notify(struct notifier_block *self, unsigned long action, void *dev)
{
    // Events, which our notifier react.
    switch (action) 
    {
        case USB_DEVICE_ADD:
            usb_dev_insert(dev);
	        break;
        case USB_DEVICE_REMOVE:
            usb_dev_remove(dev);
	        break;
        default:
	        break;
    }
    return 0;
}

// Struct to react on different notifies.
static struct notifier_block usb_notify = {
    .notifier_call = notify,
};

static int __init my_module_init(void)
{
    usb_register_notify(&usb_notify);
    call_encryption();
    printk(KERN_INFO "USB MODULE: loaded.\n");
    return 0;
}

static void __exit my_module_exit(void)
{
    usb_unregister_notify(&usb_notify);    
    printk(KERN_INFO "USB MODULE: unloaded.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
