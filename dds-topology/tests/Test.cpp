// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// DDS
#include "Index.h"
#include "Topology.h"
#include "TopologyParserXML.h"
#include "TopoBase.h"
#include "TopoElement.h"
#include "TopoProperty.h"
#include "Task.h"
#include "TaskCollection.h"
#include "TaskGroup.h"
#include "TopoUtils.h"
#include "TopoFactory.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

BOOST_AUTO_TEST_SUITE(test_dds_topology)

BOOST_AUTO_TEST_CASE(test_dds_topology_1)
{
    CTopology topology;
    topology.init("topology_test_1.xml");

    TopoElementPtr_t e1 = topology.getTopoElementByIndex(CIndex("main/task1"));
    BOOST_CHECK(e1->getPath() == "main/task1");

    TopoElementPtr_t e2 = topology.getTopoElementByIndex(CIndex("main/collection1"));
    BOOST_CHECK(e2->getPath() == "main/collection1");

    TopoElementPtr_t e3 = topology.getTopoElementByIndex(CIndex("main/group1"));
    BOOST_CHECK(e3->getPath() == "main/group1");

    TopoElementPtr_t e4 = topology.getTopoElementByIndex(CIndex("main/group1/collection1"));
    BOOST_CHECK(e4->getPath() == "main/group1/collection1");

    TopoElementPtr_t e5 = topology.getTopoElementByIndex(CIndex("main/group2/collection2/task5"));
    BOOST_CHECK(e5->getPath() == "main/group2/collection2/task5");

    BOOST_CHECK_THROW(topology.getTopoElementByIndex(CIndex("wrong_path")), runtime_error);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_1)
{
    CTopology topology;
    topology.init("topology_test_1.xml");
    TaskGroupPtr_t main = topology.getMainGroup();

    //  CTopologyParserXML parser;
    //  TaskGroupPtr_t main = make_shared<CTaskGroup>();
    //  parser.parse("topology_test_1.xml", main);

    BOOST_CHECK(main != nullptr);

    BOOST_CHECK(main->getNofTasks() == 22);
    BOOST_CHECK(main->getTotalNofTasks() == 220);
    BOOST_CHECK(main->getN() == 1);
    BOOST_CHECK(main->getName() == "main");
    BOOST_CHECK(main->getNofElements() == 4);
    BOOST_CHECK(main->getParent() == nullptr);
    BOOST_CHECK(main->getPath() == "main");
    BOOST_CHECK_THROW(main->getElement(4), std::out_of_range);

    TopoElementPtr_t element1 = main->getElement(0);
    BOOST_CHECK(element1->getName() == "task1");
    BOOST_CHECK(element1->getType() == ETopoType::TASK);
    BOOST_CHECK(element1->getParent() == main.get());
    BOOST_CHECK(element1->getPath() == "main/task1");
    BOOST_CHECK(element1->getNofTasks() == 1);
    BOOST_CHECK(element1->getTotalNofTasks() == 1);
    TaskPtr_t casted1 = dynamic_pointer_cast<CTask>(element1);
    BOOST_CHECK(casted1->getNofPorts() == 2);
    BOOST_CHECK(casted1->getPort(0)->getPortType() == EPortType::SERVER);
    BOOST_CHECK(casted1->getPort(1)->getPortType() == EPortType::CLIENT);
    BOOST_CHECK(casted1->getExec() == "app1");
    BOOST_CHECK(casted1->getEnv() == "env1");
    BOOST_CHECK(casted1->getArgs() == "-a -b -c");

    TopoElementPtr_t element2 = main->getElement(1);
    BOOST_CHECK(element2->getName() == "collection1");
    BOOST_CHECK(element2->getType() == ETopoType::COLLECTION);
    BOOST_CHECK(element2->getParent() == main.get());
    BOOST_CHECK(element2->getPath() == "main/collection1");
    BOOST_CHECK(element2->getNofTasks() == 4);
    BOOST_CHECK(element2->getTotalNofTasks() == 4);
    TaskCollectionPtr_t casted2 = dynamic_pointer_cast<CTaskCollection>(element2);
    BOOST_CHECK(casted2->getNofElements() == 4);
    BOOST_CHECK_THROW(casted2->getElement(4), std::out_of_range);

    const auto& casted2_elements = casted2->getElements();
    for (const auto& v : casted2_elements)
    {
        BOOST_CHECK(v->getParent() == element2.get());
    }

    TopoElementPtr_t element3 = main->getElement(2);
    BOOST_CHECK(element3->getName() == "group1");
    BOOST_CHECK(element3->getType() == ETopoType::GROUP);
    BOOST_CHECK(element3->getParent() == main.get());
    BOOST_CHECK(element3->getPath() == "main/group1");
    BOOST_CHECK(element3->getNofTasks() == 8);
    BOOST_CHECK(element3->getTotalNofTasks() == 80);
    TaskGroupPtr_t casted3 = dynamic_pointer_cast<CTaskGroup>(element3);
    BOOST_CHECK(casted3->getNofElements() == 3);
    BOOST_CHECK_THROW(casted3->getElement(3), std::out_of_range);
    BOOST_CHECK(casted3->getN() == 10);

    TopoElementPtr_t element4 = main->getElement(3);
    BOOST_CHECK(element4->getName() == "group2");
    BOOST_CHECK(element4->getType() == ETopoType::GROUP);
    BOOST_CHECK(element4->getParent() == main.get());
    BOOST_CHECK(element4->getPath() == "main/group2");
    BOOST_CHECK(element4->getNofTasks() == 9);
    BOOST_CHECK(element4->getTotalNofTasks() == 135);
    TaskGroupPtr_t casted4 = dynamic_pointer_cast<CTaskGroup>(element4);
    BOOST_CHECK(casted4->getNofElements() == 4);
    BOOST_CHECK_THROW(casted4->getElement(4), std::out_of_range);
    BOOST_CHECK(casted4->getN() == 15);

    TaskCollectionPtr_t casted5 = dynamic_pointer_cast<CTaskCollection>(casted4->getElement(2));
    BOOST_CHECK(casted5->getName() == "collection1");
    BOOST_CHECK(casted5->getType() == ETopoType::COLLECTION);
    BOOST_CHECK(casted5->getParent() == casted4.get());
    BOOST_CHECK(casted5->getPath() == "main/group2/collection1");
    BOOST_CHECK(casted5->getNofTasks() == 4);
    BOOST_CHECK(casted5->getTotalNofTasks() == 4);
    BOOST_CHECK(casted5->getNofElements() == 4);
    BOOST_CHECK_THROW(casted5->getElement(4), std::out_of_range);
    BOOST_CHECK(casted5->getTotalCounter() == 15);

    TaskPtr_t casted6 = dynamic_pointer_cast<CTask>(casted5->getElement(0));
    BOOST_CHECK(casted6->getName() == "task1");
    BOOST_CHECK(casted6->getType() == ETopoType::TASK);
    BOOST_CHECK(casted6->getParent() == casted5.get());
    BOOST_CHECK(casted6->getPath() == "main/group2/collection1/task1");
    BOOST_CHECK(casted6->getNofTasks() == 1);
    BOOST_CHECK(casted6->getTotalNofTasks() == 1);
    BOOST_CHECK(casted6->getNofPorts() == 2);
    BOOST_CHECK(casted6->getExec() == "app1");

    // Test getElementsByType and getTotalCounter
    TopoElementPtrVector_t elements1 = main->getElementsByType(ETopoType::TASK);
    BOOST_CHECK(elements1.size() == 4);
    for (const auto& v : elements1)
    {
        BOOST_CHECK(v->getType() == ETopoType::TASK);
    }
    TaskPtr_t castedTask = dynamic_pointer_cast<CTask>(elements1[0]);
    BOOST_CHECK(castedTask->getName() == "task1");
    BOOST_CHECK(castedTask->getPath() == "main/task1");
    BOOST_CHECK(castedTask->getTotalCounter() == 1);
    castedTask = dynamic_pointer_cast<CTask>(elements1[1]);
    BOOST_CHECK(castedTask->getName() == "task1");
    BOOST_CHECK(castedTask->getPath() == "main/group1/task1");
    BOOST_CHECK(castedTask->getTotalCounter() == 10);
    castedTask = dynamic_pointer_cast<CTask>(elements1[2]);
    BOOST_CHECK(castedTask->getName() == "task3");
    BOOST_CHECK(castedTask->getPath() == "main/group2/task3");
    BOOST_CHECK(castedTask->getTotalCounter() == 15);
    castedTask = dynamic_pointer_cast<CTask>(elements1[3]);
    BOOST_CHECK(castedTask->getName() == "task4");
    BOOST_CHECK(castedTask->getPath() == "main/group2/task4");
    BOOST_CHECK(castedTask->getTotalCounter() == 15);

    TopoElementPtrVector_t elements2 = main->getElementsByType(ETopoType::COLLECTION);
    BOOST_CHECK(elements2.size() == 5);
    for (const auto& v : elements2)
    {
        BOOST_CHECK(v->getType() == ETopoType::COLLECTION);
    }
    TaskCollectionPtr_t castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[0]);
    BOOST_CHECK(castedCollection->getName() == "collection1");
    BOOST_CHECK(castedCollection->getPath() == "main/collection1");
    BOOST_CHECK(castedCollection->getTotalCounter() == 1);
    castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[1]);
    BOOST_CHECK(castedCollection->getName() == "collection1");
    BOOST_CHECK(castedCollection->getPath() == "main/group1/collection1");
    BOOST_CHECK(castedCollection->getTotalCounter() == 10);
    castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[2]);
    BOOST_CHECK(castedCollection->getName() == "collection2");
    BOOST_CHECK(castedCollection->getPath() == "main/group1/collection2");
    BOOST_CHECK(castedCollection->getTotalCounter() == 10);
    castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[3]);
    BOOST_CHECK(castedCollection->getName() == "collection1");
    BOOST_CHECK(castedCollection->getPath() == "main/group2/collection1");
    BOOST_CHECK(castedCollection->getTotalCounter() == 15);
    castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[4]);
    BOOST_CHECK(castedCollection->getName() == "collection2");
    BOOST_CHECK(castedCollection->getPath() == "main/group2/collection2");
    BOOST_CHECK(castedCollection->getTotalCounter() == 15);

    /// test getIndicesByType
    IndexVector_t ids1 = main->getIndicesByType(ETopoType::COLLECTION);
    BOOST_CHECK(ids1.size() == 5);
    BOOST_CHECK(ids1[0].getPath() == "main/collection1");
    BOOST_CHECK(ids1[1].getPath() == "main/group1/collection1");
    BOOST_CHECK(ids1[2].getPath() == "main/group1/collection2");
    BOOST_CHECK(ids1[3].getPath() == "main/group2/collection1");
    BOOST_CHECK(ids1[4].getPath() == "main/group2/collection2");

    IndexVector_t ids2 = main->getIndicesByType(ETopoType::TASK);
    BOOST_CHECK(ids2.size() == 4);
    BOOST_CHECK(ids2[0].getPath() == "main/task1");
    BOOST_CHECK(ids2[1].getPath() == "main/group1/task1");
    BOOST_CHECK(ids2[2].getPath() == "main/group2/task3");
    BOOST_CHECK(ids2[3].getPath() == "main/group2/task4");
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_1)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_1.xml");
    BOOST_CHECK(result == true);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_2)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_2.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_3)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_3.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_4)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_4.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_5)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_5.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_6)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_6.xml");
    BOOST_CHECK(result == true);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_wrong)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("wrong_file.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_factory)
{
    // DDSCreateTopoBase
    BOOST_CHECK_THROW(CreateTopoBase(ETopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(CreateTopoBase(ETopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(CreateTopoBase(ETopoType::TOPO_PROPERTY), runtime_error);
    TopoBasePtr_t baseTask = CreateTopoBase(ETopoType::TASK);
    BOOST_CHECK(baseTask != nullptr);
    TopoBasePtr_t baseCollection = CreateTopoBase(ETopoType::COLLECTION);
    BOOST_CHECK(baseCollection != nullptr);
    TopoBasePtr_t baseGroup = CreateTopoBase(ETopoType::GROUP);
    BOOST_CHECK(baseGroup != nullptr);
    TopoBasePtr_t basePort = CreateTopoBase(ETopoType::PORT);
    BOOST_CHECK(basePort != nullptr);

    // DDSCreateTopoElement
    BOOST_CHECK_THROW(CreateTopoElement(ETopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(CreateTopoElement(ETopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(CreateTopoElement(ETopoType::TOPO_PROPERTY), runtime_error);
    TopoElementPtr_t elementTask = CreateTopoElement(ETopoType::TASK);
    BOOST_CHECK(elementTask != nullptr);
    TopoElementPtr_t elementCollection = CreateTopoElement(ETopoType::COLLECTION);
    BOOST_CHECK(elementCollection != nullptr);
    TopoElementPtr_t elementGroup = CreateTopoElement(ETopoType::GROUP);
    BOOST_CHECK(elementGroup != nullptr);
    BOOST_CHECK_THROW(CreateTopoElement(ETopoType::PORT), runtime_error);

    // DDSCreateTopoProperty
    BOOST_CHECK_THROW(CreateTopoProperty(ETopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(CreateTopoProperty(ETopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(CreateTopoProperty(ETopoType::TOPO_PROPERTY), runtime_error);
    BOOST_CHECK_THROW(CreateTopoProperty(ETopoType::TASK), runtime_error);
    BOOST_CHECK_THROW(CreateTopoProperty(ETopoType::COLLECTION), runtime_error);
    BOOST_CHECK_THROW(CreateTopoProperty(ETopoType::GROUP), runtime_error);
    TopoBasePtr_t propertyPort = CreateTopoProperty(ETopoType::PORT);
    BOOST_CHECK(propertyPort != nullptr);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_utils)
{
    // DDSTopoTypeToTag
    BOOST_CHECK_THROW(TopoTypeToTag(ETopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(TopoTypeToTag(ETopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(TopoTypeToTag(ETopoType::TOPO_PROPERTY), runtime_error);
    BOOST_CHECK(TopoTypeToTag(ETopoType::TASK) == "task");
    BOOST_CHECK(TopoTypeToTag(ETopoType::COLLECTION) == "collection");
    BOOST_CHECK(TopoTypeToTag(ETopoType::GROUP) == "group");
    BOOST_CHECK(TopoTypeToTag(ETopoType::PORT) == "port");

    // DDSCreateTopoElement
    BOOST_CHECK_THROW(TagToTopoType(""), runtime_error);
    BOOST_CHECK_THROW(TagToTopoType("topobase"), runtime_error);
    BOOST_CHECK_THROW(TagToTopoType("topoelement"), runtime_error);
    BOOST_CHECK_THROW(TagToTopoType("topoproperty"), runtime_error);
    BOOST_CHECK(TagToTopoType("task") == ETopoType::TASK);
    BOOST_CHECK(TagToTopoType("collection") == ETopoType::COLLECTION);
    BOOST_CHECK(TagToTopoType("group") == ETopoType::GROUP);
    BOOST_CHECK(TagToTopoType("port") == ETopoType::PORT);

    // DDSStringToPortType
    BOOST_CHECK_THROW(StringToPortType("blablabla"), runtime_error);
    BOOST_CHECK(StringToPortType("server") == EPortType::SERVER);
    BOOST_CHECK(StringToPortType("client") == EPortType::CLIENT);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_base_find_element)
{
    ptree pt;
    read_xml("topology_test_6.xml", pt);

    const ptree& pt1 = CTopoBase::findElement(ETopoType::TASK, "task1", pt.get_child("topology"));
    BOOST_CHECK(pt1.get<string>("<xmlattr>.name") == "task1");

    const ptree& pt2 = CTopoBase::findElement(ETopoType::COLLECTION, "collection1", pt.get_child("topology"));
    BOOST_CHECK(pt2.get<string>("<xmlattr>.name") == "collection1");

    const ptree& pt3 = CTopoBase::findElement(ETopoType::GROUP, "group1", pt.get_child("topology.main"));
    BOOST_CHECK(pt3.get<string>("<xmlattr>.name") == "group1");

    const ptree& pt4 = CTopoBase::findElement(ETopoType::PORT, "port1", pt.get_child("topology"));
    BOOST_CHECK(pt4.get<string>("<xmlattr>.name") == "port1");

    // Wrong path to property tree
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::TASK, "task1", pt), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::GROUP, "group1", pt.get_child("topology")), logic_error);

    // Wrong element names
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::TASK, "NO", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::COLLECTION, "NO", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::GROUP, "NO", pt.get_child("topology.main")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::PORT, "NO", pt.get_child("topology")), logic_error);

    // Dublicated names
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::TASK, "task2", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::COLLECTION, "collection2", pt.get_child("topology")),
                      logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::GROUP, "group2", pt.get_child("topology.main")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::PORT, "port3", pt.get_child("topology")), logic_error);
}

BOOST_AUTO_TEST_SUITE_END()
