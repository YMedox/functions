/*-------------------------------------------------------------------------------------------------------------------------------
Пример работы с библиотекой iniobject.
Функция main подробно прокомментирована.
Для сборки требует наличия glib-2.0 на машине.

Автор - Ярослав Медокс
-------------------------------------------------------------------------------------------------------------------------------*/

#include <iostream>
#include "iniobject.hpp"

using namespace std;
int delimLength;
const int blockLength = 15;

//Выводит на экран прочтенную и разобранную строку из конф-файла
template <typename T> void reportString(const char *key, T &value) {
    cout<<"Key '"<<key<<"' -> value is '"<<value<<"'"<<endl;
}
void reportGroup(const char *group) {
  cout<<"Group '"<<group<<"':"<<endl;
}
//Три служебные функции для вывода разделителей логических блоков примера
void printDelim(int num) {
   for(int i=0;i<num;i++) cout<<"-";
}
void printStartDelimiter(const char *msg) {
   printDelim(blockLength);
   cout<<" "<<msg<<" ";
   printDelim(blockLength);
   cout<<endl;
   delimLength = blockLength + strlen(msg) + 2 + blockLength;
}
void printEndDelimiter() {
   printDelim(delimLength);
   cout<<"\n"<<endl;
}

int main() {

    printStartDelimiter("Content of source conf file for reference");
    system("rm conf_ex.conf & cp conf_ex.src conf_ex.conf");            //создаем конф-файл из исходника. Сделано исключительно для сохранности исходного файла. В реальной программе это не нужно
    system("cat conf_ex.conf");                                         //показываем скопированный исходник для наглядности
    cIniObject ini("conf_ex.conf");                                     //начинаем работу здесь. Если имя файла совпадает с исполняемым файлом, то можно ничего не указывать. Конф-файл создастся рядом с исполняемым файлом
    printEndDelimiter();

    printStartDelimiter("Reading first group");
    char *group = (char *)"group1";
    reportGroup(group);
    //читаем "в лоб", это основной метод. Функций чтения больше, чем три, см.библиотеку.
    char *char_value = ini.getString(group, "strkey", true);            //если передавать false, то нужно проверять возвращаемое значение
    reportString("strKey", char_value);
    int    int_value = ini.getInt(group, "intkey", 1000);               //1000 не запишется в конф-файл, так как значение установлено
    reportString("intkey", int_value);
    double dbl_value = ini.getDouble(group, "dblkey", -1.234);          //аналогично
    reportString("dblkey", dbl_value);
    printEndDelimiter();

    //Теперь читаем сразу всю группу парой вызовов. Удобно если количество ключей - переменно или неизвестно заранее.
    //А сами ключи могут указывать на станартизованные группы.
    printStartDelimiter("Reading second group");
    group = (char *)"group2";
    reportGroup(group);
    gsize lines;
    char **keys;
    char **values;
    keys = ini.getKeys(group, &lines);
    if(lines) {
      values = (char**)malloc(sizeof(char*)*lines);  //в реальной программе нужно 1) проверить что вернул malloc. 2) вызвать free() после использования
      for(gsize i=0;i<lines;i++) {
        values[i] = ini.getString(group, keys[i], false);
        reportString(keys[i], values[i]);
      }
    }
    printEndDelimiter();

    //Теперь посмотрим что в группах, которые описаны были в group2
    //При этом, если их нет, то создавать не будем
    printStartDelimiter("Reading secondary groups");
    if(lines) {
      bool bFound = false;
      for(gsize i=0;i<lines;i++) {
        if(ini.hasGroup(values[i])) {  // только если группа существует
          if(bFound) cout<<endl;
          reportGroup(values[i]);
          char *main = ini.getString(values[i], "main", false);
          reportString("main", main);
          int age = ini.getInt(values[i], "age", 0);
          reportString("age", age);
          bFound = true;
        }
      }
    }
    if(*values) free(*values);        //больше не будем пользоваться. Удалять надо все, что мы создавали сами. Все остальное высвободит glib при удалении объекта cIniObject
    printEndDelimiter();

    //Метод 1 создания групп и ключей. Просто пробуем читать значения. Если ключая не найдено, он будет создан со значением по умолчанию
    //Будет также создана и группа, если ее нет еще.
    //Если не хотите, чтобы ключ создавался, читайте значения после положительного вызова hasKey()
    //Внимание! Такая логика работает для всех чтений, кроме getString(). Строки по умолчанию не создаются в конф-файле, только выделяется память. Используйте setString явно.
    printStartDelimiter("Creating group/keys method 1");
    group = (char *)"method1";
    reportGroup(group);
    char *main = ini.getString(group, (char*)"main", (char*)"Method 1 will not work for strings");  //так не работает
    int age = ini.getInt(group, (char*)"age", 20);                                                  //а так работает
    reportString((char*)"main", main);      //Вот эта строка покажет, что ключ есть, но в файл он не будет записан!
    reportString((char*)"age", age);
    free(main);                             //память выделялась в getString()
    printEndDelimiter();

    //Метод 2 создания групп и ключей. Создаем значения явно. Группа создастся автоматически, если ее нет еще.
    //Все ключи создаются, включая строковые
    printStartDelimiter("Creating group/keys method 2");
    group = (char *)"method2";
    reportGroup(group);
    //сначала создаем
    ini.setString(group, (char*)"main", (char*)"Creating group/keys by method 2");
    ini.setInt(group, (char*)"age", 21);
    //потом читаем
    main = ini.getString(group, (char*)"main", false);  //эта функция точно не создает ключ
    age = ini.getInt(group, (char*)"age", 100);         //сравните значение по умолчанию 100 с тем, что будет в итоговом файле
    reportString((char*)"main", main);
    reportString((char*)"age", age);
    ini.setComment(group, NULL, (char*)"Adding comment to the group");
    ini.setComment(group, (char*)"age", (char*)"Adding comment to the key");
    printEndDelimiter();

    //в заключение просто выведем на экран что у нас получилось с целевым конф-файлом
    printStartDelimiter("Look at the resuting conf file in memory");
    char *data = ini.getData();             //Не читаем из файла. Его там еще нет. Пока все действия выполнялись в памяти
    cout<<data<<endl;                       //данные автоматически запишутся в файл при уничтожении объекта ini. Или принудительно вызовом saveConfig
    printEndDelimiter();
    cout<<"Don't forget to explore result of this program, file 'conf_ex.conf'"<<endl;

    return 0;
}
