
#ifndef SMART_REPORT_H
#define SMART_REPORT_H

#ifdef __cplusplus
extern "C" {
#endif

struct smart_report {
    char *body;
};

extern void smart_init();
extern struct smart_report *smart_report_create(char *device_path);
extern void smart_report_free(struct smart_report *report);

#ifdef __cplusplus
}
#endif

#endif // SMART_REPORT_H

