#include "luagenerator.h"
#include "common_tools.hpp"

#include <QFile>
#include <QTextStream>

static const char *HEAD_CODE = "local BaseProtocolStruct = BaseProtocolStruct\nlocal BaseClass = BaseClass\nlocal MsgAdapter = MsgAdapter\n\nmodule \"Protocol\"\n\n";
static const char *CLASS_CODE_TEMPLETE = "\n-- %s\n%s = %s or BaseClass(BaseProtocolStruct)\nfunction %s:__init()\n    self.msg_type = %d\n\n%send\n\nfunction %s:Encode()\n%send\n\nfunction %s:Decode()\n%send\n";

LuaGenerator::LuaGenerator()
{

}

QString FixLength(const QString &str, int max_len)
{
    QString fix_str = str;
    for (int i = str.length(); i < max_len; ++i)
        fix_str += " ";
    return fix_str;
}

bool LuaGenerator::Generate(ProtocolFileReader &reader, const QString &lua_file_path)
{
    QString enum_str;
    QString class_str;

    // 生成枚举部分
    const QVector<EnumConfig> &enum_cfg_list = reader.GetEnumCfgList();
    for (EnumConfig enum_cfg : enum_cfg_list)
    {
        enum_str += enum_cfg.name + " = {\n";
        int enum_num = 0;
        int max_len = 0;
        for (EnumConfig::ElementItem item_cfg : enum_cfg.element_list)
        {
            if (item_cfg.name.length() > max_len)
                max_len = item_cfg.name.length();
        }

        for (EnumConfig::ElementItem item_cfg : enum_cfg.element_list)
        {
            enum_str += ("    " + FixLength(item_cfg.name, max_len) + " = " + QString::number(enum_num, 10) + ",\t-- " + item_cfg.describe + "\n");
            ++enum_num;
        }
        enum_str += "};\n\n";
    }

    // 生成协议类部分
    const QVector<ProtocolConfig> &proto_cfg_list = reader.GetProtocolCfgList();
    for (ProtocolConfig proto_cfg : proto_cfg_list)
    {
        QString init_attr_str;
        QString serialize_str;
        QString unserialize_str;

        int max_len = 0;
        for (ProtocolConfig::AttributeItem item : proto_cfg.attr_list)
        {
            if (item.name.length() > max_len)
                max_len = item.name.length();
        }
        for (ProtocolConfig::AttributeItem item : proto_cfg.attr_list)
        {
            init_attr_str += ("    self." + FixLength(item.name, max_len) + " = " + this->GetTypeInitValue(item.type, item.subtype));
            if (item.describe.length() > 0)
                init_attr_str += ("\t-- " + item.describe);
            init_attr_str += "\n";
            serialize_str += this->GetSerializeStr(item);
            unserialize_str += this->GetUnserializeStr(item);
        }

        QString tmp_str = "";
        tmp_str.sprintf(CLASS_CODE_TEMPLETE,
                        proto_cfg.describe.toStdString().c_str(),
                        proto_cfg.name.toStdString().c_str(),
                        proto_cfg.name.toStdString().c_str(),
                        proto_cfg.name.toStdString().c_str(),
                        proto_cfg.msg_type,
                        init_attr_str.toStdString().c_str(),
                        proto_cfg.name.toStdString().c_str(),
                        serialize_str.toStdString().c_str(),
                        proto_cfg.name.toStdString().c_str(),
                        unserialize_str.toStdString().c_str());
        class_str += tmp_str;
    }

    // 写入文件
    QFile lua_path_file(lua_file_path);
    if (lua_path_file.open(QFile::WriteOnly))
    {
        QTextStream out(&lua_path_file);
        out.setCodec("UTF-8");
        out << HEAD_CODE << enum_str << class_str;
    }
    lua_path_file.close();

    return true;
}

QString LuaGenerator::GetTypeInitValue(const QString &type, const QString &sub_type)
{
    if (P_TYPE_INT8 == type || P_TYPE_UINT8 == type || P_TYPE_INT16 == type || P_TYPE_UINT16 == type ||
        P_TYPE_INT32 == type || P_TYPE_UINT32 == type || P_TYPE_INT64 == type || P_TYPE_UINT64 == type)
    {
        return "0";
    }
    else if (P_TYPE_STRING == type)
    {
        return "\"\"";
    }
    else if (P_TYPE_ARRAY == type)
    {
        return "{}";
    }
    else
    {
        return sub_type + ".New()";
    }
}

QString LuaGenerator::GetWriteType(const QString &type)
{
    QString write_name_str = type;
    if (P_TYPE_INT8 == type)
        write_name_str = "WriteChar";
    else if (P_TYPE_UINT8 == type)
        write_name_str = "WriteUChar";
    else if (P_TYPE_INT16 == type)
        write_name_str = "WriteShort";
    else if (P_TYPE_UINT16 == type)
        write_name_str = "WriteUShort";
    else if (P_TYPE_INT32 == type)
        write_name_str = "WriteInt";
    else if (P_TYPE_UINT32 == type)
        write_name_str = "WriteUInt";
    else if (P_TYPE_INT64 == type)
        write_name_str = "WriteLL";
    else if (P_TYPE_UINT64 == type)
        write_name_str = "WriteLL";
    else if (P_TYPE_STRING == type)
        write_name_str = "WriteString";
    return write_name_str;
}

QString LuaGenerator::GetReadType(const QString &type)
{
    QString read_name_str = type;
    if (P_TYPE_INT8 == type)
        read_name_str = "ReadChar";
    else if (P_TYPE_UINT8 == type)
        read_name_str = "ReadUChar";
    else if (P_TYPE_INT16 == type)
        read_name_str = "ReadShort";
    else if (P_TYPE_UINT16 == type)
        read_name_str = "ReadUShort";
    else if (P_TYPE_INT32 == type)
        read_name_str = "ReadInt";
    else if (P_TYPE_UINT32 == type)
        read_name_str = "ReadUInt";
    else if (P_TYPE_INT64 == type)
        read_name_str = "ReadLL";
    else if (P_TYPE_UINT64 == type)
        read_name_str = "ReadLL";
    else if (P_TYPE_STRING == type)
        read_name_str = "ReadString";
    return read_name_str;
}

QString LuaGenerator::GetSerializeStr(const ProtocolConfig::AttributeItem &item)
{
    if (P_TYPE_INT8 == item.type || P_TYPE_UINT8 == item.type || P_TYPE_INT16 == item.type || P_TYPE_UINT16 == item.type ||
        P_TYPE_INT32 == item.type || P_TYPE_UINT32 == item.type || P_TYPE_INT64 == item.type || P_TYPE_UINT64 == item.type || P_TYPE_STRING == item.type)
    {
        return "    MsgAdapter." + this->GetWriteType(item.type) + "(self." + item.name + ")\n";
    }
    else if (P_TYPE_ARRAY == item.type)
    {
        QString array_code_str;
        array_code_str += "    MsgAdapter.WriteUShort(#self." + item.name + ")\n";
        array_code_str += "    for i = 1, #self." + item.name + " do\n";

        if (P_TYPE_INT8 == item.subtype || P_TYPE_UINT8 == item.subtype || P_TYPE_INT16 == item.subtype || P_TYPE_UINT16 == item.subtype ||
            P_TYPE_INT32 == item.subtype || P_TYPE_UINT32 == item.subtype || P_TYPE_INT64 == item.subtype || P_TYPE_UINT64 == item.subtype || P_TYPE_STRING == item.subtype)
        {
            array_code_str += ("        MsgAdapter." + this->GetWriteType(item.subtype) + "(self." + item.name + "[i])\n");
        }
        else
        {
            array_code_str += ("        self." + item.name + "[i]:Encode()\n");
        }

        array_code_str += "    end\n";

        return array_code_str;
    }

    return "GetSerializeStr_UNKNOW_TYPE";
}

QString LuaGenerator::GetUnserializeStr(const ProtocolConfig::AttributeItem &item)
{
    if (P_TYPE_INT8 == item.type || P_TYPE_UINT8 == item.type || P_TYPE_INT16 == item.type || P_TYPE_UINT16 == item.type ||
        P_TYPE_INT32 == item.type || P_TYPE_UINT32 == item.type || P_TYPE_INT64 == item.type || P_TYPE_UINT64 == item.type || P_TYPE_STRING == item.type)
    {
        return "    self." + item.name + " = MsgAdapter." + this->GetReadType(item.type) + "()\n";
    }
    else if (P_TYPE_ARRAY == item.type)
    {
        QString array_code_str;
        array_code_str += ("    local " + item.name + "_count = MsgAdapter.ReadUShort()\n");
        array_code_str += ("    self." + item.name + " = {}\n");
        array_code_str += ("    for i = 1, " + item.name + "_count do\n");

        if (P_TYPE_INT8 == item.subtype || P_TYPE_UINT8 == item.subtype || P_TYPE_INT16 == item.subtype || P_TYPE_UINT16 == item.subtype ||
            P_TYPE_INT32 == item.subtype || P_TYPE_UINT32 == item.subtype || P_TYPE_INT64 == item.subtype || P_TYPE_UINT64 == item.subtype || P_TYPE_STRING == item.subtype)
        {
            array_code_str += ("        self." + item.name + "[i] = MsgAdapter." + this->GetReadType(item.subtype) + "()\n");
        }
        else
        {
            array_code_str += ("        self." + item.name + "[i] = " + item.subtype + ".New()\n");
            array_code_str += ("        self." + item.name + "[i]:Decode()\n");
        }

        array_code_str += "    end\n";

        return array_code_str;
    }

    return "GetUnserializeStr_UNKNOW_TYPE";
}
