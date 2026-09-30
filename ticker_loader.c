#include "ticker_loader.h"

const char* SEC_URL = "https://www.sec.gov/files/company_tickers.json";
static const char filename[] = "company_tickers.csv";

/*
    TODO:
    - make into pybind11 library
    - standardize error codes in defines (you pass forward CURL error codes)
*/

int main()
{
    int status = updateWhenNeeded("X:/codeSpace/magi_project/ticker_loader/", MAXUPDATEFREQ);
    printf("status: %d", status);
    return 0;
}

int updateWhenNeeded(char* dirPath, int cutOffDays)
{
    int status = 0;
    if (validatePath(dirPath) != 0)
        return 2;

    // cap cutOffDays at the max update frequency
    cutOffDays = (cutOffDays > MAXUPDATEFREQ) ? MAXUPDATEFREQ : cutOffDays;

    // add one for \0
    char filePath[strlen(dirPath) + strlen(filename) + 1];
    snprintf(filePath, sizeof(filePath), "%s%s", dirPath, filename); // concatonate for full fill path, maybe should check for / eariler...

    time_t now = time(NULL);  
    struct tm *current_time = localtime(&now);
    char current_date[11];
    stringifyDate(current_date, current_time->tm_year, current_time->tm_mon, current_time->tm_mday);
    // if file does not exist, update to create it
    if (validatePath(filePath) != 0)
    {
        status = updateFile(filePath, current_date);
        return status;
    }

    status = checkHeaderDate(filePath, current_date, cutOffDays);
    if (status == 1)
    {
        status = updateFile(filePath, current_date);
    }
        
    return status;
}

// verify a directory or file exists and we have access to it
int validatePath(char* path)
{
    // Source - https://stackoverflow.com/a/230068
    // Posted by Graeme Perrow, modified by community. See post 'Timeline' for change history
    // Retrieved 2026-09-30, License - CC BY-SA 4.0
    return access(path, F_OK);
}

// converts raw time to a string date in form of "year/month/date"
// should try to verify that _date is 11 chars long...
void stringifyDate(char* _date, int _year, int _month, int _day)
{
    int formatSize = 11; // ex. '1990 09 11\0' string requires 11 characters
    int nyear = _year + 1900;
    int nmonth = _month + 1;
    snprintf(_date, formatSize, "%04d/%02d/%02d", nyear, nmonth, _day);
    return;
}

// rewrites csv file with updated info
int updateFile(char* filePath, char* current_date)
{
    const char* title = "company_tickers";
    const char* jsonString = NULL;
    int status_code = 0;
    status_code = writeJSONtoString(&jsonString); 
    if (status_code != 0)
        return status_code;
    createFileHeader(filePath, title, current_date);
    status_code = appendJSONtoCSV(filePath, jsonString);
    return status_code;
}

int writeJSONtoString(const char** jsonString)
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
    result = curl_easy_perform(handle);

    *jsonString = jsonChunk.memory;

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
    // if returning 0, then error occured
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

void createFileHeader(char* filePath, const char* name, char* date)
{
    char* delim = ",";
    FILE *tickerFile = fopen(filePath, "w");
    fprintf(tickerFile, "%s,%s\n", name, date);
    fclose(tickerFile);
    return;
}

// appends a JSON String into a csv file (that should have a header)
int appendJSONtoCSV(char* filePath, const char* jsonString)
{
    const cJSON *company_index = NULL;
    int status = 0;
    
    cJSON* tickers_json = cJSON_Parse(jsonString);
    if (tickers_json == NULL)
        return 404;
    
    FILE *csvFile = fopen(filePath, "a");
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
    return 0;
}

// check header for date and verify if update needed (0 if no update, 1 if update, greater than 100 for errors)
int checkHeaderDate(char* filePath, char* current_date, int cutOffRange)
{
    // note: magic numbers abound in this function, use defines for error codes in header
    FILE *tickerFile = fopen(filePath, "r");
    if (tickerFile == NULL)
    {
        fclose(tickerFile);
        return 101;
    }
    
    char headerLine[256];
    char* null_check = fgets(headerLine, sizeof(headerLine), tickerFile);
    if (null_check == NULL)
        return 103;
    char *last_date;
    char *first_value = strtok(headerLine, ",\n");
    if (first_value == NULL)
        return 104;
    last_date = strtok(NULL, ",\n");
    if (last_date == NULL)
        return 104;
    int update_status = isOutOfCutOffRange(current_date, last_date, cutOffRange);
    fclose(tickerFile);
    return update_status;
}

// new_date > old_date
// if new_date is out of cut off range of old date, then returns 1 (true)
int isOutOfCutOffRange(char* new_date, char* old_date, int cutOffRange)
{
    int n_year, n_month, n_day, o_year, o_month, o_day;
    // sscanf scans strings (first arguemnet) into variable (third argumet and onwards), according to format provided (second argument)
    sscanf(new_date, "%d/%d/%d", &n_year, &n_month, &n_day);
    sscanf(old_date, "%d/%d/%d", &o_year, &o_month, &o_day);

    // greater than 2 guarentees age, not month/year roll over
    if ((n_year - o_year) > 2)
        return 1;
    if ((n_month - o_month) > 2)
        return 1;
    
    int daysPast = 0;
    int assumedDaysInMonth = 30;
    int monthsInYears = 12;

    // count down days past from old to new date to find range
    while (daysPast <= cutOffRange)
    {
        if (n_month == o_month && n_day == o_day && n_year == o_year)
            return 0;

        n_day--;

        if (n_day == 0)
        {
            n_day = assumedDaysInMonth;
            n_month--;
            if (n_month == 0)
            {
                n_month = monthsInYears;
                n_year--;
            }
        }

        daysPast++;     
    }
    
    return 1;
}

