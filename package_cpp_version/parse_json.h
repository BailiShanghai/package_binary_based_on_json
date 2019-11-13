#ifndef _PARSE_JSON_H_
#define _PARSE_JSON_H_

#define JSON_DEBUG             1
#ifdef JSON_DEBUG
#define PJ_PRINTF              dbg_trace
#define PJ_TPRINTF             _tprintf
#else
#define PJ_PRINTF(...)
#define PJ_TPRINTF(...)
#endif

#define USING_FOPEN                 0
#define CFG_TEMP_FILE               0

#define KEY_MAGIC                    "magic"
#define KEY_VERSION                  "version"
#define KEY_SECTION                  "section"
#define KEY_COUNT                    "count"
#define KEY_FIRMWARE                 "firmware"
#define KEY_SECTION_VERSION          "version"
#define KEY_PARTITION                "partition"
#define KEY_START_ADDR               "start_addr"
#define KEY_SIZE                     "size"

#define PAD_CONTENT                  ('\xFF')

typedef struct _section_t_
{
    char *file_name;
    char *file_version;
    char *file_content;
    int file_size;

    char *partition;
    int start_addr;
    int size;
} SECTION_T;

typedef struct _config_t_
{
    char *magic_str;
    char *version;
    int section_count;
    SECTION_T *section;
} JSON_CONFIG_T;

extern int pj_deal_with_json_file(char *path);
extern int pj_get_section_end_border(void);
extern void __cdecl dbg_trace(const char *fmt, ...);
#endif // _PARSE_JSON_H_
// eof

