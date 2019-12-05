#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"
#include "geometry_node.hpp"
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
  SceneGraph createPlanetSystem() const;
  // Function that rekursively traverses the graph and sends objects to GPU -> is called by createPlanetSystem
  // (maybe implement iterativle through queue, to reduce time?)
  void drawGraph(SceneGraph scene) const;

  void createRandomStars();
  void drawStars() const;


 protected:
  void initializeShaderPrograms();
  void initializeGeometry();
  // update uniform values
  void uploadUniforms();
  // upload projection matrix
  void uploadProjection();
  // upload view matrix
  void uploadView();

  // cpu representation of model
  model_object planet_object;
  model_object star_object;
  model_object orbit_object;
  
  // camera transform matrix
  glm::fmat4 m_view_transform;
  // camera projection matrix
  glm::fmat4 m_view_projection;

};

#endif