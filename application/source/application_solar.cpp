#include "application_solar.hpp"
#include "window_handler.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "scene_graph.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <queue>

#include <iostream>


//constructor
ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,star_object{}
 ,orbit_object{}
 ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})} //camera position
 ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
{
  createRandomStars();
  initializeGeometry();
  initializeShaderPrograms();
  createOrbits();
}

//destructor
ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);

  glDeleteBuffers(1, &orbit_object.vertex_BO);
  glDeleteVertexArrays(1, &orbit_object.vertex_AO);
}

void ApplicationSolar::render() const {
  SceneGraph scene = createPlanetSystem();


  //calling the function to create planets

  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  drawGraph(scene);

  glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
  drawOrbits(scene);

  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  drawStars();

}

void ApplicationSolar::drawStars() const{

  glUseProgram(m_shaders.at("star").handle);

  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ModelViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_transform));
  
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glBindVertexArray(star_object.vertex_AO);

  glDrawArrays(star_object.draw_mode, GLint(0), star_object.num_elements);
}


SceneGraph ApplicationSolar::createPlanetSystem() const{

  // we load a circular model from the resources
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

  // creating an empty root node with nullptr as parent, named /, path / and 0 depth
  Node root{nullptr, "/"};
  auto root_pointer = std::make_shared<Node>(root);
  SceneGraph scene = SceneGraph("scene", root_pointer);

  //pointLightNode
  PointLightNode sun_light{root_pointer, "sunlight", 0.8f, {0.5f, 0.5f, 0.0f}};
  auto sunlight_pointer = std::make_shared<PointLightNode>(sun_light);
  root_pointer->addChild(sunlight_pointer);

  //sun
  GeometryNode sun{root_pointer, "sun", {0.5f, 0.2f, 0.2f}}; 
  auto sun_pointer = std::make_shared<GeometryNode>(sun);
  sun_pointer->setGeometry(planet_model);
  sun_pointer->setLocalTransform(glm::scale(sun_pointer->getLocalTransform(), glm::fvec3{2.0f})*
                                    (glm::translate(sun_pointer->getLocalTransform(), glm::fvec3{0.0f, 0.0f, 0.0f})));
  root_pointer->addChild(sun_pointer);
  scene.geometry_nodes_.push_back(sun_pointer);

  //holder node for merkury
  Node merkury_holder{root_pointer, "merkury_holder"};
  auto merkury_holder_pointer = std::make_shared<Node>(merkury_holder);
  merkury_holder_pointer->setLocalTransform(glm::rotate(merkury_holder_pointer->getLocalTransform(), float(glfwGetTime())* 1.2f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(merkury_holder_pointer);

  //merkury
  GeometryNode merkury{merkury_holder_pointer, "merkury", {0.5f, 0.2f, 0.2f}}; 
  auto merkury_pointer = std::make_shared<GeometryNode>(merkury);
  merkury_pointer->setGeometry(planet_model);
  float merkury_distance = 7.0f;
  merkury_pointer->setDistanceToCenter(merkury_distance);
  merkury_pointer->setLocalTransform(glm::scale(merkury_pointer->getLocalTransform(), glm::fvec3{0.4f})*
                                    (glm::translate(merkury_pointer->getLocalTransform(), glm::fvec3{merkury_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(merkury_pointer->getLocalTransform(), float(glfwGetTime())* 1.2f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  merkury_holder_pointer->addChild(merkury_pointer);
  scene.geometry_nodes_.push_back(merkury_pointer);

  //holder node for venus
  Node venus_holder{root_pointer, "venus_holder"};
  auto venus_holder_pointer = std::make_shared<Node>(venus_holder);
  venus_holder_pointer->setLocalTransform(glm::rotate(venus_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.2f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(venus_holder_pointer);

  //venus
  GeometryNode venus{venus_holder_pointer, "venus", {0.2f, 0.2f, 0.2f}}; 
  auto venus_pointer = std::make_shared<GeometryNode>(venus);
  venus_pointer->setGeometry(planet_model);
  float venus_distance = 12.0f;
  venus_pointer->setDistanceToCenter(venus_distance);
  venus_pointer->setLocalTransform(glm::scale(venus_pointer->getLocalTransform(), glm::fvec3{0.6})*
                                    (glm::translate(venus_pointer->getLocalTransform(), glm::fvec3{venus_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(venus_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  venus_holder_pointer->addChild(venus_pointer);
  scene.geometry_nodes_.push_back(venus_pointer);

  //holder node for earth
  Node earth_holder{root_pointer, "earth_holder"};
  auto earth_holder_pointer = std::make_shared<Node>(earth_holder);
  earth_holder_pointer->setLocalTransform(glm::rotate(earth_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.4f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(earth_holder_pointer);

  //earth
  GeometryNode earth{earth_holder_pointer, "earth", {0.5f, 0.2f, 0.2f}}; 
  auto earth_pointer = std::make_shared<GeometryNode>(earth);
  earth_pointer->setGeometry(planet_model);
  float earth_distance = 14.0f;
  earth_pointer->setDistanceToCenter(earth_distance);
  earth_pointer->setLocalTransform(glm::scale(earth_pointer->getLocalTransform(), glm::fvec3{0.3})*
                                    (glm::translate(earth_pointer->getLocalTransform(), glm::fvec3{earth_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(earth_pointer->getLocalTransform(), float(glfwGetTime())* 1.2f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  earth_holder_pointer->addChild(earth_pointer);
  scene.geometry_nodes_.push_back(earth_pointer);

  //holder node for moon
  Node moon_holder{earth_pointer, "moon_holder"};
  auto moon_holder_pointer = std::make_shared<Node>(moon_holder);
  moon_holder_pointer->setLocalTransform(glm::rotate(moon_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.4f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  earth_pointer->addChild(moon_holder_pointer);

  //moon
  GeometryNode moon{moon_holder_pointer, "moon", {0.5f, 0.2f, 0.2f}}; 
  auto moon_pointer = std::make_shared<GeometryNode>(moon);
  moon_pointer->setGeometry(planet_model);
  moon_pointer->setLocalTransform(glm::scale(moon_pointer->getLocalTransform(), glm::fvec3{0.3})*
                                    (glm::translate(moon_pointer->getLocalTransform(), glm::fvec3{5.0f, 0.0f, 0.0f}))*
                                    (glm::rotate(moon_pointer->getLocalTransform(), float(glfwGetTime())* 1.2f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  moon_holder_pointer->addChild(moon_pointer);
  scene.geometry_nodes_.push_back(moon_pointer);

  //holder node for mars
  Node mars_holder{root_pointer, "mars_holder"};
  auto mars_holder_pointer = std::make_shared<Node>(mars_holder);
  mars_holder_pointer->setLocalTransform(glm::rotate(mars_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.2f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(mars_holder_pointer);

  //mars
  GeometryNode mars{mars_holder_pointer, "mars", {0.5f, 0.2f, 0.2f}}; 
  auto mars_pointer = std::make_shared<GeometryNode>(mars);
  mars_pointer->setGeometry(planet_model);
  float mars_distance = 30.0f;
  mars_pointer->setDistanceToCenter(mars_distance);
  mars_pointer->setLocalTransform(glm::scale(mars_pointer->getLocalTransform(), glm::fvec3{0.1f})*
                                    (glm::translate(mars_pointer->getLocalTransform(), glm::fvec3{mars_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(mars_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  mars_holder_pointer->addChild(mars_pointer);
  scene.geometry_nodes_.push_back(mars_pointer);

  //holder node for jupiter
  Node jupiter_holder{root_pointer, "jupiter_holder"};
  auto jupiter_holder_pointer = std::make_shared<Node>(jupiter_holder);
  jupiter_holder_pointer->setLocalTransform(glm::rotate(jupiter_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.5f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(jupiter_holder_pointer);

  //jupiter
  GeometryNode jupiter{jupiter_holder_pointer, "jupiter", {0.5f, 0.2f, 0.2f}}; 
  auto jupiter_pointer = std::make_shared<GeometryNode>(jupiter);
  jupiter_pointer->setGeometry(planet_model);
  float jupiter_distance = 10.0f;
  jupiter_pointer->setDistanceToCenter(jupiter_distance);
  jupiter_pointer->setLocalTransform(glm::scale(jupiter_pointer->getLocalTransform(), glm::fvec3{0.9})*
                                    (glm::translate(jupiter_pointer->getLocalTransform(), glm::fvec3{jupiter_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(jupiter_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  jupiter_holder_pointer->addChild(jupiter_pointer);
  scene.geometry_nodes_.push_back(jupiter_pointer);

  //holder node for saturn
  Node saturn_holder{root_pointer, "saturn_holder"};
  auto saturn_holder_pointer = std::make_shared<Node>(saturn_holder);
  saturn_holder_pointer->setLocalTransform(glm::rotate(saturn_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.3f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(saturn_holder_pointer);

  //saturn
  GeometryNode saturn{saturn_holder_pointer, "saturn", {0.5f, 0.2f, 0.2f}}; 
  auto saturn_pointer = std::make_shared<GeometryNode>(saturn);
  saturn_pointer->setGeometry(planet_model);
  float saturn_distance = 12.0f;
  saturn_pointer->setDistanceToCenter(saturn_distance);
  saturn_pointer->setLocalTransform(glm::scale(saturn_pointer->getLocalTransform(), glm::fvec3{1.0f})*
                                    (glm::translate(saturn_pointer->getLocalTransform(), glm::fvec3{saturn_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(saturn_pointer->getLocalTransform(), float(glfwGetTime())* 0.7f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  saturn_holder_pointer->addChild(saturn_pointer);
  scene.geometry_nodes_.push_back(saturn_pointer);

  //holder node for uranus
  Node uranus_holder{root_pointer, "uranus_holder"};
  auto uranus_holder_pointer = std::make_shared<Node>(uranus_holder);
  uranus_holder_pointer->setLocalTransform(glm::rotate(uranus_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.7f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(uranus_holder_pointer);

  //uranus
  GeometryNode uranus{uranus_holder_pointer, "uranus", {0.2f, 0.2f, 0.2f}}; 
  auto uranus_pointer = std::make_shared<GeometryNode>(uranus);
  uranus_pointer->setGeometry(planet_model);
  float uranus_distance = 50.0f;
  uranus_pointer->setDistanceToCenter(uranus_distance);
  uranus_pointer->setLocalTransform(glm::scale(uranus_pointer->getLocalTransform(), glm::fvec3{0.2})*
                                    (glm::translate(uranus_pointer->getLocalTransform(), glm::fvec3{uranus_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(uranus_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  uranus_holder_pointer->addChild(uranus_pointer);
  scene.geometry_nodes_.push_back(uranus_pointer);

  //holder node for neptune
  Node neptune_holder{root_pointer, "neptune_holder"};
  auto neptune_holder_pointer = std::make_shared<Node>(neptune_holder);
  neptune_holder_pointer->setLocalTransform(glm::rotate(neptune_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.4f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(neptune_holder_pointer);

  //neptune
  GeometryNode neptune{neptune_holder_pointer, "neptune", {0.5f, 0.2f, 0.2f}}; 
  auto neptune_pointer = std::make_shared<GeometryNode>(neptune);
  neptune_pointer->setGeometry(planet_model);
  float neptune_distance = 15.0f;
  neptune_pointer->setDistanceToCenter(neptune_distance);
  neptune_pointer->setLocalTransform(glm::scale(neptune_pointer->getLocalTransform(), glm::fvec3{0.5})*
                                    (glm::translate(neptune_pointer->getLocalTransform(), glm::fvec3{neptune_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(neptune_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  neptune_holder_pointer->addChild(neptune_pointer);
  scene.geometry_nodes_.push_back(neptune_pointer);

  //calling to print the sceneGraph. Currently keeps printing. Ideally print once,
  scene.printGraph();

  //calling function to draw the objects (transform from object to world space and send to GPU)
  //giving children to be recursive -> maybe changing to iterative and giving the scene itself
  return scene;
}

//gets vector of pointers to children and draws everything in the scenegraph, which is below the root
void ApplicationSolar::drawGraph(SceneGraph scene) const{

   for (auto current_node : scene.geometry_nodes_) {
        //world transform first rotate then translate, because order makes a difference! 
        glm::fmat4 model_matrix = current_node->getWorldTransform();

        glUseProgram(m_shaders.at("planet").handle);
        // extra matrix for normal transformation to keep them orthogonal to surface
        glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
        glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                           1, GL_FALSE, glm::value_ptr(model_matrix));

        glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                           1, GL_FALSE, glm::value_ptr(normal_matrix));

        // bind the VAO to draw
        glBindVertexArray(planet_object.vertex_AO);

        int location = glGetUniformLocation(m_shaders.at("planet").handle, "planet_color");
        glUniform3f(location,current_node->getColor()[0],current_node->getColor()[1],current_node->getColor()[2] );

        // draw bound vertex array using bound shader
        glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
      }

}

void ApplicationSolar::createOrbits() {
  std::vector<float> orbits;
  uint points_in_circle = 360;

  for (int i = 0; i < points_in_circle; ++i) {
    GLfloat x = cos(i*M_PI/180); //calculating points on a circle in right order
    GLfloat y = 0;
    GLfloat z = sin(i*M_PI/180);
    orbits.push_back(x);
    orbits.push_back(y);
    orbits.push_back(z);
  }


  //Same as in draw stars
  glGenVertexArrays(1, &orbit_object.vertex_AO);
  glBindVertexArray(orbit_object.vertex_AO);

  glGenBuffers(1, &orbit_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(sizeof(float)*orbits.size()), orbits.data(), GL_STATIC_DRAW);

  //Attributes - index, size(3-dimensional), dtype, normalize data, byte-distance, offsets in bytes
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(sizeof(float)*3), 0);

  orbit_object.draw_mode = GL_LINE_LOOP; 
  orbit_object.num_elements = GLsizei(points_in_circle);
}

void ApplicationSolar::drawOrbits(SceneGraph const& scene) const{

  //go through planets and get distance and set as radius -> scale of circle. not yet correct
  for (auto const& planet : scene.geometry_nodes_) {
      glm::fmat4 orbit_matrix = glm::fmat4{1.0f};
      float radius = planet->getDistanceToCenter();
      orbit_matrix = glm::scale(orbit_matrix, glm::fvec3{radius, radius, radius});


      glUseProgram(m_shaders.at("orbit").handle);

      glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("OrbitMatrix"),
                               1, GL_FALSE, glm::value_ptr(orbit_matrix));


      glBindVertexArray(orbit_object.vertex_AO);
      
      glDrawArrays(orbit_object.draw_mode, GLint(0), orbit_object.num_elements);

    }
}

void ApplicationSolar::createRandomStars() {
  //Speichern einen Vektor mit Positionen und Farben fuer alle Sterne
  std::vector<float> positions_and_colors;

  //Anzahl der Sterne
  int num_stars = 6000;

  for (int star_index = 0; star_index < num_stars; ++star_index) {
    float rand_x = std::rand() % 100 - 50;
    positions_and_colors.push_back(rand_x);
    float rand_y = std::rand() % 100 - 50;
    positions_and_colors.push_back(rand_y);
    float rand_z = std::rand() % 100 - 50;
    positions_and_colors.push_back(-rand_z);

    float rand_x_color = std::rand() % 100;
    float rand_y_color = std::rand() % 10;
    float rand_z_color = std::rand() % 100;

    positions_and_colors.push_back((rand_x_color)/99.0f);
    positions_and_colors.push_back((rand_y_color)/99.0f);
    positions_and_colors.push_back((rand_z_color)/99.0f);
  }


  //following the slides
  //VAO is OpenGL object with necessary states for rendering. encapuslates vertex data
  glGenVertexArrays(1, &star_object.vertex_AO);
  glBindVertexArray(star_object.vertex_AO);

  //VBO is used as source data for Vertex Array Objects, bound to array buffer
  //carries vertex information such as position, color, normal...
  glGenBuffers(1, &star_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  // buffer, size of buffer, where is data, 
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*num_stars*6, positions_and_colors.data(), GL_STATIC_DRAW);

  //Attributes - index, size(3-dimensional), dtype, normalize data?, byte-distance, offsets in bytes
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(sizeof(float)*6), 0);

   //Attributes - index, size(3-dimensional), dtype, normalize data?, byte-distance, offsets in bytes
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, GLsizei(sizeof(float)*6), (void*) (sizeof(float)*3));


  star_object.draw_mode = GL_POINTS; 
  star_object.num_elements = GLsizei(num_stars);

}



void ApplicationSolar::uploadView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ModelViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_transform));

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));



}

void ApplicationSolar::uploadProjection() {
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() { 
  // bind shader to which to upload unforms
  //glUseProgram(m_shaders.at("planet").handle);
  // upload uniform values to new locations
  uploadView();
  uploadProjection();
}

///////////////////////////// intialisation functions /////////////////////////
// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/simple.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/simple.frag"}}});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;

  // create a new handle for the stars to use a different shader at vao.frag
  m_shaders.emplace("star", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/vao.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});
  // request uniform locations for shader program 
  m_shaders.at("star").u_locs["ModelViewMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;


  m_shaders.emplace("orbit", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/orbit.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/orbit.frag"}}});
  // request uniform locations for shader program
  m_shaders.at("orbit").u_locs["OrbitMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -1;

}

// load models
void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object 
  planet_object.num_elements = GLsizei(planet_model.indices.size());
}



///////////////////////////// callback functions for window events ////////////
// handle key input: W,S for Zoom and arrow keys for positioning
void ApplicationSolar::keyCallback(int key, int action, int mods) {
  if (key == GLFW_KEY_W  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.3f}); //transforms view on w key press -3 in z direction (zooms in)
    uploadView();
  }
  else if (key == GLFW_KEY_S  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.3f}); //transforms view on s key press 3 in z direction (zooms out)
    uploadView();
  } else if (key == GLFW_KEY_UP  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, -0.3f, 0.0f}); //transforms view on up key press -3 in y direction
    uploadView();
  } else if (key == GLFW_KEY_DOWN  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.3f, 0.0f}); //transforms view on up key press 3 in y direction
    uploadView();
  } else if (key == GLFW_KEY_LEFT  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.3f, 0.0f, 0.0f}); //transforms view on up key press -3 in x direction
    uploadView();
  } else if (key == GLFW_KEY_RIGHT  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-0.3f, 0.0f, 0.0f}); //transforms view on up key press 3 in x direction
    uploadView();
  }

}

//handle delta mouse movement input: rotation of view --> not perfect yet. better would be a rotation around the center of screen! 
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
    float horizontal_rotate = float(pos_x/20); //divide to slow down
    float vertical_rotate = float(pos_y/20);

    //glm rotate transforms a matrix 4x4, created from axis of 3 scalars and angle in degree (glm::radians). Scalars define the axis for rotation
    m_view_transform = glm::rotate(m_view_transform, glm::radians(vertical_rotate), glm::vec3{1.0f,0.0f,0.0f});
    m_view_transform = glm::rotate(m_view_transform, glm::radians(horizontal_rotate), glm::vec3{0.0f,1.0f,0.0f});


    uploadView();
}

//handle resizing
void ApplicationSolar::resizeCallback(unsigned width, unsigned height) {
  // recalculate projection matrix for new aspect ration
  m_view_projection = utils::calculate_projection_matrix(float(width) / float(height));
  // upload new projection matrix
  uploadProjection();
}


// exe entry point
int main(int argc, char* argv[]) {
  Application::run<ApplicationSolar>(argc, argv, 3, 2);
}