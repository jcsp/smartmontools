
#include <assert.h>
#include <stdio.h>

#include "smart_report.h"

int main(int argc, char **argv)
{
    assert(argc == 2);
    smart_init();
    struct smart_report *report = smart_report_create(argv[1]);
    if (report) {
        printf("Got report: %s\n", report->body);
    } else {
        printf("Failed to get report\n");
    }
    smart_report_free(report);

    return 0;
}

