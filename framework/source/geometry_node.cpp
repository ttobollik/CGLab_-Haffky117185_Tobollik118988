#include <geometry_node.hpp>

//destructor
GeometryNode::GeometryNode(){}

//constructor for geometry node using constructor of node
GeometryNode::GeometryNode(std::shared_ptr<Node> const& parent, std::string const& name, glm::fvec3 color):
	Node(parent, name), 
	color_{color},
	texture_path_{"neptunemap.png"} {}

//constructor for geometry node using constructor of node and adding model
GeometryNode::GeometryNode(std::shared_ptr<Node> parent, std::string const& name, float size, float speed,
		glm::fvec3 const& position, model const& model) :
		Node(parent, name, size, speed, position) { setGeometry(model); }
	

model GeometryNode::getGeometry(){
	return geometry_;
}

glm::fvec3 GeometryNode::getColor() {
	return color_;
}

void GeometryNode::setGeometry(model const& model){
	geometry_ = model;
}

void GeometryNode::set_texture_path(std::string const& path) {
	texture_path_ = path;
}

std::string GeometryNode::get_texture_path(){
	return texture_path_;
}

void GeometryNode::set_texture_obj(texture_object tex_obj){
	texture_obj_ = tex_obj;
}


texture_object GeometryNode::get_texture_obj(){
	return texture_obj_;
}	