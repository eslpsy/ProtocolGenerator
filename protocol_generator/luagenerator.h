#ifndef LUAGENERATOR_H
#define LUAGENERATOR_H

#include <QString>
#include "protocolfilereader.h"

class LuaGenerator
{
public:
    LuaGenerator();

    bool Generate(ProtocolFileReader &reader, const QString &lua_file_path);

protected:
    QString GetTypeInitValue(const QString &type, const QString &sub_type);
    QString GetWriteType(const QString &type);
    QString GetReadType(const QString &type);
    QString GetSerializeStr(const ProtocolConfig::AttributeItem &item);
    QString GetUnserializeStr(const ProtocolConfig::AttributeItem &item);
};

#endif // LUAGENERATOR_H
