#include "parsergui.h"
#include "ui_parsergui.h"

ParserGUI::ParserGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ParserGUI)
{
    ui->setupUi(this);
    match_decor = "<span style=\" font-size:8pt; color:#ff5500;\">";
    setText_edit_decor();
    SetCharsetMatches();
    SetAvailabilityColFields(false);
    SetSelectionColor();
    on_FindAction_triggered();
}

ParserGUI::~ParserGUI()
{
    delete ui;
}

//************************METHODS

// Загружает содержимое сайта url_edit в TextEdit
void ParserGUI::GetContent(){
    string url = ui->url_edit->text().toStdString();
    Parser pa;
    pa.url = url;
    string content;
    pa.get_content(content);
    if(ui->UnescapedCheckBox->isChecked()){
        UnicodUnescape(content);
    }
    QString codec_text = ui->ContentCharsetComboFrom->currentText();
    if(ui->ContentCharsetCheckBox->isChecked() && codec_text != ""){
        QTextCodec *codec = QTextCodec::codecForName(codec_text.toStdString().data());
        url_content = codec->toUnicode(content.data());        
    }
    else{
        url_content = QString::fromStdString(content);
    }
    //QString pt = ui->textEdit->toPlainText();
    ui->textEdit->clear();
    ui->textEdit->setPlainText(url_content); //url_content
}

//Сохранить заголовок столбца
void ParserGUI::SaveColumnHeader()
{
    int index = ui->comboBox->currentIndex();
    if(index == -1) return;
    QString cur_text =ui->comboBox->lineEdit()->text();
    headers[index] = cur_text.toStdString();
    ui->comboBox->setItemText(index, cur_text);   
}

//Сохранить значение регулярного выражения
void ParserGUI::SaveRegExp()
{
    int index = ui->comboBox->currentIndex();
    if(index == -1) return;
    
    regexps[index] = ui->regexp_edit->text().toStdString();    
}

//Сохранить значение исключающего выражения
void ParserGUI::SaveExcessRE()
{
    int index = ui->comboBox->currentIndex();
    if(index == -1) return;
    
    excess[index] = ui->excess_edit->text().toStdString();   
}

//Добавить столбец
void ParserGUI::AddColumn()
{
    if(!IsColFiealdsAvailable){
        SetAvailabilityColFields(true);
    }
    
    ui->comboBox->addItem("");
    
    headers.push_back("");
    regexps.push_back("");
    excess.push_back("");
    
    ui->comboBox->setCurrentIndex(ui->comboBox->count()-1);   
}

//Удалить столбец
void ParserGUI::DelColumn()
{
    int index = ui->comboBox->currentIndex();
    if(index == -1) return;
    
    headers.erase(headers.begin()+index);
    regexps.erase(regexps.begin()+index);
    excess.erase(excess.begin()+index);
    
    ui->comboBox->removeItem(index);
    
    if(ui->comboBox->count() == 0){
        SetAvailabilityColFields(false);
    }
}

//Включить\Отключить доступность полей столбца
void ParserGUI::SetAvailabilityColFields(bool on)
{
    ui->comboBox->setEditable(on);
    ui->regexp_edit->setEnabled(on);
    ui->excess_edit->setEnabled(on);
    IsColFiealdsAvailable = on;
    if(on){
        connect(ui->comboBox->lineEdit(), SIGNAL(editingFinished()), this, SLOT(on_comboBox_editingFinished()));
    }
}

//Выбор столбца
void ParserGUI::ColumnSelection(int index)
{
    if(index >= 0 && (static_cast<long>(regexps.size()) - index > 0)){
        QString rText = QString::fromStdString(regexps[index]);
        QString eText = QString::fromStdString(excess[index]);
        QString hText = QString::fromStdString(headers[index]);
        ui->regexp_edit->setText(rText);
        ui->excess_edit->setText(eText);
    }   
    else{
        ui->regexp_edit->setText("");
        ui->excess_edit->setText("");      
    }
}

//Найти соответствия
void ParserGUI::FindMatches()
{
    QString retext = ui->regexp_edit->text();
    if(retext == "")
        return;
    
    QTextEdit * te = ui->textEdit;
    QString text = te->toPlainText();
    url_content = text;
    
    int pos = 0;
    QString str, html_str;
    QString newText = "";
    QRegularExpression re(retext);
    QRegularExpressionMatch match;
    matches.clear();
    
    int cur_match, limit = -1;
    if(ui->LimitCheckBox->isChecked()){
        limit = ui->LimitEdit->text().toInt();
    }
    
    for(cur_match = 0; cur_match != limit; ++cur_match){
        pos = text.indexOf(re, 0, &match);
        if(pos == -1){
            break;
        }
        str = match.captured();
        str.replace(QRegularExpression("[ ]{2,}"), " ");
        str.replace(QRegularExpression("[\n]"), "");
        html_str = str.toHtmlEscaped();
        newText += text.left(pos).toHtmlEscaped() + match_decor + html_str + "</span>";
        text = text.mid(pos + str.size());
        matches.push_back(QStringList{str});
    }
    newText += text.toHtmlEscaped();
    
    newText = newText.replace("\n", "<br>");
    
//    url_content = newText;
    
    te->document()->clear();
    match_index = 0;
    
    te->document()->setHtml(newText);
    on_begin_te_pushButton_clicked();
}

void ParserGUI::ClearMatches()
{
    QString retext = ui->regexp_edit->text();
    QString extext = ui->excess_edit->text();

    if(retext == "" || extext == "")
        return;

    QTextEdit * te = ui->textEdit;
    QString text = te->toPlainText();
    url_content = text;
    
    matches.clear();
    
    int pos = 0;
    QString str, html_str;
    QString newText = "";
    QRegularExpression re(retext);
    QRegularExpressionMatch match;
    
    int cur_match, limit = -1;
    if(ui->LimitCheckBox->isChecked()){
        limit = ui->LimitEdit->text().toInt();
    }
    
    for(cur_match = 0; cur_match != limit; ++cur_match){
        pos = text.indexOf(re, 0, &match);
        if(pos == -1){
            break;
        }
        str = match.captured();
        
        auto Strings = SplitString(str, extext);
        for(auto & s : Strings){
            s.replace(QRegularExpression("[ ]{2,}"), " ");
            s.replace(QRegularExpression("[\n]"), "");
        }
        
        html_str = Strings[0].toHtmlEscaped() + match_decor+
                Strings[1].toHtmlEscaped() + "</span>" + Strings[2].toHtmlEscaped();
        
        newText += text.left(pos).toHtmlEscaped() + html_str;
        text = text.mid(pos+str.size());
        
        str.replace(QRegularExpression("[ ]{2,}"), " ");
        str.replace(QRegularExpression("[\n]"), "");
        
        matches.push_back(QStringList{str, Strings[1]});
    }
    newText += text.toHtmlEscaped();
    
    newText = newText.replace("\n", "<br>");
    
//    url_content = newText;
    
    ui->textEdit->document()->clear();
    te->document()->setHtml(newText);
    
    match_index = 0;
    on_begin_te_pushButton_clicked();
}

void ParserGUI::setText_edit_decor()
{
    text_edit_decor = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">")
            +"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
            +"p, li { white-space: pre-wrap; }"
            +"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8pt; font-weight:400; font-style:normal;\">"
            +"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8.25pt;\"><br /></p></body></html>";
}

bool ParserGUI::CheckToExecute()
{
    if(regexps.size() == 0) 
        return false;
    
    if(ui->url_edit->text() == "")
        return false;
    
    int filled_count = 0;
    
    for(auto & r : regexps){
        if(r != "") ++filled_count;
    }
    
    if(filled_count == 0) return false;
    
    return true;
}

//Выполнить
void ParserGUI::Execute(){
    
    if(!CheckToExecute())
        return;
    
    match_result.clear();
    
    vector<vector<string>> tresult;
    
    Parser pa;
	pa.headers = headers;
	pa.patterns = regexps;
	pa.excess_tags = excess;
    pa.unescape = ui->UnescapedCheckBox->isChecked();
    
    //pa.delims = "&nbsp;"; // ЗАКЛАДКА
	
	int records_count = 0;
    
    const char * chf = "";
    auto chfs = charsets.find(ui->CharsetComboFrom->currentText());
    if(chfs != charsets.end()){
        chf = chfs->second.toStdString().data();
    }
    
    const char * cht = "";
    auto chts = charsets.find(ui->CharsetComboTo->currentText());
    if(chts != charsets.end()){
        cht = chts->second.toStdString().data();
    }
    
    const char * chfh = "";
    auto chfhs = charsets.find(ui->HeaderCharsetComboFrom->currentText());
    if(chfhs != charsets.end()){
        chfh = chfhs->second.toStdString().data();
    }
    
    const char * chth = "";
    auto chths = charsets.find(ui->HeaderCharsetComboTo->currentText());
    if(chths != charsets.end()){
        chth = chths->second.toStdString().data();
    }
	
    if(ui->CharsetCheckBox->isChecked() && strlen(chf) && strlen(cht))
        pa.set_charset(chf, cht);
		
    if(ui->HeaderCharsetCheckBox->isChecked() && strlen(chfh) && strlen(chth))
        pa.set_header_charset(chfh, chth);

	if(headers.size() != 0){
		pa.headersOn = true;
		++records_count;
	}
	
    string url = ui->url_edit->text().toStdString();
    pa.url = url;
    
    QString low_text = ui->LowerEdit->text();
    QString up_text = ui->UpperEdit->text();
    
    QCheckBox * cb = ui->SeveralcheckBox;
    
    if(ui->SeveralcheckBox->isChecked() && low_text != ""){
        int lower = low_text.toInt();
        int upper = up_text.toInt();
        string cur_num;
        bool max_num = up_text == "" ? true : false;
        
        bool hon = headers.size() != 0;
        
        for(int i = lower; i <= upper || max_num; ++i){
            if(i != lower && hon && pa.headers.size() != 0){
                pa.headers.clear();
            }
            cur_num = to_string(i);
            pa.url = url + cur_num;
            tresult = pa.parse_content();
            
            auto itres = tresult.back();
            if(itres.size() == 0 && tresult.size() != 0){
                tresult.resize(tresult.size()-1);
            }
            
            auto beg = tresult.begin();
            if(beg->size() == 0){
                tresult.erase(beg);
            }
            
            if(tresult.size() == 0)
                break;
            
            match_result.insert(match_result.end(), tresult.begin(), tresult.end()); 
        }
    }
    else{
        match_result = pa.parse_content();
        
        auto itres = match_result.back();
        if(itres.size() == 0 && match_result.size() != 0){
            match_result.resize(match_result.size()-1);
        }
        
        auto beg = match_result.begin();
        if(beg->size() == 0){
            match_result.erase(beg);
        }
    }
    
    if(match_result.size() != 0){
        ui->save_action->setEnabled(true);
    }
    else{
        ui->save_action->setEnabled(false);
    }
    
    string error;
    int errc = pa.get_error(&error);
    
    FillTableWidget();
    
}

//Сохранить как csv
void ParserGUI::WriteCSV(QTextStream & stream)
{
    int i;
    for(auto & vs : match_result){
        for(i = 0; i < vs.size()-1; ++i){
            stream << QString::fromStdString(vs[i]) << ";";
        }
        stream << QString::fromStdString(vs[i]) << "\n";
    }    
}

//Записать bat/bash файл для консольной версии
void ParserGUI::WriteScript(QTextStream &stream, QString type)
{
    QString script;
    
    QString url = ui->url_edit->text();
    
    if(url != "")
        script = "ParserGUI -u \"" + url + "\"";
    else
        return;
    
    QString lower_page = ui->LowerEdit->text();
    QString upper_page = ui->UpperEdit->text();
    if(ui->SeveralcheckBox->isChecked() && lower_page != ""){
        if(upper_page != "")
            script = QString("%1 -i %2-%3").arg(script, lower_page, upper_page);
        else
            script = QString("%1 -i %2-eof").arg(script, lower_page);
    }
    
    AddConsoleArgs(script, headers, "-h", type);
    AddConsoleArgs(script, regexps, "-r", type);
    AddConsoleArgs(script, excess, "-rc", type);
    
    QString chf = ui->CharsetComboFrom->currentText(),
            cht = ui->CharsetComboTo->currentText(),
            chfh = ui->HeaderCharsetComboFrom->currentText(),
            chth = ui->HeaderCharsetComboTo->currentText();
    
    auto chfs = charsets.find(chf),
         chts = charsets.find(cht),
         chfhs = charsets.find(chfh),
         chths = charsets.find(chth);
    
    chf = chfs != charsets.end()? chfs->second : "";
    cht = chts != charsets.end()? chts->second : "";
    chfh = chfhs != charsets.end()? chfhs->second : "";
    chth = chths != charsets.end()? chths->second : "";
    
    if(ui->CharsetCheckBox->isChecked() && chf != "" && cht != "")
        script = QString("%1 -chf %2 -cht %3").arg(script, chf, cht);
    
    if(ui->HeaderCharsetCheckBox->isChecked() && chfh != "" && chth != "")
        script = QString("%1 -chfh %2 -chth %3").arg(script, chfh, chth);    
    
    if(ui->UnescapedCheckBox->isChecked()){
        script += " -une";
    }
    
    stream << script;
}

void ParserGUI::ReadScript(QTextStream &stream, QString type)
{
    QString script = stream.readAll();
    
    QStringList args;
    QString arg;

    
    while(script != ""){
        arg = GetNextArg(script, type);
        if(arg != "")
            args.push_back(arg);
    }
    
    FillArgs(args);
}

void ParserGUI::ClearFields()
{
    SetAvailabilityColFields(false);
    ui->url_edit->clear();
    ui->textEdit->clear();
    url_content = "";
    ui->regexp_edit->clear();
    ui->excess_edit->clear();
    ui->comboBox->clear();
    ui->SeveralcheckBox->setChecked(false);
    ui->LowerEdit->clear();
    ui->UpperEdit->clear();
    ui->CharsetCheckBox->setChecked(false);
    ui->CharsetComboFrom->setCurrentIndex(0);
    ui->CharsetComboTo->setCurrentIndex(0);
    ui->HeaderCharsetCheckBox->setChecked(false);
    ui->HeaderCharsetComboFrom->setCurrentIndex(0);
    ui->HeaderCharsetComboTo->setCurrentIndex(0);
    headers.clear();
    regexps.clear();
    excess.clear();
}

//В редакторе должны присутствовать только цифры
void ParserGUI::OnlyDigitInLineEdit(QLineEdit *edit, const QString &arg)
{
    int size = arg.size();
    
    if(size == 0) return;
    
    QString old_text = arg.left(size-1);
    
    QChar new_text = arg[size-1];
    
    if(!new_text.isDigit()){
        edit->setText(old_text);
    }    
}

//Установить цвет выделения в textedit
void ParserGUI::SetSelectionColor()
{
    QPalette pal = ui->textEdit->palette();
    
    pal.setColor(QPalette::Highlight, QColor(70, 130, 180));
    
    pal.setColor(QPalette::HighlightedText, QColor(255, 255, 224));
    
    ui->textEdit->setPalette(pal);
}

//Заполнить TableWidget
void ParserGUI::FillTableWidget()
{
    QTableWidget * table = ui->tableWidget;
    table->clear();
    table->setColumnCount(headers.size());
    table->setRowCount(match_result.size() - 1);
    QStringList hlist;
    for(auto & h : headers){
        hlist.push_back(QString::fromStdString(h));
    }
    table->setHorizontalHeaderLabels(hlist);
    
    vector<string> current_string;
    for(int i = 1; i < match_result.size(); ++i){
        current_string = match_result[i];
        for(int j = 0; j < current_string.size(); ++j){
            table->setItem(i-1, j,new QTableWidgetItem(QString::fromStdString(current_string[j])));
        }
    }
}

//Выполнить прасинг в редакторе
void ParserGUI::DoEditorParse()
{
    if(ui->url_edit->text() != ""){
        GetContent();
    }
    else{
        return;
    }
    
    if(ui->regexp_edit->text() != ""){
        if(ui->excess_edit->text() != ""){
            ClearMatches();
        }
        else{
            FindMatches();
        }
    }
    else{
        matches.clear();
        return;
    }    
}

QString ParserGUI::GetNextArg(QString &script, QString type)
{
    bool is_arg = false;
    
    bool in_quotes = false;
    
    bool win_spec = false;
    
    QString arg(""), cur_s(""), next_s("");
    
    long script_size = script.size();
    
    int i;
    
    for(i = 0; i < script_size; ++i){
        cur_s = script[i];
        next_s = script_size - i == 1? "": QString(script[i+1]);
        if(!is_arg){
            if(cur_s == "\""){
                is_arg = in_quotes = true;
            }
            else if(cur_s != " "){
                arg += cur_s;
                is_arg = true;
                in_quotes = false;
            }
            continue;
        }
        if(in_quotes && cur_s == "\\" && next_s == "\""){
            arg += script[++i];
            win_spec = !win_spec;
            continue;
        }
        if(in_quotes && cur_s == "\""){
            ++i;
            break;
        }
        if(type == ".bat" && win_spec && cur_s == "^" && next_s != "^"){
            continue;
        }
        if(!in_quotes && cur_s == " "){
            ++i;
            break;
        }
        arg += cur_s;
    }
    
    if(script_size - i == 1)
        script = "";
    else
        script = script.mid(i);
    
    return arg;
}

void ParserGUI::FillArgs(QStringList & args)
{
    QString cur_arg(""), next_arg("");
    
    long args_size = args.size();
    
    long b, e;
    
    string eof;
    
    ClearFields();
    
    for(int i = 0; i < args_size; ++i){
        
        cur_arg = args[i];
        next_arg = args_size - i == 1? "": args[i+1];
        
        if(cur_arg == "-u"){
            ui->url_edit->setText(next_arg);
            ++i;
            continue;
        }
        else if(cur_arg == "-i"){
            setIterator(b, e, eof, const_cast<char *>(next_arg.toStdString().data()));
            if(b == 0 && e == 0){
                ui->SeveralcheckBox->setChecked(false);
                ui->LowerEdit->setText("");
                ui->UpperEdit->setText("");
                
            }
            else{
                ui->SeveralcheckBox->setChecked(true);
                ui->LowerEdit->setEnabled(true);
                ui->UpperEdit->setEnabled(true);
                ui->LowerEdit->setText(QString::number(b));
                if(eof != "eof"){
                    ui->UpperEdit->setText(QString::number(e));
                }
                else{
                    ui->UpperEdit->setText("");
                }
            }
            ++i;
            continue;
        }
        else if(cur_arg == "-h"){
            if(!IsColFiealdsAvailable)
                SetAvailabilityColFields(true);
            ui->comboBox->addItem(next_arg);
            ui->comboBox->setCurrentIndex(ui->comboBox->count()-1);
            headers.push_back(next_arg.toStdString());
            ++i;
            continue;
        }
        else if(cur_arg == "-r"){
            if(!IsColFiealdsAvailable)
                SetAvailabilityColFields(true);         
            ui->regexp_edit->setText(next_arg);
            regexps.push_back(next_arg.toStdString());
            ++i;
            continue;
        }
        else if(cur_arg == "-rc"){
            if(!IsColFiealdsAvailable)
                SetAvailabilityColFields(true);         
            ui->excess_edit->setText(next_arg);
            excess.push_back(next_arg.toStdString());
            ++i;
            continue;
        }
        else if(cur_arg == "-chf"){
            ui->CharsetCheckBox->setChecked(true);
            ui->CharsetComboFrom->setEnabled(true);
            ui->CharsetComboTo->setEnabled(true);
            next_arg = FindKey(charsets, next_arg);
            int index = ui->CharsetComboFrom->findText(next_arg);
            if(index != -1){
                ui->CharsetComboFrom->setCurrentIndex(index);
            }
		}
        else if(cur_arg == "-cht"){
            ui->CharsetCheckBox->setChecked(true);
            ui->CharsetComboFrom->setEnabled(true);
            ui->CharsetComboTo->setEnabled(true);
            next_arg = FindKey(charsets, next_arg);
            int index = ui->CharsetComboTo->findText(next_arg);
            if(index != -1){
                ui->CharsetComboTo->setCurrentIndex(index);
            }
		}
        else if(cur_arg == "-chfh"){
            ui->HeaderCharsetCheckBox->setChecked(true);
            ui->HeaderCharsetComboFrom->setEnabled(true);
            ui->HeaderCharsetComboTo->setEnabled(true);
            next_arg = FindKey(charsets, next_arg);
            int index = ui->HeaderCharsetComboFrom->findText(next_arg);
            if(index != -1){
                ui->HeaderCharsetComboFrom->setCurrentIndex(index);
            }
		}
        else if(cur_arg == "-chth"){
            ui->HeaderCharsetCheckBox->setChecked(true);
            ui->HeaderCharsetComboFrom->setEnabled(true);
            ui->HeaderCharsetComboTo->setEnabled(true);
            next_arg = FindKey(charsets, next_arg);
            int index = ui->HeaderCharsetComboTo->findText(next_arg);
            if(index != -1){
                ui->HeaderCharsetComboTo->setCurrentIndex(index);
            }            
        }
        else if(cur_arg == "-une"){
            ui->UnescapedCheckBox->setChecked(true);
        }
    }
}

void ParserGUI::FillPageNumbers(QString arg)
{
    QRegularExpression re("[0-9]+-([0-9]+|eof)");
    
    QRegularExpressionMatch match;
    
    
}

//Установить соответствие кодировок Qt и libiconv
void ParserGUI::SetCharsetMatches()
{
    charsets.insert({"", ""});
    charsets.insert({"IBM 866", "CP866"});
    charsets.insert({"UTF-8", "UTF-8"});
    charsets.insert({"Windows-1251", "CP1251"});
    
    for(auto & e : charsets){
        ui->CharsetComboFrom->addItem(e.first);
        ui->CharsetComboTo->addItem(e.first);
        ui->HeaderCharsetComboFrom->addItem(e.first);
        ui->HeaderCharsetComboTo->addItem(e.first);
        ui->ContentCharsetComboFrom->addItem(e.first);
    }
}

void ParserGUI::AddConsoleArgs(QString & text, const vector<string> & array, QString arg, QString type)
{
    QString str;
    
    bool is_bat = type == ".bat";
    
    for(auto & s : array){
        str = QString::fromStdString(s).replace("\"", "\\\"");
        
        if(is_bat){
            HandleWinSpecials(str);
        }
        
        text = QString("%1 %2 \"%3\"").arg(text, arg, str);
    }
}

void ParserGUI::HandleWinSpecials(QString &text)
{
    QString specials = "<>|&^";
    
    bool insert_shield = false;
    
    for(int i = 0; i < text.size(); ++i){
        
        if(text[i] == "\""){
            insert_shield = !insert_shield;
            continue;
        }
        
        if(insert_shield && specials.contains(text[i])){
            text.insert(i++, "^");
        }
    }
    
    if(insert_shield) 
        text += "^";
}

QStringList SplitString(const QString &str, const QString &restr)
{

    regex re(restr.toStdString());
    string text = str.toStdString();

    smatch sm;

    QStringList matches;

    string value = regex_replace(text, re, "");
    string first_tag = "", last_tag = "";
    if(regex_search(text, sm, re)){
        if(sm.size() != 0){
            first_tag = sm[0].str();
        }
        text = sm.suffix().str();
    }
    if(regex_search(text, sm, re)){
        if(sm.size() != 0){
            last_tag = sm[0].str();
        }
    }

    matches.push_back(first_tag.data());
    matches.push_back(value.data());
    matches.push_back(last_tag.data());

    return matches;

}

//************************EVENTS


//Окончание редактирования url
void ParserGUI::on_url_edit_editingFinished()
{
//    if(ui->url_edit->isModified()){
//        GetContent();
//    }
//    ui->url_edit->setModified(false);
}


//Нажатие кнопки добавления столбец
void ParserGUI::on_pushButton_add_clicked()
{
    AddColumn();
}

void ParserGUI::on_pushButton_clicked()
{
    setCursor(Qt::WaitCursor);
    DoEditorParse();
    setCursor(Qt::ArrowCursor);
}

//Нажатие кнопки удаления столбца
void ParserGUI::on_pushButton_del_clicked()
{
    DelColumn();
}

//Окончания редактирования регулярного вырадения
void ParserGUI::on_regexp_edit_editingFinished()
{
//    if(ui->regexp_edit->isModified()){
//        SaveRegExp();
//        ui->textEdit->setPlainText(url_content);
//        if(ui->excess_edit->text() != ""){
//            ClearMatches();
//        }
//        else{
//            FindMatches();
//        }
//    }
//    ui->regexp_edit->setModified(false);
}

//Окончание редактирования исключающего выражения
void ParserGUI::on_excess_edit_editingFinished()
{
//    if(ui->excess_edit->isModified()){
//        SaveExcessRE();
//        ui->textEdit->setPlainText(url_content);
//        ClearMatches();
//    }
//    ui->excess_edit->setModified(false);
}

//Смена индекса combobox
void ParserGUI::on_comboBox_currentIndexChanged(int index)
{
    ColumnSelection(index);
}

void ParserGUI::on_list_mode_pushButton_toggled(bool checked)
{
    if(matches.size() == 0 || ui->textEdit->toPlainText() == "")
        return;
    if(checked){
        ui->textEdit->document()->clear();
        QString listContent = "";
        for(auto & s : matches){
            listContent += (s.size()==1?s[0].toHtmlEscaped():s[1].toHtmlEscaped()) + "<br><br>";
        }
        listContent = match_decor + listContent + "</span>";
        ui->textEdit->document()->clear();
        ui->textEdit->document()->setHtml(listContent);
        on_begin_te_pushButton_clicked();
    }
    else{
        ui->textEdit->document()->clear();
        on_pushButton_clicked();
    }   
}

//Перейти к следующему совпадению
void ParserGUI::on_te_next_pushButton_clicked()
{
    if(match_index+1 >= matches.size()){ //matches
        return;
    }
    ++match_index;
    QTextEdit * te = ui->textEdit;
    QStringList cur_match = matches[match_index];
    bool r = te->find(cur_match[0]);
    QTextCursor cu = te->textCursor();
    cu.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 3);
    te->setTextCursor(cu);
    QString m = cur_match.size()==1?cur_match[0]:cur_match[1];
    r = te->find(m, QTextDocument::FindBackward);
}

//Перейти к предыдушему совпадению
void ParserGUI::on_te_back_pushButton_clicked()
{
    if(match_index == 0 || matches.size() == 0){
        return;
    }
    --match_index;
    QTextEdit * te = ui->textEdit;
    QStringList cur_match = matches[match_index];
    bool r  = te->find(cur_match[0], QTextDocument::FindBackward);
    QTextCursor cu = te->textCursor();
    cu.clearSelection();
    cu.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, 3);
    te->setTextCursor(cu);
    QString m = cur_match.size()==1?cur_match[0]:cur_match[1];
    r = te->find(m);

}


void ParserGUI::on_begin_te_pushButton_clicked()
{
    if(matches.size() == 0){
        return;
    }
    match_index = -1;
    QTextEdit * te = ui->textEdit;
    QTextCursor cu(te->document());
    cu.movePosition(QTextCursor::Start);
    te->setTextCursor(cu);
    on_te_next_pushButton_clicked();
}

void ParserGUI::on_end_te_pushButton_clicked()
{
    int size = matches.size();
    if(size == 0){
        return;
    }
    match_index = size;
    QTextEdit * te = ui->textEdit;
    QTextCursor cu(te->document());
    cu.movePosition(QTextCursor::End);
    te->setTextCursor(cu);
    on_te_back_pushButton_clicked();
}

//Нажатие кнопки выполнить
void ParserGUI::on_ExecuteButton_clicked()
{
    setCursor(Qt::WaitCursor);
    Execute();
    setCursor(Qt::ArrowCursor);
}

//Нажатие кнопки выполнить на странице настроек
void ParserGUI::on_ExecuteButton_2_clicked()
{
    setCursor(Qt::WaitCursor);
    Execute();
    setCursor(Qt::ArrowCursor);
}

// Окончание редактирования комбобокса
void ParserGUI::on_comboBox_editingFinished()
{
    SaveColumnHeader();
}


void ParserGUI::on_SeveralcheckBox_clicked(bool checked)
{
    ui->LowerEdit->setEnabled(checked);
    ui->UpperEdit->setEnabled(checked);
}

void ParserGUI::on_CharsetCheckBox_clicked(bool checked)
{
    ui->CharsetComboFrom->setEnabled(checked);
    ui->CharsetComboTo->setEnabled(checked);
}

void ParserGUI::on_HeaderCharsetCheckBox_clicked(bool checked)
{
    ui->HeaderCharsetComboFrom->setEnabled(checked);
    ui->HeaderCharsetComboTo->setEnabled(checked);    
}

void ParserGUI::on_ContentCharsetCheckBox_clicked(bool checked)
{
    ui->ContentCharsetComboFrom->setEnabled(checked);  
}

//Сохранить как
void ParserGUI::on_save_as_action_triggered()
{
    QString selected_filter;
    
    QString fullName = QFileDialog::getSaveFileName(this,
         tr("Save File"), ".", tr("CSV (*.csv)"), &selected_filter);
    
    if(fullName == "") return;
    
    selected_filter.replace(QRegularExpression(".*\\(\\*|\\)"), "");
    
    QString cur_filter = fullName.mid(fullName.lastIndexOf("."));
   
    if(cur_filter != selected_filter)
        fullName += selected_filter;
    
    QFile file(fullName);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::information(this, tr("Ошибка записи"), tr("Не удалось сохранить файл"));
        return;
    }

    QTextStream stream(&file);
    
    if(selected_filter == ".csv"){
        WriteCSV(stream);
    }
    
    fileName = fullName;
    
    file.close();   
}

//Сохранить в файл
void ParserGUI::on_save_action_triggered()
{
    if(fileName == ""){
        on_save_as_action_triggered();
        return;
    }
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::information(this, tr("Ошибка записи"), tr("Не удалось сохранить файл"));
        return;
    }   
    QTextStream stream(&file);
    
    QString selected_filter = fileName.mid(fileName.lastIndexOf("."));
    
    if(selected_filter == ".csv"){
        WriteCSV(stream);
    }
    
    file.close();  
}

void ParserGUI::on_save_script_as_action_triggered()
{
    QString selected_filter;
    
    QString fullName = QFileDialog::getSaveFileName(this,
         tr("Save File"), ".", tr("bat (*.bat);;bash (*.sh)"), &selected_filter);
    
    if(fullName == "") return;
    
    selected_filter.replace(QRegularExpression(".*\\(\\*|\\)"), "");
    
    QString cur_filter = fullName.mid(fullName.lastIndexOf("."));
   
    if(cur_filter != selected_filter)
        fullName += selected_filter;
    
    QFile file(fullName);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::information(this, tr("Ошибка записи"), tr("Не удалось сохранить файл"));
        return;
    }

    QTextStream stream(&file);
    
    WriteScript(stream, selected_filter);
    
    script_fileName = fullName;
    
    file.close();       
}


void ParserGUI::on_save_script_action_triggered()
{
    if(script_fileName == ""){
        on_save_script_as_action_triggered();
        return;
    }
    QFile file(script_fileName);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::information(this, tr("Ошибка записи"), tr("Не удалось сохранить файл"));
        return;
    }   
    QTextStream stream(&file);
    
    QString selected_filter = script_fileName.mid(script_fileName.lastIndexOf("."));
    
    WriteScript(stream, selected_filter);
    
    file.close();   
}

//Заполнить поля из сохраненного ранее скрипта
void ParserGUI::on_open_action_triggered()
{
    QString selected_filter;
    
    QString fullName = QFileDialog::getOpenFileName(this,
         tr("Открыть файл"), ".", tr("bat (*.bat);;bash (*.sh)"), &selected_filter);
    
    if(fullName == "") return;
    
    selected_filter.replace(QRegularExpression(".*\\(\\*|\\)"), "");
    
    //QString cur_filter = fullName.mid(fullName.lastIndexOf("."));
    
    QFile file(fullName);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::information(this, tr("Ошибка открытия файла"), tr("Не удалось открыть файл"));
        return;
    }

    QTextStream stream(&file);
    
    ReadScript(stream, selected_filter);
    
    script_fileName = fullName;
    
    file.close();    
}




template<class MKEY, class MVAL>
MKEY ParserGUI::FindKey(map<MKEY, MVAL> container, MVAL val)
{
    for(auto e : container){
        if(e.second == val){
            return e.first;
        }
    }
    return MKEY();
}


void ParserGUI::on_pushButton_col_up_clicked()
{
    QComboBox * cbox = ui->comboBox;
    int cur_index = cbox->currentIndex();
    if(cur_index == 0 || cur_index == -1
            || static_cast<long>(headers.size()) <= 1
            || static_cast<long>(regexps.size()) <= 1
            || static_cast<long>(excess.size()) <= 1) return;
    
    headers[cur_index].swap(headers[cur_index-1]);
    regexps[cur_index].swap(regexps[cur_index-1]);
    excess[cur_index].swap(excess[cur_index-1]);
    
    QString cur_text = cbox->currentText();
    QString prev_text = cbox->itemText(cur_index-1);
    cbox->setCurrentIndex(cur_index-1);
    cbox->setCurrentText(cur_text);
    cbox->setItemText(cur_index, prev_text);

}

void ParserGUI::on_pushButton_col_down_clicked()
{
    QComboBox * cbox = ui->comboBox;
    int cur_index = cbox->currentIndex();
    int out_index = cbox->count() - 1;
    if(cur_index >= out_index || cur_index == -1
            || out_index >= static_cast<long>(headers.size())
            || out_index >= static_cast<long>(regexps.size())
            || out_index >= static_cast<long>(excess.size())) return;
    
    headers[cur_index].swap(headers[cur_index+1]);
    regexps[cur_index].swap(regexps[cur_index+1]);
    excess[cur_index].swap(excess[cur_index+1]);
    
    QString cur_text = cbox->currentText();
    QString next_text = cbox->itemText(cur_index+1);
    cbox->setCurrentIndex(cur_index+1);
    cbox->setCurrentText(cur_text);
    cbox->setItemText(cur_index, next_text);  
}


void ParserGUI::on_LimitCheckBox_clicked(bool checked)
{
    ui->LimitEdit->setEnabled(checked);
}


void ParserGUI::on_LowerEdit_textEdited(const QString &arg1)
{
    OnlyDigitInLineEdit(ui->LowerEdit, arg1);
}

void ParserGUI::on_UpperEdit_textEdited(const QString &arg1)
{
    OnlyDigitInLineEdit(ui->UpperEdit, arg1);
}

void ParserGUI::on_LimitEdit_textEdited(const QString &arg1)
{
    OnlyDigitInLineEdit(ui->LimitEdit, arg1);
}


void ParserGUI::on_FindAction_triggered()
{
    if(ui->SearchWidget->maximumHeight() == 0)
        ui->SearchWidget->setMaximumHeight(16777215);
    else
        ui->SearchWidget->setMaximumHeight(0);
}

void ParserGUI::on_FindButton_clicked()
{
    QString search = ui->FindEdit->text();
    
    if(ui->RegSearchButton->isChecked()){
        ui->textEdit->find(QRegExp(search));
    }
    else{
       ui->textEdit->find(search); 
    } 
}

void ParserGUI::on_FindButton_2_clicked()
{
    QString search = ui->FindEdit->text();
    if(ui->RegSearchButton->isChecked()){
        ui->textEdit->find(QRegExp(search), QTextDocument::FindBackward);
    }
    else{
       ui->textEdit->find(search, QTextDocument::FindBackward); 
    } 
}

void ParserGUI::on_pushButton_2_clicked()
{
    setCursor(Qt::WaitCursor); 
}






























