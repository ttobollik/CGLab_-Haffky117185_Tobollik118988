#include <geometry_node.hpp>

GeometryNode::GeometryNode(){}

GeometryNode::GeometryNode(std::shared_ptr<Node> const& parent, std::string const& name):
	Node(parent, name) {}


GeometryNode::GeometryNode(std::shared_ptr<Node> parent, std::string const& name, float size, float speed,
		glm::fvec3 const& position, model const& model) :
		Node(parent, name, size, speed, position) { setGeometry(model); }
	

model GeometryNode::getGeometry(){
	return geometry_;
}

void GeometryNode::setGeometry(model const& model){
	geometry_ = model;
}