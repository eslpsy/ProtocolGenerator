#include "protocolfilereader.h"
#include "common_tools.hpp"
#include <QtXml>

ProtocolFileReader::ProtocolFileReader()
{

}

ProtocolFileReader::~ProtocolFileReader()
{

}

bool IsBaseType(QString &type_str)
{
    if (P_TYPE_INT8 == type_str ||
            P_TYPE_UINT8 == type_str ||
            P_TYPE_INT16 == type_str ||
            P_TYPE_UINT16 == type_str ||
            P_TYPE_INT32 == type_str ||
            P_TYPE_UINT32 == type_str ||
            P_TYPE_INT64 == type_str ||
            P_TYPE_UINT64 == type_str ||
            P_TYPE_STRING == type_str)
        return true;

    return false;
}

bool ProtocolFileReader::Reload(const QString &xml_file_path)
{
    enum_list_.clear();
    protocol_list_.clear();

    QFile file(xml_file_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        last_error_str_ = "open xml file Error";
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file, &last_error_str_))
    {
        file.close();
        return false;
    }
    // 关闭文件
    file.close();

    // 获得doc的第一个节点，即XML说明
    QDomNode root_node = doc.firstChild();
    QDomElement enum_element = root_node.firstChildElement("enum");
    while (!enum_element.isNull())
    {
        if (enum_element.isElement())
        {
            EnumConfig enum_cfg;
            enum_cfg.name = enum_element.attribute("name");

            QDomElement attr_element = enum_element.firstChildElement("element");
            while (!attr_element.isNull())
            {
                EnumConfig::ElementItem item_cfg;
                item_cfg.name = attr_element.attribute("name");
                item_cfg.describe = attr_element.attribute("describe");
                enum_cfg.element_list.push_back(item_cfg);

                attr_element = attr_element.nextSiblingElement();
            }

            enum_list_.push_back(enum_cfg);
        }

        enum_element = enum_element.nextSiblingElement("enum");
    }

    QDomElement proto_element = root_node.firstChildElement("protocol");
    while (!proto_element.isNull())
    {
        if (proto_element.isElement())
        {
            ProtocolConfig proto_cfg;
            proto_cfg.name = proto_element.attribute("name");
            proto_cfg.msg_type = proto_element.attribute("msg_type").toInt();
            proto_cfg.describe = proto_element.attribute("describe");

            QDomElement attr_element = proto_element.firstChildElement("attr");
            while (!attr_element.isNull())
            {
                ProtocolConfig::AttributeItem item_cfg;
                item_cfg.name = attr_element.attribute("name");
                item_cfg.type = attr_element.attribute("type");
                if ("array" == item_cfg.type) // 数组类型
                {
                    item_cfg.subtype = attr_element.attribute("subtype");
                    if (!IsBaseType(item_cfg.subtype) && proto_name_set_.find(item_cfg.subtype) == proto_name_set_.end())
                    {
                        last_error_str_ = "Unknow Type [" + item_cfg.subtype + "]";
                        return false;
                    }
                }
                item_cfg.describe = attr_element.attribute("describe");
                proto_cfg.attr_list.push_back(item_cfg);

                attr_element = attr_element.nextSiblingElement();
            }

            proto_name_set_.insert(proto_cfg.name);
            protocol_list_.push_back(proto_cfg);
        }
        proto_element = proto_element.nextSiblingElement("protocol");
    }

    return true;
}
