#ifndef _PACKAGE_CPP_VERSION_H_
#define _PACKAGE_CPP_VERSION_H_

#define PACKAGE_DEBUG          1
#ifdef PACKAGE_DEBUG
#define PKG_PRINTF             printf
#define PKG_TPRINTF             _tprintf
#else
#define PKG_PRINTF(...)
#define PKG_TPRINTF(...)
#endif

#define PACKAGE_TEST                       0

#define PACKAGE_FILE_NAME                  "package_all.bin"
#define PACKAGE_CRC_FILE_NAME              "package_all_crc.bin"
#define ENCRYPT_PARAM_CNT	               6

#ifndef FILE_ATTRIBUTE_READONLY
#define FILE_ATTRIBUTE_READONLY            1 //只读
#define FILE_ATTRIBUTE_HIDDEN              2 //隐藏
#define FILE_ATTRIBUTE_SYSTEM              4 //系统
#define FILE_ATTRIBUTE_DIRECTORY           16 //目录
#define FILE_ATTRIBUTE_ARCHIVE             32 //存档
#define FILE_ATTRIBUTE_DEVICE              64 //保留
#define FILE_ATTRIBUTE_NORMAL              128 //正常
#define FILE_ATTRIBUTE_TEMPORARY           256 //临时
#define FILE_ATTRIBUTE_SPARSE_FILE         512 //稀疏文件
#define FILE_ATTRIBUTE_REPARSE_POINT       1024 //超链接或快捷方式
#define FILE_ATTRIBUTE_COMPRESSED          2048 //压缩
#define FILE_ATTRIBUTE_OFFLINE             4096 //脱机
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 8192 //索引
#define FILE_ATTRIBUTE_ENCRYPTED           16384 //加密
#define FILE_ATTRIBUTE_VIRTUAL             65536 //虚拟
#endif

#endif // _PACKAGE_CPP_VERSION_H_
// eof

