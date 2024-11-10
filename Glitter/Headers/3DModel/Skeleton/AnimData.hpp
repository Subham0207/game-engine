#pragma once

#include"../../../Vendor/glm/glm/glm.hpp"
#include <serializeAClass.hpp>

struct BoneInfo
{
	/*id is index in finalBoneMatrices*/
	int id;

	/*offset matrix transforms vertex from model space to bone space*/
	glm::mat4 offset;

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & id;
        ar & offset;
    }
};
#pragma once