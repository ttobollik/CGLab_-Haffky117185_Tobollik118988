#ifndef POINT_LIGHT_NODE_HPP
#define POINT_LIGHT_NODE_HPP
#include <node.hpp>
#include <model.hpp>


/*
Geometry Node inherits from Node and we can set Geometry (usually obj file)
*/

class PointLightNode : public Node {
	public:
		//default contructor & destructor
		PointLightNode();
		~PointLightNode();
		//constructor using pointer to parent and name
		PointLightNode(std::shared_ptr<Node> const& parent, std::string const& name, float intensity, glm::fvec3 color);

		//getter for PointLightNode
		float getLightIntensity();
		glm::fvec3 getLightColor();  

		//setter for PointLightNode
		void setLightIntensity(float intensity);
		void setLightColor(glm::fvec3 const& color);  

	private:
		//model attribute for PointLightNode
		float LightIntensity_;
		glm::fvec3 LightColor_;


};
#endif