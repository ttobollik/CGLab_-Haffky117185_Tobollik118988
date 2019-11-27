#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP
#include <node.hpp>
#include <model.hpp>

class GeometryNode : public Node {
	public:
		//default contructor
		GeometryNode();
		//constructor using pointer to parent and name
		GeometryNode(std::shared_ptr<Node> const& parent, std::string const& name);
		//constructor using pointer to parent, name, size, speed, position and a model
		GeometryNode(std::shared_ptr<Node> parent, std::string const& name, float size, float speed,
		glm::fvec3 const& position, model const& model);

		//getter for geomentry node
		model getGeometry();
		//setter for geometry node using a model
		void setGeometry(model const& model);

	private:
		//model attribute for gemometry node
		model geometry_;

};
#endif