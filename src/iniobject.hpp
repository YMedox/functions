#ifndef INIOBJECT_HPP
#define INIOBJECT_HPP

#include <glib.h>
#include <glib/gprintf.h>
#include <stdint.h>

#define _INI_PATH_MAX 1024
class cIniObject
{
    public:
        cIniObject(const char *filename);
        cIniObject();
        virtual ~cIniObject();
        bool fillFromData(const gchar* data, gsize length);
        gchar* getData();
        void fillPath(const char *filename);
        char *getPath();
        int  getInt(const gchar *group_name, const gchar *key, int default_value);
        void setInt(const gchar *group_name, const gchar *key, int value);
        double getDouble(const gchar *group_name, const gchar *key, double default_value);
        void   setDouble(const gchar *group_name, const gchar *key, double value);
        uint64_t getUInt(const gchar *group_name, const gchar *key, uint64_t default_value);
        void setUInt(const gchar *group_name, const gchar *key, uint64_t value);
        char *getString(const gchar *group_name, const gchar *key, bool create = true);
        char *getString(const gchar *group_name, const gchar *key, char *default_str);
        void setString(const gchar *group_name, const gchar *key, char *str);
        void setComment(const gchar *group_name, const gchar *key, char *comment);
        bool removeKey(const gchar *group_name, const gchar *key);
        bool removeComment(const gchar *group_name, const gchar *key);
        char **getKeys(const char *group_name, gsize *length);
        char **getGroups(gsize *length);
        void setAllowSave(bool bAllow);
        bool getAllowSave();
        bool saveConfig();
        bool hasGroup(const gchar *group_name);
        bool hasKey(const gchar *group_name, const gchar *key);
    protected:
        char path[_INI_PATH_MAX];
    private:
        void createKeyFile();
        GKeyFile *key_file;
        void checkPresence(const gchar *group_name, const gchar *key);
        bool _bData;
        bool _bAllowSaveData;

};

#endif // INIOBJECT_H
