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
 ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})} //camera position
 ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
{
  initializeGeometry();
  initializeShaderPrograms();
  createRandomStars();
}

//destructor
ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);
}

void ApplicationSolar::render() const {

  glUseProgram(m_shaders.at("planet").handle);

  //calling the function to create planets
  createPlanetSystem();

}


void ApplicationSolar::createPlanetSystem() const{

  // we load a circular model from the resources
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

  // creating an empty root node with nullptr as parent, named /, path / and 0 depth
  Node root{nullptr, "/"};
  auto root_holder = std::make_shared<Node>(root);
  SceneGraph scene = SceneGraph("scene", root_holder);

  //Constructor for Geometry Nodes (parent, name, size, speed, position(vec3), model.obj)
  //sun
  GeometryNode sun{root_holder, "sun", 3.0f, 1.0f, {0.0f, 0.0f, 0.0f}, planet_model};
  auto sun_holder = std::make_shared<GeometryNode>(sun);
  root_holder->addChild(sun_holder);


  //holder node for merkury
  Node merkury_holder{root_holder, "merkury_holder", };
  auto merkury_holder_pointer = std::make_shared<Node>(merkury_holder);
  merkury_holder_pointer->setLocalTransform(glm::translate({}, glm::fvec3{3.0f, 0.0f, 0.0f}));
  root_holder->addChild(merkury_holder_pointer);

  //merkury
  GeometryNode merkury{root_holder, "merkury"}; 
  auto merkury_pointer = std::make_shared<GeometryNode>(merkury);
  merkury_pointer->setGeometry(planet_model);
  merkury_pointer->setLocalTransform(glm::scale({}, glm::fvec3{0.05f}));
  merkury_holder_pointer->addChild(merkury_pointer);


  //venus
  GeometryNode venus{root_holder, "venus", 17.0f, 1.0f, {13.0f, 0.0f, 0.0f}, planet_model}; 
  auto venus_holder = std::make_shared<GeometryNode>(venus);
  root_holder->addChild(venus_holder);

  //earth
  GeometryNode earth{root_holder, "earth", 15.0f, 1.2f, {14.0f, 0.0f, 0.0f}, planet_model}; 
  auto earth_holder = std::make_shared<GeometryNode>(earth);
  root_holder->addChild(earth_holder);

  //moon
  GeometryNode moon{earth_holder, "moon", 2.0f, 1.8f, {4.0f, 0.0f, 0.0f}, planet_model}; 
  auto moon_holder = std::make_shared<GeometryNode>(moon);
  earth_holder->addChild(moon_holder); //adds moon to child of earth not root

  //mars
  GeometryNode mars{root_holder, "mars", 15.0f, 0.1f, {15.0f, 0.0f, 0.0f}, planet_model}; 
  auto mars_holder = std::make_shared<GeometryNode>(mars);
  root_holder->addChild(mars_holder);

  //jupiter
  GeometryNode jupiter{root_holder, "jupiter", 7.0f, 0.3f, {11.0f, 0.0f, 0.0f}, planet_model}; 
  auto jupiter_holder = std::make_shared<GeometryNode>(jupiter);
  root_holder->addChild(jupiter_holder);

  //saturn
  GeometryNode saturn{root_holder, "saturn", 10.0f, 0.8f, {16.0f, 0.0f, 0.0f}, planet_model}; 
  auto saturn_holder = std::make_shared<GeometryNode>(saturn);
  root_holder->addChild(saturn_holder);

  //uranus
  GeometryNode uranus{root_holder, "uranus", 12.0f, 0.4f, {28.0f, 0.0f, 0.0f}, planet_model}; 
  auto uranus_holder = std::make_shared<GeometryNode>(uranus);
  root_holder->addChild(uranus_holder);

  //neptune
  GeometryNode neptune{root_holder, "neptune", 13.0f, 0.1f, {34.0f, 0.0f, 0.0f}, planet_model}; 
  auto neptune_holder = std::make_shared<GeometryNode>(neptune);
  root_holder->addChild(neptune_holder);

  //calling to print the sceneGraph. Currently keeps printing. Ideally print once,
  scene.printGraph();

  //calling function to draw the objects (transform from object to world space and send to GPU)
  //giving children to be recursive -> maybe changing to iterative and giving the scene itself
  drawGraph(scene);
}

//gets vector of pointers to children and draws everything in the scenegraph, which is below the root
void ApplicationSolar::drawGraph(SceneGraph scene) const{

  std::queue<std::shared_ptr<Node>> rendering_queue;
  for (auto& child : scene.getRoot()->getChildrenList()){
    rendering_queue.push(child);
  }

  while(!rendering_queue.empty()) {
    auto& current_node = rendering_queue.front();
    rendering_queue.pop();

    glm::fmat4 model_matrix = current_node->getWorldTransform();

    //world transform first rotate then translate, because order makes a difference! 
    model_matrix = glm::rotate(model_matrix,float(glfwGetTime()) * current_node->getSpeed(), glm::fvec3{0.0f, 1.0f, 0.0f}); //rotation of planet 
    model_matrix = glm::translate(model_matrix, current_node->getPosition()); //translation 


    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));

    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normal_matrix));

    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

    //recursively calls function for children 
    if(current_node->getChildrenList().size()>0) {
      std::cout<<" i was found " << current_node->getName()<<std::endl;
      for (auto& child : current_node->getChildrenList()){
        rendering_queue.push(child);
      }
    }
  }
}

void ApplicationSolar::createRandomStars() const{
  std::vector<float> positions;
  int num_stars = 500;

  for (int star_index; star_index < num_stars; ++star_index) {
    float rand_x = std::rand() % 100;
    positions.push_back(rand_x);
    float rand_y = std::rand() % 100;
    positions.push_back(rand_y);
    float rand_z = std::rand() % 100;
    positions.push_back(rand_z);
  }

  /*
  //following the slides
  //VAO is OpenGL object with necessary states for rendering. encapuslates vertex data
  glGenVertexArrays(1, &star_object.vertex_AO);
  glBindVertexArray(star_object.vertex_AO);

  //VBO is used as source data for Vertex Array Objects, bound to array buffer
  //carries vertex information such as position, color, normal...
  glGenBuffers(1, &star_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  //glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(num_stars), ??, GL_STATIC_DRAW);


  /*
  //Index Buffer is optional Buffer Object used for defining a reading order for vertices


  //Attributes - index, size(offset), dtype, normalize data?, byte-distance, offsets in bytes
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, GLsizei(sizeof(float)*3), GLvoid*);

  star_object.draw_mode = GL_POINTS; //important for us!
  star_object.num_elements = GLsizei(num_stars);

  */

}

void ApplicationSolar::uploadView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::uploadProjection() {
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() { 
  // bind shader to which to upload unforms
  glUseProgram(m_shaders.at("planet").handle);
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