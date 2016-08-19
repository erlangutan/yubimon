/*
 * The MIT License (MIT) Copyright (c) 2016 Cyrus Durgin
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>

#include "libusb.h"

INT PIN_ATTEMPTS    = 10;
int YUBIMON_RUNNING = 1;

static int 
hotplug_callback_attach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event ev, void *data) {
    int rc = -1;

    for (int tries = PIN_ATTEMPTS; 0 < tries; --tries) {
        rc = pclose(popen("ssh-add -s /Library/OpenSC/lib/opensc-pkcs11.so", "r"));
        if (LIBUSB_SUCCESS == rc) {
            syslog(LOG_NOTICE, "Yubikey attached, SSH key loaded");
            break;
        } else {
            syslog(LOG_WARNING, "Yubikey attached, SSH key not loaded - PIN failure");
        }
    }

    if (LIBUSB_SUCCESS != rc) {
        syslog(LOG_WARNING, "Could not load SSH key after %d attempts, remove/insert Yubikey to try again",
               PIN_ATTEMPTS);
    }

    return rc;
}

static int 
hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event ev, void *data)
{
    syslog(LOG_NOTICE, "Yubikey detached, SSH key unloaded");
    return pclose(popen("ssh-add -e /Library/OpenSC/lib/opensc-pkcs11.so", "r"));
}

void
handle_sigterm(int signal) {
    YUBIMON_RUNNING = 0;
}

int
main(int argc, char *argv[]) {
    libusb_hotplug_callback_handle hp[2];
    int product_id;
    int vendor_id;
    int class_id;
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

    syslog(LOG_NOTICE, "registering callbacks");

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

    syslog(LOG_NOTICE, "yubimon attach callback registered");

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

    syslog(LOG_NOTICE, "yubimon detach callback registered");

    while (YUBIMON_RUNNING) {
        if (0 > (rc = libusb_handle_events(NULL))) {
            syslog(LOG_ERR, "libusb_handle_events() failed: %s\n", libusb_error_name(rc));
        }
    }

    syslog(LOG_NOTICE, "yubimon exiting");

    libusb_exit(NULL);

    return EXIT_SUCCESS;
}
