#include <scene_graph.hpp>
#include <node.hpp>

/**
SceneGraph* SceneGraph::getInstance(std::string name, std::shared_ptr<Node> root) {
	if (!scene_graph_instance_) {
		scene_graph_instance_ = new SceneGraph(name, root);
	}
	return scene_graph_instance_;
}
**/
SceneGraph::SceneGraph(){};

//constructor
SceneGraph::SceneGraph(std::string name, std::shared_ptr<Node> root): 
	name_{name},
	root_{root} {}

//destructor
SceneGraph::~SceneGraph(){}

//returns name as a string
std::string const& SceneGraph::getName() {
	return name_;
}

// returns pointer to root node
std::shared_ptr<Node> const& SceneGraph::getRoot() {
	return root_;
}

void SceneGraph::setName(std::string const& name){
	name_ = name;
}

void SceneGraph::setRoot(std::shared_ptr<Node> const& root){
	root_ = root;
}

//prints graph starting at the root
void SceneGraph::printGraph(){

	std::shared_ptr<Node> root = getRoot();
	int depth = root->getDepth();

	//using a recusive function for traversing graph and print it
	graphPrinter(root, depth);
}


//helper function for recursion. Could be avoided if we add a queue and to iteratively
void SceneGraph::graphPrinter(std::shared_ptr<Node> const& node, int depth) {
	//creating emptyspace, depending on depth of node
	for (int index = 0; index < depth; ++index) {
		std::cout<<" ";
	}

	std::cout<<node->getName()<<std::endl;
	for (std::shared_ptr<Node> const& child : node->getChildrenList()) {
		graphPrinter(child, depth+1);
	}

}