#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <time.h>

#define UPDATEFREQ 40

int retrieveFile(const char** jsonPointer);
static size_t write_cb(char *contents, size_t size, size_t nmemb, void *stream);
int JSONtoCSV(const char* jsonString);
int needsUpdate(char* current_date);
void dateStringified(char* _date, int _year, int _month, int _day);
int moreThanMonth(char* firstDate, char* secondDate);
void addHeader(char* name, char* date);

// reference https://curl.se/libcurl/c/getinmemory.html for writing sec json to string
struct mem_chunk {
    char *memory;
    size_t size;
};