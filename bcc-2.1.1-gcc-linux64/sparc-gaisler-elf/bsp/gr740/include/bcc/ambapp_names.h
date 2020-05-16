/*
 * Copyright (c) 2018, Cobham Gaisler AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * AMBA Plug'n Play vendor and device name definitions.
 *
 * Created from GRLIB revision 4234
 */

#ifndef _ambapp_names_h_
#define _ambapp_names_h_

#include <bcc/ambapp_ids.h>

/* A struct containing a readable name of a device id */
struct ambapp_device_name {
        int device_id; /* Unique id of a device */
        char *name;    /* Human readable name of the device */
        char *desc;    /* Description of the device */
};

/* A struct containing a readable name of a vendor and the names of
 * it's devices
 */
struct ambapp_ids {
        unsigned int vendor_id;             /* Unique id of a vendor */
        char *name;                         /* Human readable name of the vendor */
        char *desc;                         /* Description of the vendor */
        struct ambapp_device_name *devices; /* Array of devices from the vendor */
};

/**
* The table that contains all the vendor and device descriptions
 */
extern struct ambapp_ids *ambapp_ids;

/**
* Get human readable vendor/device name
 */
extern const char *ambapp_vendor_id2str(struct ambapp_ids *ids, int vendor);
extern const char *ambapp_device_id2str(struct ambapp_ids *ids, int vendor, int id);

/**
* Get human readable vendor/device description
 */
extern const char *ambapp_vendor_id2desc(struct ambapp_ids *ids, int vendor);
extern const char *ambapp_device_id2desc(struct ambapp_ids *ids, int vendor, int id);

#endif /* _ambapp_names_h_ */
