#include <camera_node.hpp>

CameraNode::CameraNode():
	Node(),
	isPerspective_{},
	isEnabled_{},
	projectionMatrix_{} {}

CameraNode::CameraNode(std::shared_ptr<Node> const& parent, std::string const& name,
	bool isPerspective, bool isEnabled, glm::mat4x4 projectionMatrix):
	Node(parent, name),
	isPerspective_{isPerspective},
	isEnabled_{isEnabled},
	projectionMatrix_{projectionMatrix} {}

CameraNode::~CameraNode(){}

bool CameraNode::getPerspective(){
	return isPerspective_;
}

bool CameraNode::getEnabled(){
	return isEnabled_;
}

void CameraNode::setEnabled(bool enable){
	isEnabled_ = enable;
}

glm::mat4 CameraNode::getProjectionMatrx(){
	return projectionMatrix_;
}

void CameraNode::setProjectionMatrix(glm::mat4 const& projectionMatrix) {
	projectionMatrix_ = projectionMatrix;
}