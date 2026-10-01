#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <time.h>

// Source - https://stackoverflow.com/a/230068
// Posted by Graeme Perrow, modified by community. See post 'Timeline' for change history
// Retrieved 2026-09-30, License - CC BY-SA 4.0

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

#define MAXUPDATEFREQ 40

#ifdef __cplusplus
    extern "C" {
#endif

int updateWhenNeeded(char* dirPath, int cutOffDays);
int validatePath(char* path);
void stringifyDate(char* _date, int _year, int _month, int _day);
int updateFile(char* filePath, char* current_date);
int writeJSONtoString(const char** jsonString);
static size_t write_cb(char *contents, size_t size, size_t nmemb, void *stream);
void createFileHeader(char* filePath, const char* name, char* date);
int appendJSONtoCSV(char* filePath, const char* jsonString);
int checkHeaderDate(char* filePath, char* current_date, int cutOffRange);
int isOutOfCutOffRange(char* new_date, char* old_date, int cutOffRange);


// reference https://curl.se/libcurl/c/getinmemory.html for writing sec json to string
struct mem_chunk {
    char *memory;
    size_t size;
};

#ifdef __cplusplus
    }
#endif