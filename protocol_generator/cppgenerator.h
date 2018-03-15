#ifndef CPPGENERATOR_H
#define CPPGENERATOR_H

#include <QString>

#include "protocolfilereader.h"

class CppGenerator
{
public:
    CppGenerator();

    bool Generate(ProtocolFileReader &reader, const QString &cpp_file_path);

protected:
    QString GetRealType(const QString &type, const QString &sub_type);
    QString GetWriteType(const QString &type);
    QString GetReadType(const QString &type);
    QString GetSerializeStr(const ProtocolConfig::AttributeItem &item);
    QString GetUnserializeStr(const ProtocolConfig::AttributeItem &item);
};

#endif // CPPGENERATOR_H
