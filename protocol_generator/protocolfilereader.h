#ifndef PROTOCOLFILEREADER_H
#define PROTOCOLFILEREADER_H

#include <QString>
#include <QVector>
#include <QSet>

struct EnumConfig
{
    struct ElementItem
    {
        QString name;
        QString describe;
    };

   QString name;
   QVector<ElementItem> element_list;
};

struct ProtocolConfig
{
    struct AttributeItem
    {
        QString name;
        QString type;
        QString subtype;
        QString describe;
    };

    QString name;
    int msg_type;
    QString describe;

    QVector<AttributeItem> attr_list;
};

class ProtocolFileReader
{
public:
    ProtocolFileReader();
    ~ProtocolFileReader();

    bool Reload(const QString &xml_file_path);

    QString GetLastErrorStr() { return last_error_str_; }
    const QVector<ProtocolConfig> &GetProtocolCfgList() { return protocol_list_; }
    const  QVector<EnumConfig> &GetEnumCfgList() { return enum_list_; }

private:
    QString last_error_str_;

    QVector<EnumConfig> enum_list_;
    QVector<ProtocolConfig> protocol_list_;

    QSet<QString> proto_name_set_;
};

#endif // PROTOCOLFILEREADER_H
