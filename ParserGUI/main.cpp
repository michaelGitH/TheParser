//ParserGUI v 0.3

#include "parsergui.h"
#include "consolemode.h"
#include <QApplication>

int main(int argc, char *argv[])
{  
    if(argc == 1){
        QApplication a(argc, argv);
        ParserGUI w;
        w.show();
        
        return a.exec();
    }
    else{
        ConsoleMode(argc, argv);
    }
}
