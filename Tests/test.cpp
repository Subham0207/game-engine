#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "3DModel/model.hpp"
#include <serializeAClass.hpp>

class TestSerializeMacro{
public:
    std::string name;

};

TEST(ExampleTest, Test1){
    auto testSerializationMacro =  new TestSerializeMacro();
}

TEST(Character, OnPlayEndCharacterShouldBeBackToTPose){
    auto testSerializationMacro =  new TestSerializeMacro();
}

TEST(ProjectManager, ShouldCreateNewProject){
    auto testSerializationMacro =  new TestSerializeMacro();
}

TEST(FileExplorer, ShouldImportModelWithOrSkeletonData){
    auto testSerializationMacro =  new TestSerializeMacro();
}