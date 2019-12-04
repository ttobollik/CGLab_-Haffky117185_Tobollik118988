#include <node.hpp>
#include <glm/glm.hpp>
#include <iostream>

//consructors
//default
Node::Node() : 
	parent_{},
	children_{},
	name_{},
	depth_{},
	path_{},
	localTransform_{},
	worldTransform_{},
	speed_{} {}

Node::Node(std::shared_ptr<Node> parent, std::string const& name) : 
	parent_{parent},
	children_{},
	name_{name},
	depth_{},
	path_{},
	localTransform_{1.0f},
	worldTransform_{1.0f},
	speed_{1.0f} {
		//if the node has a parent
		if (parent != nullptr) {
			depth_ = parent_->getDepth() + 1; //calculates depth for new node
			path_ = parent_->getPath() +"/"+name; //creates new path by adding new node to parent path
			}
		else {
			path_ = "/";
			depth_ = 0;
		}

	}

Node::Node(std::shared_ptr<Node> parent, std::string const& name, float size, float speed,
	glm::fvec3 const& position) : 
	parent_{parent},
	children_{},
	name_{name},
	depth_{},
	path_{},
	localTransform_{glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                     0.0f, 1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f, 
                     0.0f, 0.0f, 0.0f, size}},
	worldTransform_{1.0f},
	speed_{speed} {
		//if the node has a parent
		if (parent != nullptr) {
			depth_ = parent_->getDepth() + 1; //calculates depth for new node
			path_ = parent_->getPath() +"/"+name; //creates new path by adding new node to parent path
			}
		else {
			path_ = "/";
			depth_ = 0;
		}

	}



std::shared_ptr<Node> Node::getParent() const{
	return parent_;
}

void Node::setParent(std::shared_ptr<Node> const& parent){
	parent_ = parent;
}

//returns vector of pointers to all children
std::vector<std::shared_ptr<Node>> Node::getChildrenList() const{
	return children_;
}

//returns pointer to specific child??
std::shared_ptr<Node> const& Node::getChild(std::string name){
	for (auto const& child : getChildrenList()) {
		if (child->getName() == name){
			return child;
		}
	}
	return nullptr;
}

std::string Node::getName() const{
	return name_;
}

std::string Node::getPath() const{
	return path_;
}

int Node::getDepth() const{
	return depth_;
}

glm::mat4 Node::getLocalTransform() const{
	return localTransform_;
}

void Node::setLocalTransform(glm::mat4 const& transform){
	localTransform_ = transform;
}

glm::mat4 Node::getWorldTransform() const{
	if (parent_ != nullptr) {
		return parent_->getWorldTransform()*localTransform_;
	} else {
		return localTransform_;
	}
}

void Node::setWorldTransform(glm::mat4 const& transform){
	worldTransform_ = transform;
}

//adding more children? addChild?
void Node::addChild(std::shared_ptr<Node> const& node){
	children_.push_back(node);
}

//removes child node from children list using it's name to find it
std::shared_ptr<Node> Node::removeChild(std::string const& name){
	int i = 0;
	for (auto const& child : getChildrenList()) {
		if (child->getName() == name){
			return child;
			children_.erase(children_.begin()+i);
		}
		++i;
	}
	return nullptr;
}

// looks for a specific child using it's name
bool Node::hasChild(std::string const& name){
	for (auto const& child : getChildrenList()) {
		if (child->getName() == name){
			return true;
		}
	}
	return false;
}



void Node::setSpeed(float speed){
	speed_ = speed;
}


float Node::getSpeed() const{
	return speed_;

}

//prints the attributes of the node except speed and position
std::ostream& Node::print(std::ostream& os) const{
    os << "name: " << name_ << "\n"
    << "path: " << path_ << "\n"
    << "Parent: " << parent_ -> getName() << "\n" 
    << "children: ";
    for(auto const& i: children_){
        os << i -> name_ <<", ";
    } 
    os << "\n"
    << "depth: " << depth_ << "\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, Node const& n){
    return n.print(os);
}
