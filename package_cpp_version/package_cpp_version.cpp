// package_cpp_version.cpp : 定义控制台应用程序的入口点。

#include "stdafx.h"
#include "argopt.h"
#include <windows.h>
#include "package_cpp_version.h"
#include "parse_json.h"
#include "encrypt.h"
#include <algorithm>
#include <time.h>

#if PACKAGE_TEST
int opt_rw = 0;
int opt_rb = 0;
struct option_t options_test[] =
{
    {"rb", "-rb will help you rb-fy your source", NO_PARAM, NULL, 'r'},
    {"rw", "-rw is great to foobalooze the bazee", NO_PARAM, &opt_rw, 1},
    {"abc123", "-abc123 alphabetizes a lower set", NO_PARAM, NULL, 0},
    {"abc456", "-abc456 alphabetizes a higher set", NO_PARAM, NULL, 0},
    {"abc", NULL, NO_PARAM, NULL, 0},
    {"count", "-count <PARAMETER> enumerates the parameter", NEED_PARAM, NULL, 'c'},
    {"offset", "-offset [PARAMETER] set the offset for the optional paramater", OPTIONAL_PARAM, NULL, 'o'},
    {0, 0, NO_PARAM, 0}
};
#endif // PACKAGE_TEST

char *json_file_path = NULL;
char *json_input_file_path = "config.json";
struct option_t options[] =
{
    {"f", "-f is the path of configuration json file", NEED_PARAM, NULL, 'f'},
    {"h", "command help", NO_PARAM, NULL, 'h'},
    {0, 0, NO_PARAM, 0}
};

extern JSON_CONFIG_T g_json_config;
extern int pj_release_config_resource(void);
extern char *rand_string(int length);

void __cdecl dbg_trace(const char *fmt, ...)
{
    char buf[4096], *p = buf;
    va_list args;

    va_start(args, fmt);
    p += vsnprintf_s(p, sizeof(buf), _TRUNCATE, fmt, args);
    va_end(args);

    while (p > buf  &&  isspace(p[-1]))
        *--p = '\0';
    *p++ = '\r';
    *p++ = '\n';
    *p = '\0';

    OutputDebugStringA(buf);    //OutputDebugString
}

char *json_file_get_path(char *path)
{
    DWORD ret;
    char *json_path;
    char *output_path;
    TCHAR buf_tchar_ptr[MAX_PATH] = {0};
    char buf_char_ptr[MAX_PATH] = {0};
    char *colon_pos, *sprit_pos0, *sprit_pos1;

    ret = GetCurrentDirectory(MAX_PATH, buf_tchar_ptr);
    PKG_TPRINTF(_T("GetCurrentDirectory:%d, %s\r\n"), ret, buf_tchar_ptr);
    if(ret <= 0)
    {
        goto get_exit;
    }

    colon_pos = strchr(path, ':');
    sprit_pos0 = strchr(path, '/');
    sprit_pos1 = strchr(path, '\\');
    if(colon_pos && (sprit_pos0 || sprit_pos1))
    {
        json_path = path;
    }
    else
    {
        sprintf_s(buf_char_ptr, "%S", buf_tchar_ptr);
        strcat_s(buf_char_ptr, "\\");
        strcat_s(buf_char_ptr, path);
        json_path = buf_char_ptr;
    }

    output_path = _strdup(json_path);

get_exit:
    return output_path;
}


void file_attribute_printf(int attr)
{
    switch(attr)
    {
    case FILE_ATTRIBUTE_READONLY:
        PKG_PRINTF("FILE_ATTRIBUTE_READONLY\r\n");
        break;

    case FILE_ATTRIBUTE_HIDDEN:
        PKG_PRINTF("FILE_ATTRIBUTE_HIDDEN\r\n");
        break;

    case FILE_ATTRIBUTE_SYSTEM:
        PKG_PRINTF("FILE_ATTRIBUTE_SYSTEM\r\n");
        break;

    case FILE_ATTRIBUTE_DIRECTORY:
        PKG_PRINTF("FILE_ATTRIBUTE_DIRECTORY\r\n");
        break;

    case FILE_ATTRIBUTE_ARCHIVE:
        PKG_PRINTF("FILE_ATTRIBUTE_ARCHIVE\r\n");
        break;

    case FILE_ATTRIBUTE_DEVICE:
        PKG_PRINTF("FILE_ATTRIBUTE_DEVICE\r\n");
        break;

    case FILE_ATTRIBUTE_NORMAL:
        PKG_PRINTF("FILE_ATTRIBUTE_NORMAL\r\n");
        break;

    case FILE_ATTRIBUTE_TEMPORARY:
        PKG_PRINTF("FILE_ATTRIBUTE_TEMPORARY\r\n");
        break;

    case FILE_ATTRIBUTE_SPARSE_FILE:
        PKG_PRINTF("FILE_ATTRIBUTE_SPARSE_FILE\r\n");
        break;

    case FILE_ATTRIBUTE_REPARSE_POINT:
        PKG_PRINTF("FILE_ATTRIBUTE_REPARSE_POINT\r\n");
        break;

    case FILE_ATTRIBUTE_COMPRESSED:
        PKG_PRINTF("FILE_ATTRIBUTE_COMPRESSED\r\n");
        break;

    case FILE_ATTRIBUTE_OFFLINE:
        PKG_PRINTF("FILE_ATTRIBUTE_OFFLINE\r\n");
        break;

    case FILE_ATTRIBUTE_NOT_CONTENT_INDEXED:
        PKG_PRINTF("FILE_ATTRIBUTE_NOT_CONTENT_INDEXED\r\n");
        break;

    case FILE_ATTRIBUTE_ENCRYPTED:
        PKG_PRINTF("FILE_ATTRIBUTE_ENCRYPTED\r\n");
        break;

    case FILE_ATTRIBUTE_VIRTUAL:
        PKG_PRINTF("FILE_ATTRIBUTE_VIRTUAL\r\n");
        break;

    default:
        break;
    }
}

int json_file_is_exist(char *path)
{
    int len;
    DWORD ret;
    char *json_path;
    int exist_flag = 0;
    TCHAR buf_tchar_ptr[MAX_PATH] = {0};
    char buf_char_ptr[MAX_PATH] = {0};

    json_path = path;
#if USING_SWPRINTF_S
    len = swprintf_s(buf_tchar_ptr, sizeof(buf_tchar_ptr), L"%S", json_path);
    PKG_PRINTF("swprintf_s:%d\r\n", len);
#else
    do
    {
        len = MultiByteToWideChar(CP_ACP, 0, json_path, strlen(json_path) + 1, NULL, 0);
        MultiByteToWideChar(CP_ACP, 0, json_path, strlen(json_path) + 1, buf_tchar_ptr, len);
    }
    while(0);
    PKG_PRINTF("MultiByteToWideChar:%d\r\n", len);
#endif

    ret = GetFileAttributes(buf_tchar_ptr);
    PKG_TPRINTF(_T("GetFileAttributes:%d, (%s)\r\n"), ret, buf_tchar_ptr);
    if(-1 != ret)
    {
        exist_flag = 1;
        file_attribute_printf(ret);
    }

    return exist_flag;
}

int binary_fill(char *src, int src_len, char *dest, int dest_count, char padding)
{
    int count;

    printf("src:0x%x:%d dest:0x%x:%d \r\n", src, src_len, dest, dest_count);
    printf("padding:0x%02x\r\n", padding);

    memset(dest, padding, dest_count);

    count = min(src_len, dest_count);
    memcpy(dest, src, count);

    printf("dest:0x%02x:0x%02x\r\n", dest[0], dest[count - 1]);

    return dest_count;
}

int debug_tmp_file(char *buf, int buf_len)
{
    FILE *f;
    size_t wr_cnt;
    int tmp_name_flag = 0;
    char *tmp_name;
    char tmp_name_buf[20] = {0};
    char *tmp_name_head = rand_string(9);

    if(0 == tmp_name_head)
    {
        tmp_name = "default_tmp.bin";
    }
    else
    {
        tmp_name_flag = 1;
        strcat(tmp_name_buf, tmp_name_head);
        strcat(tmp_name_buf, ".bin");
        tmp_name = tmp_name_buf;
    }

    f = fopen(tmp_name, "wb+");
    if(NULL == f)
    {
        printf("create_tmp_file_failed:%s\r\n", tmp_name);
        return -2;
    }

    wr_cnt = fwrite(buf, 1, buf_len, f);
    if(buf_len != wr_cnt)
    {
        printf("tmp_fwrite:%d, %d\r\n", buf_len, wr_cnt);
    }

    fclose(f);

    if(tmp_name_flag && tmp_name_head)
    {
        free(tmp_name_head);
        tmp_name_head = 0;
    }

	return 0;
}

int package_binary_content(void)
{
    int sec_count;
    char *all_buf;
    int len, count;
    int ret, i;
    SECTION_T *sec_ptr;

    ret = 0;
    /* step0: prepare for data space*/
    len = pj_get_section_end_border();
    all_buf = (char *)malloc(len + 4);
    if(NULL == all_buf)
    {
        ret = 1;
        goto package_over;
    }

    memset(all_buf, 0xff, len);

    count = 0;
    /* step1: data organizations*/
    sec_count = g_json_config.section_count;
    for(i = 0; i < sec_count; i ++)
    {
    	int ret;
		
		#if CFG_TMP_FILE_AT_FILLING
		char *tmp_buf = &all_buf[count];
		int tmp_len;
		#endif
		
        sec_ptr = &g_json_config.section[i];		
		
        ret = binary_fill(sec_ptr->file_content, sec_ptr->file_size,
                             &all_buf[count], sec_ptr->size,
                             PAD_CONTENT);
		count += ret;
		
		#if CFG_TMP_FILE_AT_FILLING
		tmp_len = ret;
		if(tmp_len)
		{
			debug_tmp_file(tmp_buf, tmp_len);
		}
		#endif		
    }

    /* step2: persistent storage*/
    do
    {
        FILE *f;
        size_t wr_cnt;

        f = fopen(PACKAGE_FILE_NAME, "wb+");
        if(NULL == f)
        {
            printf("create_file_failed:%s\r\n", PACKAGE_FILE_NAME);
            break;
        }

        wr_cnt = fwrite(all_buf, 1, len, f);
        if(len != wr_cnt)
        {
            printf("fwrite:%d, %d\r\n", len, wr_cnt);
        }

        fclose(f);
    }
    while(0);

package_over:
    if(all_buf)
    {
        free(all_buf);
        all_buf = 0;
    }

    return ret;
}

int encrypt_binary_result(void)
{
    int ret;
    int argc = ENCRYPT_PARAM_CNT;
    char *argv[ENCRYPT_PARAM_CNT] =
    {
        "encrypt.exe",
        PACKAGE_FILE_NAME,
        "0",
        "0",
        "0",
        "0"
    };
    char *envp[1] = {0};

    ret = encrypt_main(argc, argv, envp);

    return ret;
}

int package_handler(char *path)
{
    int ret;

    if(pj_deal_with_json_file(path))
    {
        ret = 11;
        goto package_exit;
    }

    package_binary_content();

    encrypt_binary_result();

package_exit:
    pj_release_config_resource();

    return 0;
}

void printf_param(int argc, char *argv[])
{
    do
    {
        int i;
        for(i = 0; i < argc; i ++)
        {
            PKG_PRINTF("argv[%d] = %s\r\n", i , argv[i]);
        }

        PKG_PRINTF("\r\n\r\n");
    }
    while(0);
}

void printf_usage(void)
{
    PKG_PRINTF("merge_tool.exe -f config.json\r\n");
}

int main_package(int argc, char *argv[])
{
    int index = 0;
    int input_path_flag = 0;

    printf_param(argc, argv);

    /* get options_test */
    while (1)
    {
        int c;
        char *parameter;

        c = argopt(argc,
                   argv,
                   options,
                   &index,
                   &parameter);

        switch (c)
        {
        case ARGOPT_OPTION_FOUND :
            PKG_PRINTF("ARGOPT_OPTION_FOUND:%d %s\r\n", index, argv[index]);
            break;

        case ARGOPT_AMBIGIOUS_OPTION :
            argopt_completions(stdout,
                               "Ambigious option found. Possible completions:",
                               ++argv[index],
                               options);
            break;

        case ARGOPT_UNKNOWN_OPTION :
            fprintf(stdout, "Unknown option found:\t%s\n", argv[index]);
            argopt_help(stdout,
                        "Unknown option found",
                        argv[0],
                        options);
            break;

        case ARGOPT_NOT_OPTION :
            fprintf(stdout, "Bad or malformed option found:\t%s\n", argv[index]);
            argopt_help(stdout,
                        "Bad or malformed option found",
                        argv[0],
                        options);
            break;

        case ARGOPT_MISSING_PARAM :
            fprintf(stdout, "Option missing parameter:\t%s\n", argv[index]);
            argopt_help(stdout,
                        "Option missing parameter",
                        argv[0],
                        options);
            break;

        case ARGOPT_DONE :
            PKG_PRINTF("\nargv parameters parsed.\n");
            goto opt_done;

        case 0 :
            fprintf(stdout, "Option found:\t\t%s\n", argv[index]);
            break;

        case 'h':
            printf_usage();
            return 0;

        case 'f' :
            if(input_path_flag && json_input_file_path)
            {
                free(json_input_file_path);

                input_path_flag = 0;
                json_input_file_path = NULL;
            }

            json_input_file_path = _strdup(parameter);
            input_path_flag = 1;
            break;

        default:
            printf ("?? get_option() returned character code 0%o ??\n", c);
            printf_usage();
            break;
        }

        if (parameter)
        {
            free(parameter);
            parameter = NULL;
        }
    }

opt_done:
    /* main task*/
    json_file_path = json_file_get_path(json_input_file_path);
    if(!json_file_is_exist(json_file_path))
    {
        printf("\nconfiguration json file is exceptional!\n");
    }

    package_handler(json_file_path);

    /* exit task*/
    if(input_path_flag)
    {
        input_path_flag = 0;

        if(json_input_file_path)
        {
            free(json_input_file_path);
            json_input_file_path = NULL;
        }
    }

    if(json_file_path)
    {
        free(json_file_path);
        json_file_path = NULL;
    }

    if ((index) && (index < argc))
    {
        printf ("non-option argv-elements: ");

        while (index < argc)
            printf("%s ", argv[index++]);

        printf ("\n");

        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

#if PACKAGE_TEST
int main_test(int argc, char *argv[])
{
    /* current argv index */
    int index = 0;

    /* options_test with arguments */
    int count;
    int offset;

    printf_param(argc, argv);

    /* get options_test */
    while (1)
    {
        int c;
        char *parameter;

        c = argopt(argc,
                   argv,
                   options_test,
                   &index,
                   &parameter);

        switch (c)
        {
        case ARGOPT_OPTION_FOUND :
            fprintf(stdout, "%d - Option found:\t\t%s (*flag is not NULL)\n", __LINE__, argv[index]);
            break;

        case ARGOPT_AMBIGIOUS_OPTION :
            argopt_completions(stdout,
                               "Ambigious option found. Possible completions:",
                               ++argv[index],
                               options_test);
            break;

        case ARGOPT_UNKNOWN_OPTION :
            fprintf(stdout, "%d - Unknown option found:\t%s\n", __LINE__, argv[index]);
            argopt_help(stdout,
                        "Unknown option found",
                        argv[0],
                        options_test);
            break;

        case ARGOPT_NOT_OPTION :
            fprintf(stdout, "%d - Bad or malformed option found:\t%s\n", __LINE__, argv[index]);
            argopt_help(stdout,
                        "Bad or malformed option found",
                        argv[0],
                        options_test);
            break;

        case ARGOPT_MISSING_PARAM :
            fprintf(stdout, "%d - Option missing parameter:\t%s\n", __LINE__, argv[index]);
            argopt_help(stdout,
                        "Option missing parameter",
                        argv[0],
                        options_test);
            break;

        case ARGOPT_DONE :
            fprintf(stdout, "\nargv[] parsed.\n");
            goto opt_done;

        case 0 :
            fprintf(stdout, "%d - Option found:\t\t%s\n", __LINE__, argv[index]);
            break;

        case 'c' :
            count = strtol(parameter, NULL, 10);
            fprintf(stdout, "%d - Count option found:\t%s\n", __LINE__, argv[index - 1]);
            fprintf(stdout, "%d - Count parameter is:\t\"%s\"\n", __LINE__, parameter);
            break;

        case 'o' :
            if (parameter)
            {
                fprintf(stdout, "%d - Offset option found:\t%s\n", __LINE__, argv[index - 1]);
                fprintf(stdout, "%d - Offset parameter is:\t\"%s\"\n", __LINE__, parameter);
                offset = strtol(parameter, NULL, 16);
            }
            else
            {
                fprintf(stdout, "%d - Offset option found:\t%s\n", __LINE__, argv[index]);
            }
            break;

        case 'r' :
            fprintf(stdout, "%d - Option found:\t\t%s\n", __LINE__, argv[index]);
            opt_rb = 1;
            break;

        default:
            printf ("?? get_option() returned character code 0%o ??\n", c);
            break;
        }

        if (parameter)
            free(parameter);

        if (opt_rw)
        {
            fprintf(stdout, "-rw option found:\t%s\n", argv[index]);
            opt_rw = 0; /* purely cosmetic for this test progam */
        }
    }

opt_done:
    if ((index) && (index < argc))
    {
        printf ("non-option ARGV-elements: ");

        while (index < argc)
            printf("%s ", argv[index++]);

        printf ("\n");

        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
#endif // PACKAGE_TEST

int main(int argc, char *argv[])
{
#if PACKAGE_TEST
    return main_test(argc, argv);
#endif

    return main_package(argc, argv);
}
// eof

