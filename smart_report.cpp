
#include <iostream>
#include <string>
#include <assert.h>

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(__FreeBSD__)
#include <sys/param.h>
#endif

#include "int64.h"
#include "atacmds.h"
#include "dev_interface.h"
#include "ataprint.h"
#include "knowndrives.h"
#include "scsicmds.h"
#include "scsiprint.h"
#include "smartctl.h"
#include "utility.h"

#include "smart_report.h"

/* Various global flags that the smartmontools classes require to link */
bool failuretest_conservative = false;
unsigned char failuretest_permissive = 0;
void failuretest(failure_type type, int returnvalue) {}
bool printing_is_switchable = false;
bool printing_is_off = false;
void checksumwarning(const char * string) {}

/* Buffer for pout data */
static std::string *pout_buffer = NULL;
/* Max size per pout call */
#define POUT_MAX 4096

void pout(const char *fmt, ...){
    assert(pout_buffer);
    char buf[POUT_MAX];

    va_list ap;
    va_start(ap,fmt);
    vsnprintf(buf, POUT_MAX, fmt, ap);
    va_end(ap);
    *pout_buffer += std::string(buf);
}


static bool initialized = false;

extern "C" {

void smart_init()
{
    assert(!initialized);
    initialized = true;
    smart_interface::init();
}

/**
 * Return a newly allocated smart_report, or NULL on error.
 */
struct smart_report *smart_report_create(char *device_path)
{
    assert(initialized);
    smart_device_auto_ptr dev;

    /* Open device with automatic type detection, return if open fails */
    dev = smi()->get_smart_device(device_path, NULL);
    if (!dev) {
        return NULL;
    }
    dev.replace(dev->autodetect_open());
    if (!dev->is_open()) {
        return NULL;
    }

    /* Set options equivalent to "smartctl -a" */
    scsi_print_options scsiopts;
    ata_print_options ataopts;
    ataopts.drive_info = scsiopts.drive_info = true;
    ataopts.drive_info           = scsiopts.drive_info          = true;
    ataopts.smart_check_status   = scsiopts.smart_check_status  = true;
    ataopts.smart_general_values = true;
    ataopts.smart_vendor_attrib  = scsiopts.smart_vendor_attrib = true;
    ataopts.smart_error_log      = scsiopts.smart_error_log     = true;
    ataopts.smart_selftest_log   = scsiopts.smart_selftest_log  = true;
    ataopts.smart_selective_selftest_log = true;
    scsiopts.smart_ss_media_log = true;

    /* Write data to pout_buffer */
    pout_buffer = new std::string;
    if (dev->is_scsi()) {
        scsiPrintMain(dev->to_scsi(), scsiopts);
    } else if (dev->is_ata()) {
        ataPrintMain(dev->to_ata(), ataopts);
    }

    /* Generate report */
    smart_report *result = (struct smart_report*)malloc(sizeof(smart_report));
    result->body = (char*)malloc(pout_buffer->size() + 1);
    strncpy(result->body, pout_buffer->c_str(), pout_buffer->size());
    delete pout_buffer;
    pout_buffer = NULL;

    return result;
}

/**
 * Free a smart_report, or do nothing if passed NULL
 */
void smart_report_free(struct smart_report *report)
{
    if (report) {
        free(report->body);
        free(report);
    }
}

} // extern C
