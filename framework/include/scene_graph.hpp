#ifndef SCENEGRAPH_HPP
#define SCENEGRAPH_HPP

#include <iostream>
#include "node.hpp"
#include "geometry_node.hpp"
#include "PointLightNode.hpp"
#include <memory>


/*
Scenegraph which saves a node. This is the root node of the graph. From there, shared pointer to children are saved.
*/

class SceneGraph {
public:
	//static SceneGraph* getInstance(std::string name, std::shared_ptr<Node> root);

	//constructor
	SceneGraph(std::string name, std::shared_ptr<Node> root);
	//destructor
	~SceneGraph();
	//getter
	std::string const& getName();
	std::shared_ptr<Node> const& getRoot();

	void printGraph();
	std::vector<std::shared_ptr<GeometryNode>> geometry_nodes_;
	std::vector<std::shared_ptr<PointLightNode>> point_light_nodes_;

private:
	//setter are private because of Singleton
	void setName(std::string const& name);
	void setRoot(std::shared_ptr<Node> const& root); //root node of scene graph
	
	void graphPrinter(std::shared_ptr<Node> const& node, int depth);

	//static SceneGraph* scene_graph_instance_;
	std::shared_ptr<Node> root_;
	std::string name_;
	
};

#endif