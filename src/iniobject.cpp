/*-------------------------------------------------------------------------------------------------------
Библиотека для работы с классическими conf файлами формата:
[group name]
key1=value1
key2=value2

Представляет собой обертку надо вызовами библиотеки glib-2.0, которая должна быть установлена на машине.
Конфигурационный файл создается, если не передан путь, в папке с исполняемым файлом, таким же именем и
расширением conf.
Логика работы прямая: читаем с помощью getSomething, пишем с помощью setSomething.
Важно! Для удобства работы в реальных проектах, функции типа getSоmething (кроме getString) создают ключи со
значениями по умолчанию, если они не были заданы в конф-файле. Это позволяет видеть гарантированно все
настраиваемые параметры (кроме строковых). И если не запрещено сохранять с помощью setAllowSave(false).

Автор - Ярослав Медокс
-------------------------------------------------------------------------------------------------------*/
#include "iniobject.hpp"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


//При инициации всегда проверяем наличие файла и его открываемость
cIniObject::cIniObject(const char *filename) {
    GError   *error = NULL;
    _bData = false;
    fillPath(filename);
    createKeyFile();
    int f = open(path, O_CREAT, S_IRWXU);
    if(f != -1) close(f);
    else {
        g_warning ("Error opening key file %s", path);
        return;
    }
    if (!g_key_file_load_from_file (key_file, path, G_KEY_FILE_KEEP_COMMENTS, &error)) {
      if (!g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT)) g_warning ("Error loading key file: %s", error->message);
      g_error_free(error);
    }
}

//без параметров - создаем конфигурационный файл
cIniObject::cIniObject() {
    _bData = true;
    createKeyFile();
}

//создает объект конф-файла
void cIniObject::createKeyFile() {
    key_file = g_key_file_new();
    setAllowSave(true);
}

//позволяет загрузить в конф-файл из переменной data
bool cIniObject::fillFromData(const gchar* data, gsize length) {
    GError   *error = NULL;
    _bData = true;
    if (!g_key_file_load_from_data (key_file, data, length, G_KEY_FILE_KEEP_COMMENTS, &error)) {
      g_warning ("Error loading key file from data: %s", error->message);
      g_error_free(error);
      return false;
    }
    return true;
}

//Возвращает контент конф-файла
gchar* cIniObject::getData() {
  return g_key_file_to_data(key_file, NULL, NULL);
}

//Если не был явно указано имя конф-файла, то ищем его рядом с исполняемым файлом и таким же именем + расширение .conf
void cIniObject::fillPath(const char *filename) {
    memset(path, 0, _INI_PATH_MAX);
    if(filename == NULL) {
      ssize_t len = readlink("/proc/self/exe", path, _INI_PATH_MAX-1);
      path[len]   = 0;
      strcpy(&path[len], ".conf");
    } else {
      unsigned int n;
      char* p = strrchr(path, '/');
      if(p) n = p - &path[0];
      else  n = 0;
      strcpy(&path[n], filename);
    }
}

//Путь к файлу
char* cIniObject::getPath() {
  return path;
}

//Запрет или разрешение на внесение изменений в конф-файл в процессе работы программы
void cIniObject::setAllowSave(bool bAllow){
  _bAllowSaveData = bAllow;
}
bool cIniObject::getAllowSave() {
  return _bAllowSaveData;
}

//Получить значение типа int из конф-файла по группе и ключу, с заданием значения по умолчанию
//Если ключ не найден он будет создан и сохранен в конф-файле
int cIniObject::getInt(const gchar *group_name, const gchar *key, int default_value) {
  GError   *error = NULL;
  int ret_val = g_key_file_get_integer (key_file, group_name, key, &error);
  if(ret_val == 0) {
    if(g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND) || g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND) || g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE)) {
      g_error_free(error);
      setInt(group_name, key, default_value);
      return default_value;
    }
  }
  return ret_val;
}
//Устанаваливает значение ключа типа int
void cIniObject::setInt(const gchar *group_name, const gchar *key, int value) {
  checkPresence(group_name, key);
  g_key_file_set_integer (key_file, group_name, key, value);
}

//Проверяет наличие ключа. Если его нет, создается ключ со значением "`"
void cIniObject::checkPresence(const gchar *group_name, const gchar *key) {
  if(!g_key_file_has_group(key_file, group_name)) g_key_file_set_value(key_file, group_name, key, "`");
}

//Получить значение типа double из конф-файла по группе и ключу, с заданием значения по умолчанию
//Если ключ не найден он будет создан и сохранен в конф-файле
double cIniObject::getDouble(const gchar *group_name, const gchar *key, double default_value) {
  GError   *error = NULL;
  double ret_val = g_key_file_get_double (key_file, group_name, key, &error);
  if(ret_val == 0) {
    if(g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND) || g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND) || g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE)) {
      g_error_free(error);
      setDouble(group_name, key, default_value);
      return default_value;
    }
  }
  return ret_val;
}
//Устанаваливает значение ключа типа double
void cIniObject::setDouble(const gchar *group_name, const gchar *key, double value) {
//  if(!g_key_file_has_group(key_file, group_name)) g_key_file_set_value(key_file, group_name, key, "`");
  checkPresence(group_name, key);
  g_key_file_set_double (key_file, group_name, key, value);
}

//Получить значение типа uint16_t из конф-файла по группе и ключу, с заданием значения по умолчанию
//Если ключ не найден он будет создан и сохранен в конф-файле
uint64_t cIniObject::getUInt(const gchar *group_name, const gchar *key, uint64_t default_value) {
  GError   *error = NULL;
  uint64_t ret_val = g_key_file_get_uint64 (key_file, group_name, key, &error);
  if(ret_val == 0) {
    if(g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND) || g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND) || g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE)) {
      g_error_free(error);
      setUInt(group_name, key, default_value);
      return default_value;
    }
  }
  return ret_val;
}
//Устанаваливает значение ключа типа uint16_t
void cIniObject::setUInt(const gchar *group_name, const gchar *key, uint64_t value) {
  checkPresence(group_name, key);
  g_key_file_set_uint64 (key_file, group_name, key, value);
}

//Получить значение типа char* из конф-файла по группе и ключу, с указанием создавать ключ или нет
char* cIniObject::getString(const gchar *group_name, const gchar *key, bool create) {
  GError   *error = NULL;
  char *ret_val = g_key_file_get_string (key_file, group_name, key, &error);
  if(ret_val == NULL) {
    if(g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND) || g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND) || g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE)) {
      g_error_free(error);
      if(create) setString(group_name, key, (char*)"");
      return NULL;
    }
  }
  if(ret_val) { if(strlen(ret_val) == 0) return NULL; }
  return ret_val;
}
//Получить значение типа char* из конф-файла по группе и ключу, с заданием значения по умолчанию
//Если ключ не найден он НЕ будет создан и сохранен в конф-файле
char* cIniObject::getString(const gchar *group_name, const gchar *key, gchar *default_str) {
  GError   *error = NULL;
  char *ret_val = g_key_file_get_string (key_file, group_name, key, &error);
  if(ret_val == NULL) {
      if(default_str){
        ret_val = static_cast<char*>(malloc(strlen(default_str)+1));
        if(ret_val) { strcpy(ret_val, default_str); }
        else        { g_warning("Error allocating memory in %s line %d", __FUNCTION__, __LINE__); }
      } else {
        g_warning("Wrong default_str in %s line %d", __FUNCTION__, __LINE__);
      }
  }
  return ret_val;
}
//Устанавливает значение строкового ключа
void cIniObject::setString(const gchar *group_name, const gchar *key, char *str) {
  checkPresence(group_name, key);
  g_key_file_set_string (key_file, group_name, key, str);
}
//Устанавливает комментарий
void cIniObject::setComment(const gchar *group_name, const gchar *key, char *comment) {
  g_key_file_set_comment (key_file, group_name, key, comment, NULL);
}

//принудительно сохраняет файл, даже если _bAllowSaveData = false
bool cIniObject::saveConfig() {
  if(!_bData) { //только если конфиг взят непосредственно из файла, а не внешние данные.
    if (!g_key_file_save_to_file (key_file, path, NULL)) {
      g_warning ("Error saving key file: %s", path);
      return false;
    }
    return true;
  }
  return false;
}

//Если разрешено сохрание файл будет принудительно сохранен
cIniObject::~cIniObject() {
  if(_bAllowSaveData) {  //сохраняет файл, только если разрешено сохранять. (по умолчанию - разрешено)
    saveConfig();
  }
  g_key_file_free(key_file);
}

//Возвращает массив указателей на имена ключей в группе
char **cIniObject::getKeys(const char *group_name, gsize *length) {
  return g_key_file_get_keys(key_file, group_name, length, NULL);
}
//Возвращает массив указателей на список групп
char **cIniObject::getGroups(gsize *length) {
  return g_key_file_get_groups(key_file, length);
}
//Удаляет указанный ключ из указанной группы
bool cIniObject::removeKey(const gchar *group_name, const gchar *key) {
  return g_key_file_remove_key(key_file, group_name, key, NULL);
}
//Удаляет комментарий
bool cIniObject::removeComment(const gchar *group_name, const gchar *key) {
  return g_key_file_remove_comment(key_file, group_name, key, NULL);
}

//Проверяет есть ли группа в конф-файле
bool  cIniObject::hasGroup(const gchar *group_name) {
  return g_key_file_has_group(key_file, group_name);
}

//Проверяет есть ли ключ указанной группе
bool  cIniObject::hasKey(const gchar *group_name, const gchar *key) {
  return g_key_file_has_key(key_file, group_name, key, NULL);
}
