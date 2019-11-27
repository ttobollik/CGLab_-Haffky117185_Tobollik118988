#ifndef NODE_HPP
#define NODE_HPP
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <memory>

class Node {
public:
	//constructors
	//default
	Node();
	//using pointer to parent and name
	Node(std::shared_ptr<Node> parent, std::string const& name);
	//using pointer to parent, name, size, speed and position
	Node(std::shared_ptr<Node> parent, std::string const& name, float size, float speed,
		glm::fvec3 const& position);

	//getter and setter
	//gives nack smart pointer to parent
	std::shared_ptr<Node> getParent() const;
	// sets parent node using a smart pointer (object gets destroyed if it has no pointer attached to it)
	void setParent(std::shared_ptr<Node> const& parent);
	//gives back a vector of pointers to all children of the current node
	std::vector<std::shared_ptr<Node>> getChildrenList() const;
	//gives back pointer to a specific child by comparing the name attribute
	std::shared_ptr<Node> const& getChild(std::string name);

	std::string getName() const; //name of node
	std::string getPath() const; //path, where the node is
	int getDepth() const; //depth, where the node is on the graph
	glm::mat4 getLocalTransform() const;
	glm::mat4 getWorldTransform() const;
	glm::fvec3 getPosition() const;
	float getSpeed() const; 

	void setLocalTransform(glm::mat4 const& transform);
	void setWorldTransform(glm::mat4 const& transform);
	void addChild(std::shared_ptr<Node> const& node);
	std::shared_ptr<Node> removeChild(std::string const& node);
	bool hasChild(std::string const& name);
	void setPosition(glm::fvec3 const& position);
	void setSpeed(float speed);

	virtual std::ostream& print(std::ostream& os) const;
	
protected:
	std::shared_ptr<Node> parent_;
	std::vector<std::shared_ptr<Node>> children_;
	std::string name_;
	int depth_;
	std::string path_;
	glm::mat4 localTransform_;
	glm::mat4 worldTransform_{glm::mat4(1.0f)};
	glm::fvec3 position_;
	float speed_;

};


std::ostream& operator<<(std::ostream& os, Node const& n);
#endif