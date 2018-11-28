#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "curl/curl.h"
#include <iostream>
#include <regex>
#include <iconv.h>
#include <cstring>
#include <thread>
#include <unicode\unistr.h>

using namespace std;

string conv_charset(const char *from, const char *to, string str, int *size);

void UnicodUnescape(string & str);

class Parser
{

    char CurlErrorBuffer[CURL_ERROR_SIZE];
    
    char * chf;
    char * cht;
    char * chfh;
    char * chth;

    string errorBuffer;

    string buffer;

    CURL * curl;

    int ERROR_CODE;

    CURL * curl_start(bool header = false);

    int string_handle(const string str, vector<vector<string>>::iterator & it, const vector<string> & patterns, vector<string> excess_tags = vector<string>());

    void clean_string(string & str, const string & pat);
    
    void clear_delims(string & str);
    
    void add_string(vector<vector<string>>::iterator it, string str);
	
	void set_headers(vector<vector<string>> & content_table);

    static int writer(char *data, size_t size, size_t nmemb, string *buffer);

public:
    Parser();
    ~Parser();

	bool headersOn;
    bool unescape;
    string url;
	vector<string> headers;
    vector<string> patterns;
    vector<string> excess_tags;
    string delims = "";
    
    void set_charset(const char * from, const char * to);
	
	void set_header_charset(const char * from, const char * to);

    int get_content(string & content);

    int get_error(string * buffer = nullptr);

    vector<vector<string>> parse_content();
};



#endif // PARSER_H
