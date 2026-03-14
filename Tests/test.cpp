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

TEST(AssetBrowser, ShouldPassCorrectAssetGuidToCharacterObjectThatGotSpawnedIntoLevel){
    auto testSerializationMacro =  new TestSerializeMacro();
}

TEST(ProjectManifest, SavedFileShouldBeInCorrectFormat)
{
    //Verify Lvl file is guidId.meta.json
    //engineDir is the path of the executable
    //projectDir is ../../
    //entryLevel is just the level name since we can figure out from mounts dict where is Levels folder.
    auto testSerializationMacro =  new TestSerializeMacro();
}

TEST(ProjectManifest, ShouldConvertProjectDirToActualPathWhenFileLoaded)
{
    //Check if ProjectDir is converted from ../../ to actual path. So path chains don't look confusing.
    auto testSerializationMacro =  new TestSerializeMacro();
}