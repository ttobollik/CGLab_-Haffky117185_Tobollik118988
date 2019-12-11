#ifndef CAMERA_NODE_HPP
#define CAMERA_NODE_HPP
#include <glm/glm.hpp>
#include <node.hpp>


/*
Camera Node which inherits from Node. Will be needed fro View-Projection-Matrix
*/

class CameraNode : Node {
	public:
		CameraNode();
		CameraNode(std::shared_ptr<Node> const& parent, std::string const& name,
	bool isPerspective, bool isEnabled, glm::mat4x4 projectionMatrix);
		~CameraNode();
		bool getPerspective();
		bool getEnabled();
		void setEnabled(bool enable);
		glm::mat4 getProjectionMatrx();
		void setProjectionMatrix(glm::mat4 const& projectionMatrix);

	private:
		bool isPerspective_;
		bool isEnabled_;
		glm::mat4 projectionMatrix_;
};

#endif