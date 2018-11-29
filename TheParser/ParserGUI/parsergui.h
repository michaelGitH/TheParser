#ifndef PARSERGUI_H
#define PARSERGUI_H

#include <QMainWindow>
#include <QString>
#include <QMessageBox>
#include <QRegExp>
#include <QTextCursor>
#include <QTextCodec>
#include <QDebug>
#include <QStringList>
#include <QTextBlock>
#include <QFileDialog>
#include <QLineEdit>
#include <QSplashScreen>
#include <map>

#include "parser.h"
#include "consolemode.h"


QStringList SplitString(const QString &str, const QString &restr);

namespace Ui {
class ParserGUI;
}

class ParserGUI : public QMainWindow
{
    Q_OBJECT
    
public:
    
    enum TE_MODE{TEXT_MODE, LIST_MODE};
    
    enum FILE_FORMAT{NONE, CSV};
    
    explicit ParserGUI(QWidget *parent = 0);
    
    ~ParserGUI();
    
    void GetContent(); // Загружает содержимое сайта url_edit в TextEdit
    
    void SaveColumnHeader(); //Сохранить заголовок столбца
    
    void SaveRegExp(); //Сохранить значение регулярного выражения
    
    void SaveExcessRE(); //Сохранить значение исключающего выражения
    
    void AddColumn(); //Добавить столбец
    
    void DelColumn(); //Удалить столбец
    
    void ColumnSelection(int index); //Выбор столбца
    
    void FindMatches(); //Найти соответствия

    void ClearMatches(); //Убрать лишнее из найденных соответсвий
    
    //Включить\Отключить доступность полей столбца
    void SetAvailabilityColFields(bool on);
    
    void SetStyle();
    
    void setText_edit_decor();
    
    //Проверка перед выгрузкой
    bool CheckToExecute();
    
    //Сохранить как csv
    void WriteCSV(QTextStream & stream);
    
    //Выполнить
    void Execute();
    
    //Записать bat/bash файл для консольной версии
    void WriteScript(QTextStream & stream, QString type);
    
    //Заполнить поля из сохраненного ранее скрипта
    void ReadScript(QTextStream & stream, QString type);
    
    void ClearFields();
    
    template<class MKEY, class MVAL>
    MKEY FindKey(map<MKEY, MVAL> container, MVAL val); 
    
    //В редакторе должны присутствовать только цифры
    void OnlyDigitInLineEdit(QLineEdit * edit, const QString & arg);
    
    //Установить цвет выделения в textedit
    void SetSelectionColor();
    
    //Заполнить TableWidget
    void FillTableWidget();
    
    //Выполнить прасинг в редакторе
    void DoEditorParse();
 
private slots:
    void on_pushButton_clicked();
    
    //Окончание редактирования url
    void on_url_edit_editingFinished();
    
    //Нажатие кнопки добавления столбец
    void on_pushButton_add_clicked();
    
    //Нажатие кнопки удаления столбца
    void on_pushButton_del_clicked();
    
    //Окончания редактирования регулярного вырадения
    void on_regexp_edit_editingFinished();
    
    //Смена индекса combobox
    void on_comboBox_currentIndexChanged(int index);
    
    //Окончание редактирования исключающего выражения
    void on_excess_edit_editingFinished();
    
    void on_list_mode_pushButton_toggled(bool checked);
    
    void on_te_next_pushButton_clicked();
    
    void on_te_back_pushButton_clicked();
    
    void on_begin_te_pushButton_clicked();

    void on_end_te_pushButton_clicked();
    
    //Нажатие кнопки выполнить
    void on_ExecuteButton_clicked();
    
    //Нажатие кнопки выполнить на странице настроек
    void on_ExecuteButton_2_clicked();
    
    // Окончание редактирования комбобокса
    void on_comboBox_editingFinished(); 
    
    //Парсить несколько страниц, или одну
    void on_SeveralcheckBox_clicked(bool checked);
    
    //Сохранить в файл
    void on_save_action_triggered();
    
    void on_CharsetCheckBox_clicked(bool checked);
    
    void on_HeaderCharsetCheckBox_clicked(bool checked);
    
    void on_save_as_action_triggered();
    
    void on_save_script_action_triggered();
    
    void on_save_script_as_action_triggered();
    
    //Заполнить поля из сохраненного ранее скрипта
    void on_open_action_triggered();
    
    
    void on_ContentCharsetCheckBox_clicked(bool checked);
    
    void on_pushButton_col_up_clicked();
    
    void on_pushButton_col_down_clicked();
    
    void on_LimitCheckBox_clicked(bool checked);
    
    void on_LowerEdit_textEdited(const QString &arg1);
    
    void on_UpperEdit_textEdited(const QString &arg1);
    
    void on_LimitEdit_textEdited(const QString &arg1);
    
    void on_FindAction_triggered();
    
    void on_FindButton_clicked();
    
    void on_FindButton_2_clicked();
    
    void on_pushButton_2_clicked();
    
private:
    Ui::ParserGUI *ui;
    
    bool IsColFiealdsAvailable; //Флаг доступности полей столбца

    QString url_content;
    QString list_content;
    long match_index;
    
    QString match_decor;
    
    QString text_edit_decor;
    
    QString fileName;
    
    QString script_fileName;
    
    vector<QStringList> matches; //Найденные совпадения регулярному выражению
    
    FILE_FORMAT file_format = CSV;
    
    vector<string> headers; // Контейнер заголовков столбцов таблицы
    vector<string> regexps; // Контейнер регулряных выражений для столбцов таблицы 
    vector<string> excess; // Контейнер регулярных выражений для отброса лишних символов 
    vector<vector<string>> match_result;
    map<QString, QString> charsets;
    
    void AddConsoleArgs(QString & text, const vector<string> & array, QString arg, QString type);
    
    void HandleWinSpecials(QString & text);
    
    QString GetNextArg(QString & script, QString type);
    
    void FillArgs(QStringList & args);
    
    void FillPageNumbers(QString arg);
    
    //Установить соответствие кодировок Qt и libiconv
    void SetCharsetMatches();
    
    
    
};

#endif // PARSERGUI_H
