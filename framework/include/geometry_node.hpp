#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP
#include <node.hpp>
#include <model.hpp>
#include <structs.hpp>

/*
Geometry Node inherits from Node and we can set Geometry (usually obj file)
*/

class GeometryNode : public Node {
	public:
		//default contructor
		GeometryNode();
		//constructor using pointer to parent and name
		GeometryNode(std::shared_ptr<Node> const& parent, std::string const& name, glm::fvec3 color);
		//constructor using pointer to parent, name, size, speed, position and a model
		GeometryNode(std::shared_ptr<Node> parent, std::string const& name, float size, float speed,
		glm::fvec3 const& position, model const& model);

		//getter for geomentry node
		model getGeometry();
		glm::fvec3 getColor();
		//setter for geometry node using a model
		void setGeometry(model const& model);
		void set_texture_path(std::string const& path);
		std::string get_texture_path();

		void set_texture_obj(texture_object tex_obj);
		texture_object get_texture_obj();

	private:
		//model attribute for gemometry node
		model geometry_;
		glm::fvec3 color_;
		std::string texture_path_;
		texture_object texture_obj_;

};
#endif