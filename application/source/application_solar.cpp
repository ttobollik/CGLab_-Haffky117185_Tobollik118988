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

#include <iostream>

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})} //camera position
 ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
{
  initializeGeometry();
  initializeShaderPrograms();
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

void ApplicationSolar::render() const {
  /***
  // bind shader to upload uniforms
  glUseProgram(m_shaders.at("planet").handle);

  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f}); //rotation of planet
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f}); //translation of planet
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));

  // bind the VAO to draw
  glBindVertexArray(planet_object.vertex_AO);

  // draw bound vertex array using bound shader
  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL); 
  ***/
  glUseProgram(m_shaders.at("planet").handle);
  createPlanetSystem();

}


void ApplicationSolar::createPlanetSystem() const{

  // we load a circular model from the resources
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

  // creating an empty root node with nullptr as parent, named /, path / and 0 depth
  Node root{nullptr, "/"};
  auto root_pointer = std::make_shared<Node>(root);
  SceneGraph scene = SceneGraph("scene", root_pointer);

  //Constructor for Geometry Nodes (parent, name, size, speed, position, model.obj)
  GeometryNode sun{root_pointer, "sun", 3.0f, 1.0f, {0.0f, 0.0f, 0.0f}, planet_model};
  auto sun_pointer = std::make_shared<GeometryNode>(sun);
  root_pointer->addChild(sun_pointer);



  GeometryNode merkury{root_pointer, "merkury"}; 
  merkury.setGeometry(planet_model);
  //set a transformation for the sun and make a pointer
  merkury.setLocalTransform(glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 
                                   0.0f, 0.0f, 0.0f, 20.0f}); //last element for size
  merkury.setPosition({11.0f, 0.0f, 0.0f});
  merkury.setSpeed(1.2f); //high number --> faster because multiplied with time in render
  auto merkury_pointer = std::make_shared<GeometryNode>(merkury);

  root_pointer->addChild(merkury_pointer);


  GeometryNode venus{root_pointer, "venus"}; 
  venus.setGeometry(planet_model);
  //set a transformation for the sun and make a pointer
  venus.setLocalTransform(glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 
                                   0.0f, 0.0f, 0.0f, 17.0f}); //last element for size
  venus.setPosition({13.0f, 0.0f, 0.0f});
  venus.setSpeed(1.0f); 
  auto venus_pointer = std::make_shared<GeometryNode>(venus);
  root_pointer->addChild(venus_pointer);

  GeometryNode earth{root_pointer, "earth"}; 
  earth.setGeometry(planet_model);
  //set a transformation for the sun and make a pointer
  earth.setLocalTransform(glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 
                                   0.0f, 0.0f, 0.0f, 15.0f}); //last element for size
  earth.setPosition({14.0f, 0.0f, 0.0f});
  earth.setSpeed(1.2f);
  auto earth_pointer = std::make_shared<GeometryNode>(earth);
  root_pointer->addChild(earth_pointer);


  GeometryNode moon{earth_pointer, "moon"}; 
  moon.setGeometry(planet_model);
  //set a transformation for the sun and make a pointer
  moon.setLocalTransform(glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 
                                   0.0f, 0.0f, 0.0f, 2.0f}); //last element for size
  moon.setPosition({4.0f, 0.0f, 0.0f});
  moon.setSpeed(1.8f);
  auto moon_pointer = std::make_shared<GeometryNode>(moon);
  earth_pointer->addChild(moon_pointer);

  
  GeometryNode mars{root_pointer, "mars"}; 
  mars.setGeometry(planet_model);
  //set a transformation for the sun and make a pointer
  mars.setLocalTransform(glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 
                                   0.0f, 0.0f, 0.0f, 15.0f}); //last element for size
  mars.setPosition({15.0f, 0.0f, 0.0f});
  mars.setSpeed(0.1f);
  auto mars_pointer = std::make_shared<GeometryNode>(mars);
  root_pointer->addChild(mars_pointer);

  
  GeometryNode jupiter{root_pointer, "jupiter"}; 
  jupiter.setGeometry(planet_model);
  //set a transformation for the sun and make a pointer
  jupiter.setLocalTransform(glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 
                                   0.0f, 0.0f, 0.0f, 7.0f}); //last element for size
  jupiter.setPosition({11.0f, 0.0f, 0.0f});
  jupiter.setSpeed(0.3f);
  auto jupiter_pointer = std::make_shared<GeometryNode>(jupiter);
  root_pointer->addChild(jupiter_pointer);


  GeometryNode saturn{root_pointer, "saturn"}; 
  saturn.setGeometry(planet_model);
  //set a transformation for the sun and make a pointer
  saturn.setLocalTransform(glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 
                                   0.0f, 0.0f, 0.0f, 10.0f}); //last element for size
  saturn.setPosition({16.0f, 0.0f, 0.0f});
  saturn.setSpeed(0.8f);
  auto saturn_pointer = std::make_shared<GeometryNode>(saturn);
  root_pointer->addChild(saturn_pointer);


  GeometryNode uranus{root_pointer, "uranus"}; 
  uranus.setGeometry(planet_model);
  //set a transformation for the sun and make a pointer
  uranus.setLocalTransform(glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 
                                   0.0f, 0.0f, 0.0f, 12.0f}); //last element for size
  uranus.setPosition({28.0f, 0.0f, 0.0f});
  uranus.setSpeed(0.4f);
  auto uranus_pointer = std::make_shared<GeometryNode>(uranus);
  root_pointer->addChild(uranus_pointer);

  
  GeometryNode neptune{root_pointer, "neptune"}; 
  neptune.setGeometry(planet_model);
  //set a transformation for the sun and make a pointer
  neptune.setLocalTransform(glm::fmat4{1.0f, 0.0f, 0.0f, 0.0f, 
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 
                                   0.0f, 0.0f, 0.0f, 13.0f}); //last element for size
  neptune.setPosition({34.0f, 0.0f, 0.0f});
  neptune.setSpeed(0.1f);
  auto neptune_pointer = std::make_shared<GeometryNode>(neptune);
  root_pointer->addChild(neptune_pointer);


  scene.printGraph();
  drawGraph(root_pointer->getChildrenList());
}

void ApplicationSolar::drawGraph(std::vector<std::shared_ptr<Node>> children) const{
  //std::cout<<merkury_pointer->hasChild("earth")<<std::endl;


  if (children.size()>0){
    for (auto const& child : children) {

        glm::fmat4 model_matrix = glm::fmat4{1.0};
        if(child->getDepth() >= 2){
            model_matrix = child->getParent()->getLocalTransform();
            model_matrix = glm::rotate(model_matrix, float(glfwGetTime())*(child->getParent()->getSpeed()), glm::fvec3{0.0f, 1.0f, 0.0f});
            model_matrix = glm::translate(model_matrix, child->getParent()->getPosition());
        }
        
        model_matrix = glm::rotate(model_matrix * child->getLocalTransform(),float(glfwGetTime()) * child->getSpeed(), glm::fvec3{0.0f, 1.0f, 0.0f});
        model_matrix = glm::translate(model_matrix, child->getPosition());

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
        if(child->getChildrenList().size()>0) {
          drawGraph(child->getChildrenList());
        }
      }
    }
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
// handle key input
void ApplicationSolar::keyCallback(int key, int action, int mods) {
  if (key == GLFW_KEY_W  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.3f});
    uploadView();
  }
  else if (key == GLFW_KEY_S  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.3f});
    uploadView();
  } else if (key == GLFW_KEY_UP  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, -0.3f, 0.0f});
    uploadView();
  } else if (key == GLFW_KEY_DOWN  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.3f, 0.0f});
    uploadView();
  } else if (key == GLFW_KEY_LEFT  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.3f, 0.0f, 0.0f});
    uploadView();
  } else if (key == GLFW_KEY_RIGHT  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-0.3f, 0.0f, 0.0f});
    uploadView();
  }

}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
    float horizontal_rotate = float(pos_x/20); //divide to slow down
    float vertical_rotate = float(pos_y/20);

    //glm rotate transforms a matrix 4x4, created from axis of 3 scalars and angle in degree (glm::radians). Scalars define the axis for rotation
    m_view_transform = glm::rotate(m_view_transform, glm::radians(horizontal_rotate), glm::vec3{0,1,0});
    m_view_transform = glm::rotate(m_view_transform, glm::radians(vertical_rotate), glm::vec3{1,0,0});

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