#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"
#include "geometry_node.hpp"
#include "camera_node.hpp"
#include "PointLightNode.hpp"
#include "node.hpp"
#include "scene_graph.hpp"

// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // react to key input
  void keyCallback(int key, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);
  //handle resizing
  void resizeCallback(unsigned width, unsigned height);

  // draw all objects
  void render() const;


  // Function that creates multiple planets (by hand)
  void createPlanetSystem();
  // Function that creates a set number of stars. Parameters are set inside function
  void createRandomStars();
  void createOrbits();

  void drawGraph() const;
  void drawStars() const;
  void drawOrbits()const;


 protected:
  void initializeShaderPrograms();
  void initializeGeometry();
  void initializeTexture();
  void initializeSkybox();
  void initializeFrameBuffer(unsigned width, unsigned height); //necessary for texture bound to framebuffer
  void generateQuadObjects() const;
  // update uniform value
  void uploadUniforms();
  // upload projection matrix
  void uploadProjection();
  // upload view matrix
  void uploadView();

  // cpu representation of model
  model_object planet_object;
  model_object star_object;
  model_object orbit_object;
  model_object skybox_object;

  texture_object texture_framebuffer;
  texture_object renderbuffer_framebuffer;
  texture_object framebuffer;

  
  
  // camera transform matrix
  glm::fmat4 m_view_transform;
  // camera projection matrix
  glm::fmat4 m_view_projection;

  SceneGraph scene_;

};

#endif