#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>

#include "libusb.h"

int YUBIMON_RUNNING = 1;

static int LIBUSB_CALL
hotplug_callback_attach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *data) {
    int rc = -1;
    struct libusb_device_descriptor desc;

    if (LIBUSB_SUCCESS != (rc = libusb_get_device_descriptor(dev, &desc))) {
        syslog(LOG_ERR, "Cannot get usb device descriptor: %s", libusb_error_name(rc));
        return EXIT_FAILURE;
    }

    return pclose(popen("ssh-add -s /Library/OpenSC/lib/opensc-pkcs11.so", "r"));
}

static int LIBUSB_CALL
hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *data)
{
    return pclose(popen("ssh-add -e /Library/OpenSC/lib/opensc-pkcs11.so", "r"));
}

void
handle_sigterm(int signal) {
    YUBIMON_RUNNING = 0;
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

    signal(SIGTERM, &handle_sigterm);
    openlog("yubimon", LOG_PID, LOG_DAEMON);
    syslog(LOG_NOTICE, "yubimon starting");

    if (0 > (rc = libusb_init(NULL))) {
        syslog(LOG_ERR, "Cannot initialize libusb: %s", libusb_error_name(rc));
        return EXIT_FAILURE;
    }

    if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        syslog(LOG_ERR, "No USB hotplug capability on this system");
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    if (LIBUSB_SUCCESS != (rc = libusb_hotplug_register_callback(NULL,
                                                                 LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
                                                                 0,
                                                                 vendor_id,
                                                                 product_id,
                                                                 class_id,
                                                                 hotplug_callback_attach,
                                                                 NULL,
                                                                 &hp[0]))) {
        syslog(LOG_ERR, "Cannot register hotplug attach callback: %s", libusb_error_name(rc));
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    if (LIBUSB_SUCCESS != (rc = libusb_hotplug_register_callback(NULL,
                                                                 LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                                                                 0,
                                                                 vendor_id,
                                                                 product_id,
                                                                 class_id,
                                                                 hotplug_callback_detach,
                                                                 NULL,
                                                                 &hp[1]))) {
        syslog(LOG_ERR, "Cannot register hotplug detach callback: %s", libusb_error_name(rc));
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    while (YUBIMON_RUNNING) {
        if (0 > (rc = libusb_handle_events(NULL))) {
            syslog(LOG_ERR, "libusb_handle_events() failed: %s\n", libusb_error_name(rc));
        }
    }

    syslog(LOG_NOTICE, "yubimon exiting");

    libusb_exit(NULL);

    return EXIT_SUCCESS;
}
