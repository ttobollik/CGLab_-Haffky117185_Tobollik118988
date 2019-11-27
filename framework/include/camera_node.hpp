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