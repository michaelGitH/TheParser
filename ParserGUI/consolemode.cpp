
#include "consolemode.h"

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

void test(){
    
    vector<string> text =
    {
        "-u",
        "https://www.eldorado.ru/cat/1461428/page/",
        "-i",
        "1-5",
        "-h",
        "Имя",
        "-r",
        "<a itemprop=\"url\"\n.+>.+</a>",
        "-rc",
        "<a itemprop=\"url\"\n.+/\">|</a>",
        "-r",
        "discountPrice itemPrice\">.+<span class=\"rub\">",
        "-rc",
        "discountPrice itemPrice\">|<span class=\"rub\">",
        "-chf",
        "UTF-8",
        "-cht",
        "CP866",
        "-chfh",
        "UTF-8",
        "-chth",
        "CP866",
        "-ShowNumPage"
    };
    
    const int arc = text.size() +1;
    
    char * arv[arc];
    
    for(int i = 1; i < arc; ++i){
        auto & str = text[i-1];
        char * newp = new char[str.size()+1];
        strcpy(newp, str.data());
        arv[i] = newp;
    }
    
    ConsoleMode(arc, arv);
    
    
    int tmp = 0;
}


int GetResult(const string & url, vector<vector<string>> & res, const vector<string> & headers, 
              const vector<string> & patterns, const vector<string> & tags, char * chf, char * cht, char * chfh, char * chth, bool unescape){
	Parser pa;
	pa.headers = headers;
	pa.patterns = patterns;
	pa.excess_tags = tags;
    pa.unescape = unescape;
	
	int records_count = 0;
	
    if(chf != nullptr && cht != nullptr)
        pa.set_charset(chf, cht);
		
    if(chfh != nullptr && chth != nullptr)
        pa.set_header_charset(chfh, chth);

	if(headers.size() != 0){
		pa.headersOn = true;
		++records_count;
	}
	
    pa.url = url;
    res = pa.parse_content();
    string error;
    int errc = pa.get_error(&error);
    if(errc){
        cerr << error;
    }
    return errc;
}

string get_content(string url, bool headers){
    Parser pa;
	pa.url = url;
    pa.headersOn = headers;
    string content;
    pa.get_content(content);
    return content;
}

void GetContentCM(int argc, char ** argv){
    
    if(argc > 4){
        cerr << "too many options!";
        exit(1);
    }
	else if(argc < 3){
		cerr << "url was not set!";
		exit(1);
	}
    
	Parser pa;
	pa.url = argv[2];
    
    if(argc == 4 && string(argv[3]) == "-sh" ){
        pa.headersOn = true;
    }
        
	string content;
    int code = pa.get_content(content);
	std::cout << content << endl;
	if(code) exit(1);
	else exit(0);
}

void TestCharset(int argc, char ** argv){
	if(argc > 5){
        cerr << "too many options!";
        exit(1);	
	}
	else if(argc < 5){
		cerr << "too few options!";
		exit(1);
	}
	
	string phrase = argv[2];
	
	int s = phrase.size();
	
	string enc_phrase = conv_charset(argv[3], argv[4], phrase, &s);
	
	if(enc_phrase == ""){
		cerr << "Bad charset convert!";
		exit(1);
	}
	
	std::cout << "Befor ecnode: " << phrase
		<< "\nAfter encode: " << enc_phrase << endl;
		
	exit(0);
		
}

void ConsoleMode(int argc, char ** argv){

	if(string(argv[1]) == "-gc"){
		GetContentCM(argc, argv);
	}
	else if(string(argv[1]) == "-charset"){
		TestCharset(argc, argv);
	}

    vector<string> urls;
    vector<string> patterns;
    vector<string> headers;
    vector<string> tags;
    vector<vector<string>> result, tresult;
    long int itbeg = 0, itend = 0;
    string itcend = "";
    char * chf = nullptr, * cht = nullptr;
	char * chfh = nullptr, * chth = nullptr;
	bool ShowNumPage = false;
    bool Unescape = false;

    for(int i = 1; i < argc; ++i){
        string str = argv[i];
        if(str == "-u"){
			if(i + 1 >= argc){
				cerr << "Url is not set!\n";
				exit(1);
			}
            urls.push_back(argv[++i]);
        }
        else if(str == "-i"){
			if(i + 1 >= argc){
				cerr << "Iterator range is not set!\n";
				exit(1);
			}
            setIterator(itbeg, itend, itcend, argv[++i]);
        }
        else if(str == "-h"){
			if(i + 1 >= argc){
				cerr << "Header is not set!\n";
				exit(1);
			}
			headers.push_back(argv[++i]);
		}
        else if(str == "-r"){
			if(i + 1 >= argc){
				cerr << "Regular expression is not set!\n";
				exit(1);
			}
			patterns.push_back(argv[++i]);
		}
        else if(str == "-rc"){
			if(i + 1 >= argc){
				cerr << "Excess regular expression is not set!\n";
				exit(1);
			}
			tags.push_back(argv[++i]);
		}
        else if(str == "-chf"){
			if(i + 1 >= argc){
				cerr << "Charset 'From' is not set!\n";
				exit(1);
			}
			chf = argv[++i];
		}
        else if(str == "-cht"){
			if(i + 1 >= argc){
				cerr << "Charset 'To' is not set!\n";
				exit(1);
			}
			cht = argv[++i];
		}
        else if(str == "-chfh"){
			if(i + 1 >= argc){
				cerr << "Charset 'From' is not set!\n";
				exit(1);
			}
			chfh = argv[++i];
		}
        else if(str == "-chth"){
			if(i + 1 >= argc){
				cerr << "Charset 'To' is not set!\n";
				exit(1);
			}
			chth = argv[++i];
		}
		else if(str == "-ShowNumPage"){
			ShowNumPage = true;
		}
        else if(str == "-une"){
            Unescape = true;
        }
        else{
            cerr << string("Invalid option (") + str + ")\n";
            exit(1);            
        }
    }
    
    int error;
    bool increm;
	for(auto & url : urls){
        if(itbeg == 0 && itbeg == itend){
            error = GetResult(url, tresult, headers, patterns, tags, chf, cht, chfh, chth, Unescape);
            for(auto & v: tresult){
                for(auto & s: v){
                    std::cout << s << ";";
                }
                std::cout << endl;
            }     
            result.insert(result.end(), tresult.begin(), tresult.end());
        }
        else{
            increm = itcend == "eof" ? true : false;
            for(long i = itbeg; i <= itend || increm; ++i){
                if(i > itbeg)
                    headers.clear();
                string purl = url + to_string(i);
                
                error = GetResult(purl, tresult, headers, patterns, tags, chf, cht, chfh, chth, Unescape);	
					
                auto itres = tresult.back();
                if(itres.size() == 0){
                    tresult.resize(tresult.size()-1);
                }
                
                auto beg = tresult.begin();
                if(beg->size() == 0){
                    tresult.erase(beg);
                }
                
                if(tresult.size() == 0)
					break;
                
                if(ShowNumPage)
					std::cout << "\n\n" << i << "\n\n";
                
                for(auto & v: tresult){
                    for(auto & s: v){
                        std::cout << s << ";";
                    }
                    std::cout << endl;
                }      
                result.insert(result.end(), tresult.begin(), tresult.end());               
            }
        }
        headers.clear();
	}
	

	
}

void setIterator(long int & b, long int & e, string & itcend, char * range){

	smatch sm;
	
	regex reg("[0-9]+-([0-9]+|eof)");
	
	string srange = range;
	
	if(!regex_match(srange, sm, reg)){
		cerr << "Incorrect format iterator range!\n";
		exit(1);
	}
	
	reg = regex("([0-9]+|eof)");
	
	if(regex_search(srange, sm, reg)){
		for(auto m : sm)
			b = stol(string(m));
		srange = sm.suffix().str();
	}
	else{
		cerr << "Error converting range numbers!";
		exit(1);
	}
	
	if(regex_search(srange, sm, reg)){
        for(auto m : sm){
            string match = m;
            if(match == "eof"){
                e = 0;
                itcend = "eof";
            }
            else{
                e = stol(string(m));
                itcend = "";
            }
        }
		srange = sm.suffix().str();
	}
	else{
		cerr << "Error converting range numbers!";
		exit(1);
	}
}

int fun(){

    Parser pa;

    string url;

//    vector<string> pats;
    pa.patterns.push_back("data-sel=\".+\">\n.+\n\\W</a>");
    pa.patterns.push_back("<span class=\"c-pdp-price__discount\">\\n.+");
    pa.patterns.push_back("<div  class=\"c-pdp-price__current\" .+>.+");
    pa.excess_tags.push_back("data-sel=\".+\">\n|\n\\W</a>");
    pa.excess_tags.push_back("<span class=\"c-pdp-price__discount\">\\n");
    pa.excess_tags.push_back("<div  class=\"c-pdp-price__current\" .+>");

    for(int y = 1; y <= 134; ++y){
        pa.url = string("https://www.mvideo.ru/promo/superfinal-skidok-mark109832800/f/page=") + to_string(y);

        auto res = pa.parse_content();
        string error;
        if(pa.get_error(&error)){
            cerr << error;
        }
        for(auto & v: res){
            for(auto & s: v){
                std::cout << s << ";";
            }
            std::cout << endl;
        }
    }

    return 0;
}

//int main(int argc, char ** argv)
//{
    
//    if(argc > 1)    
//		ConsoleMode(argc, argv);
//    else
//        test();
	
//    return 0;
//}
