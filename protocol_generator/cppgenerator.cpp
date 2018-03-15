#include "cppgenerator.h"
#include "common_tools.hpp"

#include <QFile>
#include <QTextStream>

const static char *HPP_HEADER_CODE_STR = "#pragma once\n\n#include \"netmsg/serialize_base.hpp\"\n#include <string>\n#include <vector>\n\nnamespace Protocol {\n\n";
const static char *HPP_CLASS_CODE_TEMPLETE = "    /*\n     * %s\n     */\n    class %s : public MsgSerializeBase\n    {\n    public:\n        static const int MSG_TYPE = %d;\n\n%s\n        virtual void Serialize(StaticByteArray &collector) const;\n        virtual void Unserialize(StaticByteArray &collector);\n        virtual int GetMsgType() const { return MSG_TYPE; }\n    };\n    DEFINE_PROTOCOL_DESCRIBE(%s);\n\n";
const static char *CPP_HEADER_CODE_STR = "#include \"netmsg/static_byte_array.hpp\"\n#include \"%s.hpp\"\n\nnamespace Protocol {\n\n";
const static char *CPP_SERIALIZE_CODE_STR = "    void %s::Serialize(StaticByteArray &collector) const\n    {\n%s    }\n\n    void %s::Unserialize(StaticByteArray &collector)\n    {\n%s    }\n\n";

CppGenerator::CppGenerator()
{

}

extern QString FixLength(const QString &str, int max_len);
/*
{
    QString fix_str = str;
    for (int i = str.length(); i < max_len; ++i)
        fix_str += " ";
    return fix_str;
}
*/

bool CppGenerator::Generate(ProtocolFileReader &reader, const QString &file_path)
{
    QString proto_name = file_path.mid(file_path.lastIndexOf("/") + 1);
    QString hpp_code_str;
    QString cpp_code_str;

    hpp_code_str = HPP_HEADER_CODE_STR;
    cpp_code_str.sprintf(CPP_HEADER_CODE_STR, proto_name.toStdString().c_str());

    QString tmp_str;
    QString CREATE_PROTOCOL_DESCRIBE_STR;
    QString serialize_code_str;
    QString enum_str;

    // 生成枚举部分
    const QVector<EnumConfig> &enum_cfg_list = reader.GetEnumCfgList();
    for (EnumConfig enum_cfg : enum_cfg_list)
    {
        int max_len = 0;
        for (EnumConfig::ElementItem item_cfg : enum_cfg.element_list)
        {
            if (item_cfg.name.length() > max_len)
                max_len = item_cfg.name.length();
        }

        enum_str += "    enum " + enum_cfg.name + "\n    {\n";
        for (EnumConfig::ElementItem item_cfg : enum_cfg.element_list)
        {
            enum_str += ("        " + enum_cfg.name + "_" + FixLength(item_cfg.name + ",", max_len + 1) + " // " + item_cfg.describe + "\n");
        }
        enum_str += "    };\n\n";
    }
    hpp_code_str += enum_str;

    // 生成协议类部分
    const QVector<ProtocolConfig> &proto_cfg_list = reader.GetProtocolCfgList();
    for (ProtocolConfig proto_cfg : proto_cfg_list)
    {
        QString attr_list_str;
        QString serialize_str;
        QString unserialize_str;
        for (ProtocolConfig::AttributeItem item : proto_cfg.attr_list)
        {
            attr_list_str += ("        " + this->GetRealType(item.type, item.subtype) + " " + item.name + ";");
            if (item.describe.length() > 0) // 有描述则增加注释描述
                attr_list_str += (" // " + item.describe);
            attr_list_str += "\n";

            serialize_str += this->GetSerializeStr(item);
            unserialize_str += this->GetUnserializeStr(item);
        }
        if (serialize_str.length() <= 0) serialize_str = "\n"; // 至少给个换行
        if (unserialize_str.length() <= 0) unserialize_str = "\n"; // 至少给个换行

        tmp_str = "";
        tmp_str.sprintf(HPP_CLASS_CODE_TEMPLETE,
                        proto_cfg.describe.toStdString().c_str(),
                        proto_cfg.name.toStdString().c_str(),
                        proto_cfg.msg_type,
                        attr_list_str.toStdString().c_str(),
                        proto_cfg.name.toStdString().c_str());
        hpp_code_str += tmp_str;

        CREATE_PROTOCOL_DESCRIBE_STR += "    CREATE_PROTOCOL_DESCRIBE(" + proto_cfg.name + ");\n";

        tmp_str = "";
        tmp_str.sprintf(CPP_SERIALIZE_CODE_STR,
                        proto_cfg.name.toStdString().c_str(),
                        serialize_str.toStdString().c_str(),
                        proto_cfg.name.toStdString().c_str(),
                        unserialize_str.toStdString().c_str());
        serialize_code_str += tmp_str;
    }
    hpp_code_str += "}"; // 名空间结尾
    cpp_code_str += (CREATE_PROTOCOL_DESCRIBE_STR + "\n" + serialize_code_str + "}");

    // 写入文件
    QFile hpp_path_file(file_path + ".hpp");
    if (hpp_path_file.open(QFile::WriteOnly))
    {
        QTextStream out(&hpp_path_file);
        out.setCodec("gb18030");
        out << hpp_code_str;
    }
    hpp_path_file.close();

    QFile cpp_path_file(file_path + ".cpp");
    if (cpp_path_file.open(QFile::WriteOnly))
    {
        QTextStream out(&cpp_path_file);
        out.setCodec("gb18030");
        //out.setCodec("UTF-8");
        out << cpp_code_str;
    }
    cpp_path_file.close();

    return true;
}

QString CppGenerator::GetRealType(const QString &type, const QString &sub_type)
{
    if (P_TYPE_INT8 == type)
        return "char";
    if (P_TYPE_UINT8 == type)
        return "unsigned char";
    if (P_TYPE_INT16 == type)
        return "short";
    if (P_TYPE_UINT16 == type)
        return "unsigned short";
    if (P_TYPE_INT32 == type)
        return "int";
    if (P_TYPE_UINT32 == type)
        return "unsigned int";
    if (P_TYPE_INT64 == type)
        return "long long";
    if (P_TYPE_UINT64 == type)
        return "unsigned long long";
    if (P_TYPE_STRING == type)
        return "std::string";

    if (P_TYPE_ARRAY == type)
    {
        return "std::vector<" + this->GetRealType(sub_type, "") + ">";
    }

    return type;
}

QString CppGenerator::GetWriteType(const QString &type)
{
    QString write_name_str = type;
    if (P_TYPE_INT8 == type)
        write_name_str = "WriteInt8";
    else if (P_TYPE_UINT8 == type)
        write_name_str = "WriteUint8";
    else if (P_TYPE_INT16 == type)
        write_name_str = "WriteInt16";
    else if (P_TYPE_UINT16 == type)
        write_name_str = "WriteUint16";
    else if (P_TYPE_INT32 == type)
        write_name_str = "WriteInt32";
    else if (P_TYPE_UINT32 == type)
        write_name_str = "WriteUint32";
    else if (P_TYPE_INT64 == type)
        write_name_str = "WriteInt64";
    else if (P_TYPE_UINT64 == type)
        write_name_str = "WriteUint64";
    else if (P_TYPE_STRING == type)
        write_name_str = "WriteString";
    return write_name_str;
}

QString CppGenerator::GetReadType(const QString &type)
{
    QString read_name_str = type;
    if (P_TYPE_INT8 == type)
        read_name_str = "ReadInt8";
    else if (P_TYPE_UINT8 == type)
        read_name_str = "ReadUint8";
    else if (P_TYPE_INT16 == type)
        read_name_str = "ReadInt16";
    else if (P_TYPE_UINT16 == type)
        read_name_str = "ReadUint16";
    else if (P_TYPE_INT32 == type)
        read_name_str = "ReadInt32";
    else if (P_TYPE_UINT32 == type)
        read_name_str = "ReadUint32";
    else if (P_TYPE_INT64 == type)
        read_name_str = "ReadInt64";
    else if (P_TYPE_UINT64 == type)
        read_name_str = "ReadUint64";
    else if (P_TYPE_STRING == type)
        read_name_str = "ReadString";
    return read_name_str;
}

QString CppGenerator::GetSerializeStr(const ProtocolConfig::AttributeItem &item)
{
    if (P_TYPE_INT8 == item.type || P_TYPE_UINT8 == item.type || P_TYPE_INT16 == item.type || P_TYPE_UINT16 == item.type ||
        P_TYPE_INT32 == item.type || P_TYPE_UINT32 == item.type || P_TYPE_INT64 == item.type || P_TYPE_UINT64 == item.type || P_TYPE_STRING == item.type)
    {
        return "		collector." + this->GetWriteType(item.type) + "(" + item.name + ");\n";
    }
    else if (P_TYPE_ARRAY == item.type)
    {
        QString array_code_str;
        array_code_str += "		collector.WriteUint16(static_cast<unsigned short>(" + item.name + ".size()));\n";
        array_code_str += "		for (std::vector<" + this->GetRealType(item.subtype, 0) + ">::const_iterator it = "+ item.name + ".begin(); it != "+ item.name + ".end(); ++it)\n		{\n";

        if (P_TYPE_INT8 == item.subtype || P_TYPE_UINT8 == item.subtype || P_TYPE_INT16 == item.subtype || P_TYPE_UINT16 == item.subtype ||
            P_TYPE_INT32 == item.subtype || P_TYPE_UINT32 == item.subtype || P_TYPE_INT64 == item.subtype || P_TYPE_UINT64 == item.subtype || P_TYPE_STRING == item.subtype)
            array_code_str += ("			collector." + this->GetWriteType(item.subtype) + "(*it);\n");
        else
            array_code_str += ("			it->Serialize(collector);\n");
        array_code_str += "		}\n";
        return array_code_str;
    }

    return "";
}

QString CppGenerator::GetUnserializeStr(const ProtocolConfig::AttributeItem &item)
{
    if (P_TYPE_INT8 == item.type || P_TYPE_UINT8 == item.type || P_TYPE_INT16 == item.type || P_TYPE_UINT16 == item.type ||
        P_TYPE_INT32 == item.type || P_TYPE_UINT32 == item.type || P_TYPE_INT64 == item.type || P_TYPE_UINT64 == item.type || P_TYPE_STRING == item.type)
    {
        return "		" + item.name + " = collector." + this->GetReadType(item.type) + "();\n";
    }
    else if (P_TYPE_ARRAY == item.type)
    {
        QString array_code_str;
        array_code_str += "		{\n			" + item.name + ".clear();\n			int array_count = collector.ReadUint16();\n";
        array_code_str += "			for (int i = 0; i < array_count; ++i)\n        	{\n";

        if (P_TYPE_INT8 == item.subtype || P_TYPE_UINT8 == item.subtype || P_TYPE_INT16 == item.subtype || P_TYPE_UINT16 == item.subtype ||
            P_TYPE_INT32 == item.subtype || P_TYPE_UINT32 == item.subtype || P_TYPE_INT64 == item.subtype || P_TYPE_UINT64 == item.subtype || P_TYPE_STRING == item.subtype)
            array_code_str += ("				" + item.name + ".push_back(collector." + this->GetReadType(item.subtype) + "());\n");
        else
            array_code_str += ("				" + item.subtype + " tmp_item;\n				tmp_item.Unserialize(collector);\n				" + item.name + ".push_back(tmp_item);\n");

        array_code_str += "			}\n		}\n";
        return array_code_str;
    }

    return "";
}
