#include "parser.h"

Parser::Parser()
{
    chf = cht = chfh = chth = nullptr;
	headersOn = false;
    unescape = false;
    errorBuffer = "";
    CurlErrorBuffer[0] = '\0';
    curl = curl_start();
}

Parser::~Parser(){
    curl_easy_cleanup(curl);
}

CURL * Parser::curl_start(bool header){
    CURL * curl;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, CurlErrorBuffer);
        
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 
        
        if(header)
            curl_easy_setopt(curl, CURLOPT_HEADER, 1);
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Parser::writer);
        
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

     }

    return curl;
}

int Parser::get_content(string & content){

    buffer.clear();
    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    
    if(headersOn){
        curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    }
    else{
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    }

    ERROR_CODE = curl_easy_perform(curl);

    if (ERROR_CODE == CURLE_OK)
        content = buffer;
    else{
        errorBuffer += string("\n") + CurlErrorBuffer;
        content = CurlErrorBuffer;
    }

    return ERROR_CODE;
}

int Parser::get_error(string * buffer){
    if(ERROR_CODE){
        if(buffer != nullptr)
            *buffer = string(errorBuffer);
        return ERROR_CODE;
    }
    return CURLE_OK;
}

vector<vector<string>> Parser::parse_content(){

    vector<vector<string>> result;

    string content;
    if(patterns.size() == 0){
        ERROR_CODE = 100;
        errorBuffer += string("\n") + "Regular expression was not specified!";
        return result;
    }


    if(get_content(content) != CURLE_OK){
        return result;
    }
    
    if(unescape){
        UnicodUnescape(content);
    }

    smatch sm;

    string common_str = "";

    for(auto & p : patterns){
        common_str += "|" + p;
    }
    common_str.erase(0, 1);

    regex common_pattern(common_str);

    int col_count = patterns.size(), coln, prev_coln;
	
	if(headersOn){
		set_headers(result);
	}
	
    int cur_col = col_count;
    
    vector<string> e_str;
    vector<vector<string>>::iterator newstr;
    result.push_back(e_str);
    
    while(regex_search(content, sm, common_pattern)){
        if(cur_col >= col_count){
            newstr = --(result.end());
            if(newstr->size() == col_count){
                result.push_back(e_str);
            }
            result.push_back(e_str);
            newstr = --(--(result.end()));
            cur_col = prev_coln = 0;
        }
        for(auto s : sm){
            if(!s.matched){
                break;
            }
            coln = string_handle(s, newstr, patterns, excess_tags);
            if(coln != cur_col){
                cur_col = coln;
            }
            ++cur_col;
            prev_coln = coln;
        }
        content = sm.suffix().str();
    }

    return result;
}

int Parser::string_handle(const string str_source, vector<vector<string>>::iterator & it, const vector<string> & patterns, vector<string> excess_tags){ 
    
    int rv = 0;
    smatch sm;
    unsigned tagsize = excess_tags.size(); // количество исключающих шаблонов
    unsigned itcount = it->size(); // Количество заполненных столбцов в тек строке
    unsigned colcount = patterns.size(); // Всего столбцов
    string str = str_source; // Строка, для которой НУЖНО ОПРЕДЕЛИТЬ СТОЛБЕЦ
    for(unsigned i = 0; i < colcount; ++i){ // Для каждого столбца
        rv = i;
        regex reg(patterns[i]);
        if(regex_match(str, sm, reg)){ // Проверка на соответствие шаблону текущего столбца
            
            // СООТВЕТСТВИЕ УСТАНОВЛЕНО
            
            if(i > itcount){ // Если индекс текущего столбца больше количества заполненных столбцов
                for(unsigned j = itcount; j < i; ++j){
                    it->push_back(""); // Для предыдущих столбцов нет данных
                    // Поэтому заполняем их пустыми строками
                }
            }
            else if(itcount > i){ // Количество столбцов превышает индекс текущего
                for(unsigned j = itcount; j < colcount; ++j){
                    it->push_back(""); // Значит для оставшихся столбцов нет данных
                    // Поэтому заполняем их пустыми строками
                }
                ++it; // И переходим к следующей строке
                for(unsigned j = 0; j < i; ++j){ // Если индекс текущей колонки не нулевой
                    it->push_back(""); // Значит в новой строке, для меньших индексов колонок, нет данных
                    // Поэтому заполняем их пустыми строками
                }
                rv = colcount;
            }
            if(tagsize != 0 && tagsize - 1 >= i){
                clean_string(str, excess_tags[i]);
            }
            
            add_string(it, str);

            return rv;
        }
    }

    return -1;
}

void Parser::add_string(vector<vector<string>>::iterator it, string str){
    
    if(chf != nullptr && cht != nullptr){
        int size = str.size();
//        int base_size = size;
        string winstr = conv_charset(chf, cht, str, &size);
        
        if(winstr != ""){
            it->push_back(winstr);
        }
        else{
            str = "Bad charset convert(used base charset): " + str;
            it->push_back(str); 
        }   
    }
    else{
        clear_delims(str);
        it->push_back(str);
    }
}

void Parser::clean_string(string & str, const string & pat){
    regex reg(pat);
    str = regex_replace(str, reg, "");
}

void Parser::clear_delims(string & str)
{
    if(delims == "") return;
    regex reg(delims);
    str = regex_replace(str, reg, " ");
}

int Parser::writer(char *data, size_t size, size_t nmemb, string *buffer)
{
  //переменная - результат, по умолчанию нулевая
  int result = 0;
  //проверяем буфер
  if (buffer != NULL)
  {
    //добавляем к буферу строки из data, в количестве nmemb
    buffer->append(data, size * nmemb);
    //вычисляем объем принятых данных
    result = size * nmemb;
  }
  //вовзращаем результат
  return result;
}

void Parser::set_charset(const char * from, const char * to){
    int f = strlen(from), t = strlen(to);
    chf = new char[f+1], cht = new char[t+1];
    strcpy(chf, from);
    strcpy(cht, to);
}

void Parser::set_header_charset(const char * from, const char * to){
    int f = strlen(from), t = strlen(to);
    chfh = new char[f+1], chth = new char[t+1];
    strcpy(chfh, from);
    strcpy(chth, to);
}

void Parser::set_headers(vector<vector<string>> & content_table){
	if(chfh == nullptr || chth == nullptr){
		content_table.push_back(headers);
		return;
	}
	for(auto & h: headers){
        int size = h.size();

        string winstr = conv_charset(chfh, chth, h, &size);
        
        if(winstr != ""){
            h = winstr;
        }
        else{
            h = "Bad charset convert(used base charset): " + h;
        }
	}
	content_table.push_back(headers);
}

string conv_charset(const char *from, const char *to, string str, int *size)
{
    string out;
    
    if (*size <= 0) {
        out = "";
    } else {
        char *buf, *holder, *cstr;
        size_t insize, outsize, bufsize;
        int r;
        iconv_t cd; 

        cd = iconv_open(to, from);
        if (cd == (iconv_t) - 1)
            return "";

        insize = *size;
        outsize = bufsize = insize * 4;

        buf = new char[bufsize+1];
        buf[bufsize] = '\0';
        holder = buf;
        
        cstr = new char[(*size)+1];
        char * cstr_ptr = cstr;
        strcpy(cstr, str.data());

        r = iconv(cd, &cstr, &insize, &buf, &outsize);
        if (r < 0 || insize != 0) {
            delete[] holder;
            delete[] cstr_ptr;
            iconv_close(cd);
            return "";
        }


        int newsize = bufsize - outsize;

        buf -= newsize;

        out = buf;
        out.resize(newsize);
        
        delete[] holder;
        delete[] cstr_ptr;
        iconv_close(cd);
    }

    return out;
}

void UnicodUnescape(string &str)
{
    icu::UnicodeString text(str.data());
    icu::UnicodeString untext = text.unescape();
    str = "";
    untext.toUTF8String(str);
}
