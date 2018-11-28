#ifndef CONSOLEMODE_H
#define CONSOLEMODE_H

#include "parser.h"

//Параметры:
//1 -u(url) Адрес
//2 -i(iterator) итератор (диапазон n-m). Будет добавлен в конец адреса
//3 -h(header) ИмяСтолбца -r(regexp) РегулярноеВыражение -rc(regexp_clear) РегулярноеВыражениеУдалениеЛишнего
//  Соответствия заголовков РГ и РГУЛ устанавливаются в прямой последовательности
//  Если не будет задан -h столбец будет создан без заголовка
//  Если будет задан -h без -r будет создан пустой столбец с заголовком -h
//  Количество столбцов неограничено
//4 -chf кодировка из -cht кодировка в
//5 -chfh -chth - аналог предыдущего для заголовков
//6 -ShowNumPage - Выводить номер страницы
//7 -une - Преобразовать из unicode unescape

void setUrl(string & cururl, const string & newurl);

void setIterator(long int & b, long int & e, string & itcend, char * range);

void ConsoleMode(int argc, char ** argv);

int GetResult(const string & url, vector<vector<string>> & res, const vector<string> & headers, 
              const vector<string> & patterns, const vector<string> & tags, char * chf, char * cht, char * chfh, char * chth, bool unescape);

string get_content(string url, bool headers = false);

void GetContentCM(int argc, char ** argv);

void TestCharset(int argc, char ** argv);

void test();


#endif // CONSOLEMODE_H
