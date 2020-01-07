#include <PointLightNode.hpp>

PointLightNode::PointLightNode():
	Node(),
	LightIntensity_{0.5f},
	LightColor_{0.2f, 0.2f, 0.7f} {}


PointLightNode::~PointLightNode(){}

PointLightNode::PointLightNode(std::shared_ptr<Node> const& parent, std::string const& name, float intensity, glm::fvec3 color):
	Node(parent, name),
	LightIntensity_{intensity},
	LightColor_{color} {}


float PointLightNode::getLightIntensity(){
	return LightIntensity_;
}

glm::fvec3 PointLightNode::getLightColor(){
	return LightColor_;
} 


void PointLightNode::setLightIntensity(float intensity){
	LightIntensity_ = intensity;
}

void PointLightNode::setLightColor(glm::fvec3 const& color){
	LightColor_ = color;
} 