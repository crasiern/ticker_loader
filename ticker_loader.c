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

const char* SEC_URL = "https://www.sec.gov/files/company_tickers.json";
static const char filename[] = "../company_tickers.csv";

// reference https://curl.se/libcurl/c/getinmemory.html for writing sec json to string
struct mem_chunk {
    char *memory;
    size_t size;
};

int main()
{
    time_t now = time(NULL);  
    struct tm *current_time = localtime(&now);
    char current_date[11];
    dateStringified(current_date, current_time->tm_year, current_time->tm_mon, current_time->tm_mday);
    if(needsUpdate(current_date) == 0) 
        return 0;
    
    addHeader("company_tickers", current_date);
    const char* jsonString = NULL;
    retrieveFile(&jsonString);
    JSONtoCSV(jsonString);
    return 0;
}

int retrieveFile(const char** jsonPointer)
{
    curl_global_init(CURL_GLOBAL_ALL);
    CURLcode result;

    CURL* handle = curl_easy_init();
    struct mem_chunk jsonChunk;

    jsonChunk.memory = malloc(1); 
    jsonChunk.size = 0;

    curl_easy_setopt(handle, CURLOPT_URL, SEC_URL);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "ticker_loader crasiern@gmail.com");
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&jsonChunk);
    curl_easy_perform(handle);

    *jsonPointer = jsonChunk.memory;

    curl_easy_cleanup(handle);
    curl_global_cleanup();
    return (int) result;
}

// borrowed from https://curl.se/libcurl/c/url2file.html
// writes curl output into file
static size_t write_cb(char *contents, size_t size, size_t nmemb, void *stream)
{
    size_t realsize = size * nmemb;
    struct mem_chunk *mem = (struct mem_chunk *)stream;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
    // change to error that halts program
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int JSONtoCSV(const char* jsonString)
{
    // may not need const
    const cJSON *company_index = NULL;
    int status = 0;
    
    cJSON* tickers_json = cJSON_Parse(jsonString);
    if (tickers_json == NULL)
        return 404;
    
    FILE *csvFile = fopen(filename, "a");
    // create header

    int index = 0;
    for (;;)
    {
        // hopefully 8 is enough, needs \0
        char indexOfJSON[8];
        snprintf(indexOfJSON, sizeof(indexOfJSON), "%d", index);
        company_index = cJSON_GetObjectItemCaseSensitive(tickers_json, indexOfJSON);

        if (company_index == NULL)
            break;

        cJSON *cik_str = company_index->child;
        cJSON *ticker = cik_str->next;
        cJSON *title =ticker->next;
        fprintf(csvFile, "%d,%s,%s\n", cik_str->valueint, ticker->valuestring, title->valuestring);
        
        index++;
    }

    cJSON_Delete(tickers_json);
    fclose(csvFile);
}

int needsUpdate(char* current_date)
{
    FILE *tickerFile = fopen(filename, "r");
    if (tickerFile == NULL)
    {
        fclose(tickerFile);
        return 1;
    }
    
    char headerLine[256];
    char* reszult = fgets(headerLine, sizeof(headerLine), tickerFile);
    char *last_date;
    strtok(headerLine, ",\n");
    last_date = strtok(NULL, ",\n");
    int result = moreThanMonth(current_date, last_date);
    fclose(tickerFile);
    return result;
}

void dateStringified(char* _date, int _year, int _month, int _day)
{
    int formatSize = 11; // ex. '1990 09 11\0' string requires 11 characters
    int nyear = _year + 1900;
    int nmonth = _month + 1;
    snprintf(_date, formatSize, "%04d %02d %02d", nyear, nmonth, _day);
}

// first date > second date
int moreThanMonth(char* firstDate, char* secondDate)
{
    int year1, month1, day1, year2, month2, day2;
    sscanf(firstDate, "%d" "%d" "%d", &year1, &month1, &day1);
    sscanf(secondDate, "%d" "%d" "%d", &year2, &month2, &day2);

    // greater than 2 guarentees age, not month/year roll over
    if ((year1 - year2) > 2)
        return 1;
    if ((month1 - month2) > 2)
        return 1;
    
    int daysPast = 0;
    int assumedDaysInMonth = 30;
    int monthsInYears = 12;
    while (daysPast < UPDATEFREQ)
    {
        if (month1 == month2 && day1 == day2)
            return 0;

        day1--;

        if (day1 == 0)
        {
            day1 = assumedDaysInMonth;
            month1--;
            if (month1 == 0)
            {
                month1 = monthsInYears;
                year1--;
            }
        }

        daysPast++;     
    }
    
    return 1;
}

void addHeader(char* name, char* date)
{
    char* delim = ",";
    FILE *tickerFile = fopen(filename, "w");
    fprintf(tickerFile, "%s,%s\n", name, date);
    fclose(tickerFile);
}