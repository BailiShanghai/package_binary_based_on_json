#include "stdafx.h"
#include "parse_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <windows.h>
#include <time.h>

JSON_CONFIG_T g_json_config = {0};
char *json_content_ptr = NULL;

char *rand_string(int length)
{
    char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    size_t stringLen = 26 * 2 + 10 + 7;
    char *randomString;

    randomString = (char *)malloc(sizeof(char) * (length + 1));
    if (!randomString)
    {
        return (char *)0;
    }

    unsigned int key = 0;

    for (int n = 0; n < length; n++)
    {
        key = rand() % stringLen;
        randomString[n] = string[key];
    }

    randomString[length] = '\0';

    return randomString;
}

int tmp_file(char *buf, int buf_len)
{
    FILE *f;
    size_t wr_cnt;
    int tmp_name_flag = 0;
    char *tmp_name;
    char tmp_name_buf[20] = {0};
    char *tmp_name_head = rand_string(8);

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
        return -3;
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

int pj_get_section_end_border(void)
{
    SECTION_T *sec_ptr;

    sec_ptr = &g_json_config.section[g_json_config.section_count - 1];
    if(NULL == sec_ptr)
    {
        return 0;
    }

    return sec_ptr->start_addr + sec_ptr->size;
}

char *pj_create_section(int count)
{
    char *sec_ptr;

    sec_ptr = (char *)malloc(count * sizeof(SECTION_T));

    return sec_ptr;
}

void pj_destroy_secton(char *sec_ptr)
{
    free(sec_ptr);
}

int pj_create_json_file_content(char *path)
{
#if (0 == USING_FOPEN)
    errno_t err;
#endif

    int len;
    size_t count;
    FILE *f_json_handler;

    if(NULL == path)
    {
        return -1;
    }

#if USING_FOPEN
    f_json_handler = fopen(path, "rt");
    if(NULL == f_json_handler)
    {
        return -2;
    }
#else
    err = fopen_s(&f_json_handler, path, "rt");
    if(0 != err)
    {
        PJ_PRINTF("fopen_s:%d\r\n", err);
        return -2;
    }
#endif

    fseek(f_json_handler, 0, SEEK_END);
    len = ftell(f_json_handler);

    PJ_PRINTF("path:%s\r\n", path);
    PJ_PRINTF("f_json_handler:0x%x\r\n", f_json_handler);
    PJ_PRINTF("len:%d\r\n", len);

    json_content_ptr = (char *)malloc(len + 4);
    if(NULL == json_content_ptr)
    {
        fclose(f_json_handler);
        return -3;
    }

    memset(json_content_ptr, 0, len + 2);

    fseek(f_json_handler, 0, SEEK_SET);
#if USING_FOPEN
    count = fread(json_content_ptr, 1, len, f_json_handler);
#else
    count = fread(json_content_ptr, 1, len, f_json_handler);
#endif

    if(0 == count)
    {
        PJ_PRINTF("read json file exceptionally\r\n");
    }

    PJ_PRINTF("===================================================\r\n");
    PJ_PRINTF("json_content:%s\r\n", json_content_ptr);
    PJ_PRINTF("===================================================\r\n\r\n\r\n");
    fclose(f_json_handler);

    return 0;
}

int pj_destroy_json_file_content(void)
{
    if(json_content_ptr)
    {
        free(json_content_ptr);
        json_content_ptr = NULL;
    }

    return 0;
}

int pj_string_to_value(char *str)
{
    errno_t err;
    char *dup_str;
    int value = 0;
    char *lower_str;
    char *str_0x_pos;
    char *str_k_pos;
    char *str_m_pos;
    int include_k_flag = 0;
    int include_m_flag = 0;
    int include_0x_flag = 0;
    int val_factor = 1;
    int k_factor = 1024;
    int m_factor = 1024 * 1024;

    if((NULL == str) || (0 == strlen(str)))
    {
        goto to_exit;
    }

    /* prepare for string*/
    dup_str = strdup(str);
    if(NULL == dup_str)
    {
        goto to_exit;
    }

    /* convert the string to lowercase*/
    printf("dup_str:%s\r\n", dup_str);
    err = _strlwr_s(dup_str, strlen(dup_str) + 1);
    if(err)
    {
        printf("_strlwr_s failed\r\n");
        goto to_exit;
    }
    lower_str = dup_str;
    printf("lower_str:%s\r\n", lower_str);

    /* check the sub-string*/
    str_0x_pos = strstr(lower_str, "0x");
    if(str_0x_pos)
    {
        include_0x_flag = 1;
    }

    str_k_pos = strstr(lower_str, "k");
    if(str_k_pos)
    {
        include_k_flag = 1;
        val_factor = k_factor;
    }

    str_m_pos = strstr(lower_str, "m");
    if(str_m_pos)
    {
        include_m_flag = 1;
        val_factor = m_factor;
    }

    if((str_k_pos) && (str_m_pos))
    {
        goto to_exit;
    }

    /* handle the data*/
    if(str_0x_pos)
    {
        value = strtoul(lower_str, NULL, 16);
    }
    else if((str_k_pos) || (str_m_pos))
    {
        lower_str[strlen(lower_str) - 1] = 0;
        value = strtoul(lower_str, NULL, 10);
    }
    else
    {
        value = strtoul(lower_str, NULL, 10);
    }

    value = value * val_factor;

to_exit:
    /* release the string buffer of dup*/
    if(dup_str)
    {
        free(dup_str);
    }

    return value;
}

int pr_print_config(void)
{
    int count, i;
    SECTION_T *sec_ptr;

    printf("\r\nKEY_MAGIC:%s\r\n", g_json_config.magic_str);
    printf("KEY_VERSION:%s\r\n", g_json_config.version);
    printf("KEY_COUNT:%d\r\n", g_json_config.section_count);
    printf("KEY_SECTION:0x%x\r\n", g_json_config.section);

    count = g_json_config.section_count;
    for(i = 0; i < count; i ++)
    {
        sec_ptr = &g_json_config.section[i];

        printf("KEY_FIRMWARE[%d]:%s\r\n", i, sec_ptr->file_name);
        printf("KEY_SECTION_VERSION[%d]:%s\r\n", i, sec_ptr->file_version);
        printf("KEY_PARTITION[%d]:%s\r\n", i, sec_ptr->partition);
        printf("KEY_START_ADDR[%d]:0x%x\r\n", i, sec_ptr->start_addr);
        printf("KEY_SIZE[%d]:%d\r\n\r\n", i, sec_ptr->size);
    }
    printf("\r\n");

    return 0;
}

int pj_print_json(cJSON *root)
{
    int i;
    char *text;
    cJSON *item;

    if(NULL == root)
    {
        return -6;
    }

    for(i = 0; i < cJSON_GetArraySize(root); i ++)
    {
        item = cJSON_GetArrayItem(root, i);
        if(cJSON_Object == item->type)
        {
            pj_print_json(item);
        }
        else
        {
            text = cJSON_Print(item);
            printf("%s:", item->string);
            printf("%s\n", text);

            if(text)
            {
                free(text);
                text = 0;
            }
        }
    }

    return 0;
}

int pj_parse_section_fields(cJSON *root)
{
    int i;
    cJSON *item;
    int array_cnt;
    SECTION_T *sec_ptr;
    cJSON *section_item;

    array_cnt = cJSON_GetArraySize(root);

    /* alloc the space*/
    sec_ptr = (SECTION_T *)malloc(array_cnt * sizeof(SECTION_T));
    if(NULL == sec_ptr)
    {
        return 1;
    }

    g_json_config.section = sec_ptr;
    g_json_config.section_count = array_cnt;

    /* parse and set the parameters*/
    for(i = 0; i < array_cnt; i ++)
    {
        item = cJSON_GetArrayItem(root, i);

        section_item = cJSON_GetObjectItem(item, KEY_FIRMWARE);
        sec_ptr[i].file_name = strdup(section_item->valuestring);

        section_item = cJSON_GetObjectItem(item, KEY_SECTION_VERSION);
        sec_ptr[i].file_version = strdup(section_item->valuestring);

        section_item = cJSON_GetObjectItem(item, KEY_PARTITION);
        sec_ptr[i].partition = strdup(section_item->valuestring);

        section_item = cJSON_GetObjectItem(item, KEY_START_ADDR);
        sec_ptr[i].start_addr = pj_string_to_value(section_item->valuestring);

        section_item = cJSON_GetObjectItem(item, KEY_SIZE);
        sec_ptr[i].size = pj_string_to_value(section_item->valuestring);
    }

    return 0;
}

int pj_parse_section_fields_check(cJSON *root)
{
    int i;
    cJSON *item;
    int ret = 0;
    int array_cnt;
    cJSON *section_item;

    array_cnt = cJSON_GetArraySize(root);
    if(0 == array_cnt)
    {
        ret = 1;
        goto fields_check_exit;
    }

    for(i = 0; i < array_cnt; i ++)
    {
        item = cJSON_GetArrayItem(root, i);
        if(NULL == item)
        {
            ret = 1;
            break;
        }

        section_item = cJSON_GetObjectItem(item, KEY_FIRMWARE);
        if(NULL == section_item)
        {
            ret = 1;
            goto fields_check_exit;
        }
        printf("firmware:%s\r\n", section_item->valuestring);

        section_item = cJSON_GetObjectItem(item, KEY_SECTION_VERSION);
        if(NULL == section_item)
        {
            ret = 1;
            goto fields_check_exit;
        }
        printf("section_version:%s\r\n", section_item->valuestring);

        section_item = cJSON_GetObjectItem(item, KEY_PARTITION);
        if(NULL == section_item)
        {
            ret = 1;
            goto fields_check_exit;
        }
        printf("partition_name:%s\r\n", section_item->valuestring);

        section_item = cJSON_GetObjectItem(item, KEY_START_ADDR);
        if(NULL == section_item)
        {
            ret = 1;
            goto fields_check_exit;
        }
        printf("start_address:%s, 0x%08x\r\n", section_item->valuestring, pj_string_to_value(section_item->valuestring));

        section_item = cJSON_GetObjectItem(item, KEY_SIZE);
        if(NULL == section_item)
        {
            ret = 1;
            goto fields_check_exit;
        }
        printf("size:%s, 0x%08x\r\n", section_item->valuestring, pj_string_to_value(section_item->valuestring));
    }

    (void)i;

fields_check_exit:
    return ret;
}

int pj_parse_check(cJSON *root)
{
    int count;
    int ret = 0;
    cJSON *item;

    item = cJSON_GetObjectItem(root, KEY_MAGIC);
    if(NULL == item)
    {
        ret = 1;
        goto check_exit;
    }
    printf("magic:%s\r\n", item->valuestring);

    item = cJSON_GetObjectItem(root, KEY_VERSION);
    if(NULL == item)
    {
        ret = 1;
        goto check_exit;
    }
    printf("version:%s\r\n", item->valuestring);

    item = cJSON_GetObjectItem(root, KEY_COUNT);
    if(NULL == item)
    {
        ret = 1;
        goto check_exit;
    }

    count = item->valueint;
    if((count < 0) || (count > 10))
    {
        ret = 1;
        goto check_exit;
    }
    printf("count:%d\r\n", item->valueint);

    item = cJSON_GetObjectItem(root, KEY_SECTION);
    if(NULL == item)
    {
        ret = 1;
        goto check_exit;
    }

    ret = pj_parse_section_fields_check(item);

check_exit:
    return ret;
}


int pj_parse_detail(cJSON *root)
{
    int ret = 0;
    cJSON *item;

    item = cJSON_GetObjectItem(root, KEY_MAGIC);
    g_json_config.magic_str = strdup(item->valuestring);

    item = cJSON_GetObjectItem(root, KEY_VERSION);
    g_json_config.version = strdup(item->valuestring);

    item = cJSON_GetObjectItem(root, KEY_COUNT);
    g_json_config.section_count = item->valueint;

    item = cJSON_GetObjectItem(root, KEY_SECTION);
    ret = pj_parse_section_fields(item);

    return ret;
}

int pj_section_address_check(void)
{
    int ret = 0;
    int i, count;
    int tmp_addr;
    int address = -1;
    SECTION_T *sec_ptr;

    count = g_json_config.section_count;
    for(i = 0; i < count; i ++)
    {
        sec_ptr = &g_json_config.section[i];
        tmp_addr = sec_ptr->start_addr;
        if(address > tmp_addr)
        {
            ret = 1;
            break;
        }

        address = tmp_addr;
    }

    return ret;
}

int pj_include_dot_slash_count(char *string, char *dot_slash)
{
    int count = 0;
    char *tmp_str;

    tmp_str = string;
    while(tmp_str && dot_slash)
    {
        char *pos;

        pos = strstr(tmp_str, dot_slash);
        if(NULL == pos)
        {
            break;
        }

        count ++;
        tmp_str = &pos[2];
    }

    return count;
}

int pj_get_file_info(char *dir_path, char *file_path, char **content)
{
    char *buf;
    errno_t err;
    int rd_count;
    FILE *file_hdl;
    int ret;
    int file_count;
    int len, cat_cnt;
    char *slash_pos;
    char *actual_path = NULL;
    char *current_path = NULL;

    ret = 0;
    file_count = 0;
    *content = NULL;
    printf("\r\n          dir_path:%s\r\n", dir_path);
    printf("          file_path:%s\r\n", file_path);

    /* step0: get file path*/
    slash_pos = strchr(file_path, '\\');
    if(NULL == slash_pos)
    {
        len = strlen(dir_path) + strlen(file_path) + 4;
        actual_path = (char *)malloc(len);
        if(NULL == actual_path)
        {
            ret = 1;
            goto get_over;
        }
		memset(actual_path, 0, len);

        cat_cnt = _snprintf(actual_path, len, "%s%s", dir_path, file_path);
        if(cat_cnt < 0)
        {
            ret = 2;
            goto get_over;
        }

        printf("          actual_path:%s\r\n", actual_path);
    }
    else
    {
        DWORD val_ret;
        char *dot_dot_slash_pos;
        int i, current_path_len;
        char *slash_ptr, *new_file_path;
        char buf_char_ptr[MAX_PATH] = {0};
        TCHAR buf_tchar_ptr[MAX_PATH] = {0};
        int dot_slash_cnt, dot_dot_slash_cnt;
        int actual_path_len;

        val_ret = GetCurrentDirectory(MAX_PATH, buf_tchar_ptr);
        if(val_ret <= 0)
        {
            ret = 3;
            goto get_over;
        }
        sprintf_s(buf_char_ptr, "%S", buf_tchar_ptr);
        current_path = strdup(buf_char_ptr);

        printf("          current_path:%s\r\n", current_path);
        dot_slash_cnt = pj_include_dot_slash_count(file_path, ".\\");
        dot_dot_slash_cnt = pj_include_dot_slash_count(file_path, "..\\");
        printf("          dot:%d:%d\r\n", dot_slash_cnt, dot_dot_slash_cnt);

        current_path_len = strlen(current_path);
        if('\\' == current_path[current_path_len - 1])
        {
            current_path[current_path_len - 1] = 0;
        }

        for(i = 0; i < dot_dot_slash_cnt; i ++)
        {
            slash_ptr = strrchr(current_path, '\\');
            if(NULL == slash_ptr)
            {
                break;
            }

            slash_ptr[0] = 0;
        }
        printf("          current_path2:%s\r\n", current_path);

        new_file_path = file_path;
        for(i = 0; i < dot_dot_slash_cnt; i ++)
        {
            dot_dot_slash_pos = strstr(new_file_path, "..\\");
            if(NULL == dot_dot_slash_pos)
            {
                break;
            }

            dot_dot_slash_pos[0] = 0;
            dot_dot_slash_pos[1] = 0;
            new_file_path = &dot_dot_slash_pos[2];
        }
        printf("          new_file_path:%s\r\n", new_file_path);

        for(i = 0; i < (dot_slash_cnt - dot_dot_slash_cnt); i ++)
        {
            slash_ptr = strstr(new_file_path, ".\\");
            if(NULL == slash_ptr)
            {
                break;
            }

            slash_ptr[0] = 0;
            new_file_path = &slash_ptr[1];
        }
        printf("          new_file_path2:%s\r\n", new_file_path);

        actual_path_len = strlen(current_path) + strlen(new_file_path) + 4;
        actual_path = (char *)malloc(actual_path_len);
        if(NULL == actual_path)
        {
            ret = 4;
            goto get_over;
        }

        cat_cnt = _snprintf(actual_path, actual_path_len, "%s%s", current_path, new_file_path);
        if(cat_cnt < 0)
        {
            ret = 5;
            goto get_over;
        }
        printf("          actual_path:%s\r\n", actual_path);
    }

    /* step1: get file size and content*/
    file_count = 0;
    do
    {
        err = fopen_s(&file_hdl, actual_path, "rb");
        if(0 != err)
        {
            ret = 6;
            break;
        }
        fseek(file_hdl, 0, SEEK_END);
        len = ftell(file_hdl);

        buf = (char *)malloc(len + 4);
        if(NULL == buf)
        {
            ret = 7;
            fclose(file_hdl);

            break;
        }
        memset(buf, 0xff, len + 4);

        fseek(file_hdl, 0, SEEK_SET);
        rd_count = fread(buf, 1, len, file_hdl);
        if(0 == rd_count)
        {
            ret = 8;
            fclose(file_hdl);

            break;
        }

        if(len != rd_count)
        {
            printf("fread_warning:%d:%d\\r\n", actual_path, rd_count);
        }

        printf("          fread:%s\r\n", actual_path);
        printf("          fread: 0x%x:%d\r\n", file_hdl, rd_count);
        fclose(file_hdl);

        file_count = len;
        *content = buf;
    }while(0);
	
#if CFG_TEMP_FILE
	tmp_file(buf, len);
#endif

get_over:
    printf("          ret:%d\r\n", ret);
    if(current_path)
    {
        free(current_path);
        current_path = NULL;
    }

    if(actual_path)
    {
        free(actual_path);
        actual_path = NULL;
    }

    return file_count;
}

int pj_section_get_files_size(char *json_path)
{
    int i, count;
    int ret = 0;
    char *tmp_path;
    char *slash_pos;
    SECTION_T *sec_ptr;
    char *directory_path = NULL;

    printf("pj_section_get_file_size\r\n");
    printf("          json_path:%s\r\n", json_path);
    if(NULL == json_path)
    {
        ret = 1;
        goto get_exit;
    }

    tmp_path = strdup(json_path);
    if(NULL == tmp_path)
    {
        ret = 2;
        goto get_exit;
    }

    slash_pos = strrchr(tmp_path, '\\');
    printf("          slash_pos:%s\r\n", slash_pos);
    if(NULL == slash_pos)
    {
        ret = 3;
        goto get_exit;
    }
    slash_pos[1] = 0;
    directory_path = tmp_path;
    printf("          directory_path:%s\r\n", directory_path);

    count = g_json_config.section_count;
    for(i = 0; i < count; i ++)
    {
        sec_ptr = &g_json_config.section[i];
        sec_ptr->file_size = pj_get_file_info(directory_path, sec_ptr->file_name,
                                              &sec_ptr->file_content);
        printf("          section_file:%d 0x%x\r\n", sec_ptr->file_size, sec_ptr->file_content);
        if(0 == sec_ptr->file_size)
        {
            ret = 4;
            break;
        }
    }

get_exit:
    if(tmp_path)
    {
        free(tmp_path);
        tmp_path = NULL;
    }

    return ret;
}

int pj_section_size_check(void)
{
    return 0;
}

int pj_release_config_resource(void)
{
    int i;
    int count;
    SECTION_T *sec_ptr;

    if(g_json_config.magic_str)
    {
        free(g_json_config.magic_str);
        g_json_config.magic_str = NULL;
    }

    if(g_json_config.version)
    {
        free(g_json_config.version);
        g_json_config.version = NULL;
    }

    count = g_json_config.section_count;
    for(i = 0; i < count; i ++)
    {
        sec_ptr = &g_json_config.section[i];

        if(sec_ptr->file_name)
        {
            free(sec_ptr->file_name);
            sec_ptr->file_name = NULL;
        }

        if(sec_ptr->file_version)
        {
            free(sec_ptr->file_version);
            sec_ptr->file_version = NULL;
        }

        if(sec_ptr->file_content)
        {
            free(sec_ptr->file_content);
            sec_ptr->file_content = NULL;
        }

        if(sec_ptr->partition)
        {
            free(sec_ptr->partition);
            sec_ptr->partition = NULL;
        }

        sec_ptr->size = 0;
        sec_ptr->file_size = 0;
        sec_ptr->start_addr = 0;
    }

    if(g_json_config.section)
    {
        free(g_json_config.section);
        g_json_config.section = NULL;
    }

    g_json_config.section_count = 0;

    return 0;
}

int pj_parse_handler(char *json_path)
{
#ifdef CONFIG_PRINT_JSON_INFO
    char *text;
#endif

    int ret = 0;
    cJSON *root;

    printf("pj_parse_handler: 0x%x\r\n", json_content_ptr);
    if(NULL == json_content_ptr)
    {
        ret = -4;
        goto parse_over;
    }

    root = cJSON_Parse(json_content_ptr);
    if(NULL == root)
    {
        printf("parse_failed: [%s]", cJSON_GetErrorPtr());
        ret = -5;
        goto parse_over;
    }

#ifdef CONFIG_PRINT_JSON_INFO
    text = cJSON_Print(root);
    if(text)
    {
        printf("%s\r\n", text);
        free(text);
    }
    printf("cjson_print_over:0x%x\r\n\r\n\r\n", root);

    pj_print_json(root);
    printf("pj_print_over\r\n\r\n");
#endif // CONFIG_PRINT_JSON_INFO

    ret = pj_parse_check(root);
    if(0 != ret)
    {
        goto parse_over;
    }

    ret = pj_parse_detail(root);
    if(0 != ret)
    {
        goto parse_over;
    }

#if JSON_DEBUG
    pr_print_config();
#endif

    ret = pj_section_address_check();
    if(0 != ret)
    {
        goto parse_over;
    }

    ret = pj_section_get_files_size(json_path);
    if(0 != ret)
    {
        goto parse_over;
    }

    ret = pj_section_size_check();
    if(0 != ret)
    {
        goto parse_over;
    }

parse_over:
    return ret;
}

int pj_deal_with_json_file(char *path)
{
    int ret;

    pj_create_json_file_content(path);
    ret = pj_parse_handler(path);
    pj_destroy_json_file_content();

    return ret;
}

// eof

