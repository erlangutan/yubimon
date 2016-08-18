#include <stdlib.h>
#include <stdio.h>
#include "libusb.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

static int LIBUSB_CALL
hotplug_callback_attach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data) {
    struct libusb_device_descriptor desc;

    if (LIBUSB_SUCCESS != libusb_get_device_descriptor(dev, &desc)) {
        //fprintf(stderr, "Error getting device descriptor\n");
        //need to bail!
        return EXIT_FAILURE;
    }
    FILE * output = popen("ssh-add -s /Library/OpenSC/lib/opensc-pkcs11.so", "r");
    pclose(output);

    return 0;
}

static int LIBUSB_CALL
hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    FILE * output = popen("ssh-add -e /Library/OpenSC/lib/opensc-pkcs11.so", "r");
    pclose(output);

    return 0;
}

int main(int argc, char *argv[])
{
    libusb_hotplug_callback_handle hp[2];
    int product_id, vendor_id, class_id;
    int rc = -1;

    // matches Yubikey4 vendor and product id
    vendor_id  = 0x1050;
    product_id = 0x0405;
    class_id   = LIBUSB_HOTPLUG_MATCH_ANY;

    if (0 > (rc = libusb_init (NULL))) {
        return EXIT_FAILURE;
    }

    if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG)) {
        //printf ("Hotplug capabilites are not supported on this platform\n");
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    if (LIBUSB_SUCCESS != libusb_hotplug_register_callback(NULL,
                                                           LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
                                                           0,
                                                           vendor_id,
                                                           product_id,
                                                           class_id,
                                                           hotplug_callback_attach,
                                                           NULL,
                                                           &hp[0])) {
        //fprintf(stderr, "Error registering attach callback\n");
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    if (LIBUSB_SUCCESS != libusb_hotplug_register_callback(NULL,
                                                           LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                                                           0,
                                                           vendor_id,
                                                           product_id,
                                                           class_id,
                                                           hotplug_callback_detach,
                                                           NULL,
                                                           &hp[1])) {
        //fprintf(stderr, "Error registering detach callback\n");
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    int done = 0;
    while (1) {
        done += 1;
        if (0 > (rc = libusb_handle_events(NULL))) {
            printf("libusb_handle_events() failed: %s\n", libusb_error_name(rc));
        }
    }

    libusb_exit(NULL);

    return EXIT_SUCCESS;
}
