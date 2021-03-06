#include "application_solar.hpp"
#include "window_handler.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "scene_graph.hpp"
#include "texture_loader.hpp"

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



std::vector<GLfloat> quadVertices = {
  //hard coded first triangle
  -1.0f, 1.0f, 0.0f, 1.0f,
  -1.0f, -1.0f, 0.0f, 0.0f,
  1.0f, -1.0f, 1.0f, 0.0f,

  //hard coded second triangle
  -1.0f, 1.0f, 0.0f, 1.0f,
  1.0f, -1.0f, 1.0f, 0.0f,
  1.0f, 1.0f, 1.0f, 1.0f
};

bool greyscale = false;
bool horizontalMirror = false;
bool verticalMirror = false;
bool blur = false;




void ApplicationSolar::generateQuadObjects() const{

  glBindFramebuffer(GL_FRAMEBUFFER, 0); //zero is the default framebuffer
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT); 
  glDisable(GL_DEPTH_TEST); //depth test unnecessary because we just render a texture 

  unsigned int quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);

  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*quadVertices.size(), quadVertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, GLsizei(4 * sizeof(float)), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, GLsizei(4 * sizeof(float)), (void*)(2*sizeof(float)));


  glUseProgram(m_shaders.at("quad").handle);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, texture_framebuffer.handle);

  glUniform1i(m_shaders.at("quad").u_locs.at("TextureFragment"), 2);
  

  glDrawArrays(GL_TRIANGLES, 0, 6);
}


//constructor
ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,star_object{}
 ,orbit_object{}
 ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})} //camera position
 ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
 ,scene_{}
{
  createPlanetSystem();
  createRandomStars();
  initializeTexture();
  initializeGeometry();
  initializeShaderPrograms();
  createOrbits();
  initializeFrameBuffer(initial_resolution.x, initial_resolution.y);

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

  //first we bind the framebuffer and render everything to it
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.handle);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  drawGraph();

  //glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
  //drawOrbits();

  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  drawStars();


  //now it is time to go back to the default framebuffer - here we want to use the color attachment from the framebuffer to render it to a quad
  generateQuadObjects();
  

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


void ApplicationSolar::createPlanetSystem(){

  // we load a circular model from the resources
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD); //changes due to suggestion from Francesco

  // creating an empty root node with nullptr as parent, named /, path / and 0 depth
  Node root{nullptr, "/"};
  auto root_pointer = std::make_shared<Node>(root);
  scene_ = SceneGraph("scene", root_pointer);

  //cameraNode 
  CameraNode camera{root_pointer, "camera", true, true, m_view_projection};
  std::shared_ptr<CameraNode> camera_pointer = std::make_shared<CameraNode>(camera);
  //root_pointer->addChild(camera_pointer); throws error
  scene_.camera_ = camera_pointer;

  //pointLightNode
  PointLightNode sun_light{root_pointer, "sunlight", 1.3f, {1.0f, 1.0f, 1.0f}};
  auto sunlight_pointer = std::make_shared<PointLightNode>(sun_light);
  root_pointer->addChild(sunlight_pointer);
  scene_.point_light_nodes_.push_back(sunlight_pointer);

  //sun
  GeometryNode sun{root_pointer, "sun", {1.0f, 0.863f, 0.0f}}; 
  auto sun_pointer = std::make_shared<GeometryNode>(sun);
  sun_pointer->setGeometry(planet_model);
  sun_pointer->set_texture_path("sunmap.png");
  sun_pointer->setLocalTransform(glm::scale(sun_pointer->getLocalTransform(), glm::fvec3{2.0f})*
                                    (glm::translate(sun_pointer->getLocalTransform(), glm::fvec3{0.0f, 0.0f, 0.0f})));
  root_pointer->addChild(sun_pointer);
  scene_.geometry_nodes_.push_back(sun_pointer);

  //holder node for merkury
  Node merkury_holder{root_pointer, "merkury_holder"};
  auto merkury_holder_pointer = std::make_shared<Node>(merkury_holder);
  merkury_holder_pointer->setLocalTransform(glm::rotate(merkury_holder_pointer->getLocalTransform(), float(glfwGetTime())* 1.9f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(merkury_holder_pointer);

  //merkury
  GeometryNode merkury{merkury_holder_pointer, "merkury", {0.667f, 0.667f, 0.667f}}; 
  auto merkury_pointer = std::make_shared<GeometryNode>(merkury);
  merkury_pointer->setGeometry(planet_model);
  float merkury_distance = 7.0f;
  merkury_pointer->set_texture_path("mercurymap.png"); //setting the filename for the texture
  merkury_pointer->setDistanceToCenter(merkury_distance);
  merkury_pointer->setLocalTransform(glm::scale(merkury_pointer->getLocalTransform(), glm::fvec3{0.4f})*
                                    (glm::translate(merkury_pointer->getLocalTransform(), glm::fvec3{merkury_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(merkury_pointer->getLocalTransform(), float(glfwGetTime())* 1.2f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  merkury_holder_pointer->addChild(merkury_pointer);
  scene_.geometry_nodes_.push_back(merkury_pointer);

  //holder node for venus
  Node venus_holder{root_pointer, "venus_holder"};
  auto venus_holder_pointer = std::make_shared<Node>(venus_holder);
  venus_holder_pointer->setLocalTransform(glm::rotate(venus_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.2f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(venus_holder_pointer);

  //venus
  GeometryNode venus{venus_holder_pointer, "venus", {1.0f, 0.8f, 0.03f}}; 
  auto venus_pointer = std::make_shared<GeometryNode>(venus);
  venus_pointer->setGeometry(planet_model);
  venus_pointer->set_texture_path("venusmap.png");
  float venus_distance = 12.0f;
  venus_pointer->setDistanceToCenter(venus_distance);
  venus_pointer->setLocalTransform(glm::scale(venus_pointer->getLocalTransform(), glm::fvec3{0.6})*
                                    (glm::translate(venus_pointer->getLocalTransform(), glm::fvec3{venus_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(venus_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  venus_holder_pointer->addChild(venus_pointer);
  scene_.geometry_nodes_.push_back(venus_pointer);

  //holder node for earth
  Node earth_holder{root_pointer, "earth_holder"};
  auto earth_holder_pointer = std::make_shared<Node>(earth_holder);
  earth_holder_pointer->setLocalTransform(glm::rotate(earth_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.2f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(earth_holder_pointer);

  //earth
  GeometryNode earth{earth_holder_pointer, "earth", {0.0f, 0.2f, 0.9f}}; 
  auto earth_pointer = std::make_shared<GeometryNode>(earth);
  earth_pointer->setGeometry(planet_model);
  earth_pointer->set_texture_path("earthmap.png");
  float earth_distance = 20.0f;
  earth_pointer->setDistanceToCenter(earth_distance);
  earth_pointer->setLocalTransform(glm::scale(earth_pointer->getLocalTransform(), glm::fvec3{0.3})*
                                    (glm::translate(earth_pointer->getLocalTransform(), glm::fvec3{earth_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(earth_pointer->getLocalTransform(), float(glfwGetTime())* 2.2f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  earth_holder_pointer->addChild(earth_pointer);
  scene_.geometry_nodes_.push_back(earth_pointer);

  //holder node for moon
  Node moon_holder{earth_pointer, "moon_holder"};
  auto moon_holder_pointer = std::make_shared<Node>(moon_holder);
  moon_holder_pointer->setLocalTransform(glm::rotate(moon_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.4f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  earth_pointer->addChild(moon_holder_pointer);

  //moon
  GeometryNode moon{moon_holder_pointer, "moon", {0.667f, 0.667f, 0.667f}}; 
  auto moon_pointer = std::make_shared<GeometryNode>(moon);
  moon_pointer->setGeometry(planet_model);
  moon_pointer->setLocalTransform(glm::scale(moon_pointer->getLocalTransform(), glm::fvec3{0.3})*
                                    (glm::translate(moon_pointer->getLocalTransform(), glm::fvec3{5.0f, 0.0f, 0.0f}))*
                                    (glm::rotate(moon_pointer->getLocalTransform(), float(glfwGetTime())* 3.2f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  moon_holder_pointer->addChild(moon_pointer);
  scene_.geometry_nodes_.push_back(moon_pointer);

  //holder node for mars
  Node mars_holder{root_pointer, "mars_holder"};
  auto mars_holder_pointer = std::make_shared<Node>(mars_holder);
  mars_holder_pointer->setLocalTransform(glm::rotate(mars_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.2f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(mars_holder_pointer);

  //mars
  GeometryNode mars{mars_holder_pointer, "mars", {1.0f, 0.1f, 0.1f}}; 
  auto mars_pointer = std::make_shared<GeometryNode>(mars);
  mars_pointer->setGeometry(planet_model);
  float mars_distance = 30.0f;
  mars_pointer->setDistanceToCenter(mars_distance);
  mars_pointer->setLocalTransform(glm::scale(mars_pointer->getLocalTransform(), glm::fvec3{0.1f})*
                                    (glm::translate(mars_pointer->getLocalTransform(), glm::fvec3{mars_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(mars_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  mars_holder_pointer->addChild(mars_pointer);
  scene_.geometry_nodes_.push_back(mars_pointer);

  //holder node for jupiter
  Node jupiter_holder{root_pointer, "jupiter_holder"};
  auto jupiter_holder_pointer = std::make_shared<Node>(jupiter_holder);
  jupiter_holder_pointer->setLocalTransform(glm::rotate(jupiter_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.5f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(jupiter_holder_pointer);

  //jupiter
  GeometryNode jupiter{jupiter_holder_pointer, "jupiter", {0.0f, 0.9f, 0.1f}}; 
  auto jupiter_pointer = std::make_shared<GeometryNode>(jupiter);
  jupiter_pointer->setGeometry(planet_model);
  jupiter_pointer->set_texture_path("jupitermap.png");
  float jupiter_distance = 40.0f;
  jupiter_pointer->setDistanceToCenter(jupiter_distance);
  jupiter_pointer->setLocalTransform(glm::scale(jupiter_pointer->getLocalTransform(), glm::fvec3{0.9})*
                                    (glm::translate(jupiter_pointer->getLocalTransform(), glm::fvec3{jupiter_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(jupiter_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  jupiter_holder_pointer->addChild(jupiter_pointer);
  scene_.geometry_nodes_.push_back(jupiter_pointer);

  //holder node for saturn
  Node saturn_holder{root_pointer, "saturn_holder"};
  auto saturn_holder_pointer = std::make_shared<Node>(saturn_holder);
  saturn_holder_pointer->setLocalTransform(glm::rotate(saturn_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.3f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(saturn_holder_pointer);

  //saturn
  GeometryNode saturn{saturn_holder_pointer, "saturn", {0.694f, 0.052f, 0.788f}}; 
  auto saturn_pointer = std::make_shared<GeometryNode>(saturn);
  saturn_pointer->setGeometry(planet_model);
  float saturn_distance = 50.0f;
  saturn_pointer->set_texture_path("saturnmap.png");
  saturn_pointer->setDistanceToCenter(saturn_distance);
  saturn_pointer->setLocalTransform(glm::scale(saturn_pointer->getLocalTransform(), glm::fvec3{1.0f})*
                                    (glm::translate(saturn_pointer->getLocalTransform(), glm::fvec3{saturn_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(saturn_pointer->getLocalTransform(), float(glfwGetTime())* 0.7f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  saturn_holder_pointer->addChild(saturn_pointer);
  scene_.geometry_nodes_.push_back(saturn_pointer);

  //holder node for uranus
  Node uranus_holder{root_pointer, "uranus_holder"};
  auto uranus_holder_pointer = std::make_shared<Node>(uranus_holder);
  uranus_holder_pointer->setLocalTransform(glm::rotate(uranus_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.7f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(uranus_holder_pointer);

  //uranus
  GeometryNode uranus{uranus_holder_pointer, "uranus", {1.0f, 1.0f, 1.0f}}; 
  auto uranus_pointer = std::make_shared<GeometryNode>(uranus);
  uranus_pointer->setGeometry(planet_model);
  float uranus_distance = 60.0f;
  uranus_pointer->setDistanceToCenter(uranus_distance);
  uranus_pointer->setLocalTransform(glm::scale(uranus_pointer->getLocalTransform(), glm::fvec3{0.2})*
                                    (glm::translate(uranus_pointer->getLocalTransform(), glm::fvec3{uranus_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(uranus_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  uranus_holder_pointer->addChild(uranus_pointer);
  scene_.geometry_nodes_.push_back(uranus_pointer);

  //holder node for neptune
  Node neptune_holder{root_pointer, "neptune_holder"};
  auto neptune_holder_pointer = std::make_shared<Node>(neptune_holder);
  neptune_holder_pointer->setLocalTransform(glm::rotate(neptune_holder_pointer->getLocalTransform(), float(glfwGetTime())* 0.4f, glm::fvec3{0.0f, 1.0f, 0.0f}));
  root_pointer->addChild(neptune_holder_pointer);

  //neptune
  GeometryNode neptune{neptune_holder_pointer, "neptune", {0.3f, 0.9f, 1.0f}}; 
  auto neptune_pointer = std::make_shared<GeometryNode>(neptune);
  neptune_pointer->setGeometry(planet_model);
  neptune_pointer->set_texture_path("neptunemap.png");
  float neptune_distance = 70.0f;
  neptune_pointer->setDistanceToCenter(neptune_distance);
  neptune_pointer->setLocalTransform(glm::scale(neptune_pointer->getLocalTransform(), glm::fvec3{0.5})*
                                    (glm::translate(neptune_pointer->getLocalTransform(), glm::fvec3{neptune_distance, 0.0f, 0.0f}))*
                                    (glm::rotate(neptune_pointer->getLocalTransform(), float(glfwGetTime())* 1.0f, glm::fvec3{0.0f, 1.0f, 0.0f})));
  neptune_holder_pointer->addChild(neptune_pointer);
  scene_.geometry_nodes_.push_back(neptune_pointer);

  //calling to print the sceneGraph. Currently keeps printing. Ideally print once,
  scene_.printGraph();

  //calling function to draw the objects (transform from object to world space and send to GPU)
  //giving children to be recursive -> maybe changing to iterative and giving the scene itself
  //return scene;
}

//gets vector of pointers to children and draws everything in the scenegraph, which is below the root------------------------------------------------------
void ApplicationSolar::drawGraph() const{

  int i = 0;
   for (auto current_node : scene_.geometry_nodes_) {        

        glm::mat4 pl = current_node->getParent()->getLocalTransform();

        glm::mat4 rm = {};

        rm = glm::rotate(glm::mat4x4{}, current_node->getSpeed()*0.0001f, glm::fvec3(0.0f, 1.0f, 0.0f));

        current_node->getParent()->setLocalTransform(rm*pl);

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


        glUniform3f(glGetUniformLocation(m_shaders.at("planet").handle, "planet_color"),
                  current_node->getColor()[0],current_node->getColor()[1],current_node->getColor()[2] );

        for (auto light : scene_.point_light_nodes_) {
            glm::fvec3 light_color = light->getLightColor();
            float light_intensity = light->getLightIntensity();
            glm::fvec4 light_position = light->getWorldTransform()*glm::fvec4(0.0f, 0.0f, 0.0f, 1.0f); 
            glUniform3f(glGetUniformLocation(m_shaders.at("planet").handle, "light_color"),
                        light_color[0],light_color[1], light_color[2]);
            glUniform1f(glGetUniformLocation(m_shaders.at("planet").handle, "light_intensity"),
                        light_intensity);
            glUniform3f(glGetUniformLocation(m_shaders.at("planet").handle, "light_position"),
                        light_position[0],light_position[1], light_position[2]);
            if (current_node->getName() == "sun") {
              glUniform1f(glGetUniformLocation(m_shaders.at("planet").handle, "sun"),
                        1.0f);
            } else {
              glUniform1f(glGetUniformLocation(m_shaders.at("planet").handle, "sun"),
                        0.0f);
            }
            

        }

        //get texture object abd activate index, bind with handle und uniform hochladen! 

        texture_object texture = current_node->get_texture_obj(); /****************receive texture*/


        glActiveTexture(GL_TEXTURE1+i); /***************bind texture*/
        glBindTexture(texture.target, texture.handle);

        glUniform1i(glGetUniformLocation(m_shaders.at("planet").handle,"Texture"), texture.handle);

        // draw bound vertex array using bound shader
        glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
        i++;
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

void ApplicationSolar::drawOrbits() const{

  //go through planets and get distance and set as radius -> scale of circle. not yet correct
  for (auto const& planet : scene_.geometry_nodes_) {
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

  glUseProgram(m_shaders.at("quad").handle);
  glUniform1i(m_shaders.at("quad").u_locs.at("greyscale"), greyscale);
  glUniform1i(m_shaders.at("quad").u_locs.at("horizontalMirror"), horizontalMirror);
  glUniform1i(m_shaders.at("quad").u_locs.at("verticalMirror"), verticalMirror);
  glUniform1i(m_shaders.at("quad").u_locs.at("blur"), blur);

  /*

  glUseProgram(m_shaders.at("skybox").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  */


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

/*
  glUseProgram(m_shaders.at("skybox").handle);
  glUniformMatrix4fv(m_shaders.at("skybox").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
                     */


}

// update uniform locations
void ApplicationSolar::uploadUniforms() { 
  // bind shader to which to upload unforms
  //glUseProgram(m_shaders.at("planet").handle);
  // upload uniform values to new locations
  uploadView();
  uploadProjection();
}

///////////////////////////// initialisation functions /////////////////////////
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


  m_shaders.emplace("quad", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/framebuffer.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/framebuffer.frag"}}});
  // request uniform locations for shader program
  m_shaders.at("quad").u_locs["TextureFragment"] = -1;
  m_shaders.at("quad").u_locs["greyscale"] = 0;
  m_shaders.at("quad").u_locs["horizontalMirror"] = 0;
  m_shaders.at("quad").u_locs["verticalMirror"] = 0;
  m_shaders.at("quad").u_locs["blur"] = 0;



/*
  m_shaders.emplace("skybox", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/skybox.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/skybox.frag"}}});
  // request uniform locations for shader program
  m_shaders.at("skybox").u_locs["NormalMatrix"] = -1;
  m_shaders.at("skybox").u_locs["ViewMatrix"] = -1;
  m_shaders.at("skybox").u_locs["ProjectionMatrix"] = -1;
  */

}

void ApplicationSolar::initializeFrameBuffer(unsigned width, unsigned height){

//need to define depth and color buffer and then bind to framebuffer (color for us will be texture as defined in exercise)


//generating Depth buffer so we can bind it via its handle to the frame buffer
  glGenRenderbuffers(1, &renderbuffer_framebuffer.handle);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_framebuffer.handle);

  //to create deoth and stencilbuffer object we need renderbuffersotrage functin (learn open gl - framebuffers)
  //for internal format we could also use DEPTH24_STENCIL8
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);


//generating Texture for Color (similar to learn open gl website)

  glActiveTexture(GL_TEXTURE2); //1 is for planet textures
  glGenTextures(1, &texture_framebuffer.handle);
  //first parameter is render target
  glBindTexture(GL_TEXTURE_2D, texture_framebuffer.handle); //need to bind it so abz subsequent texture commands will configure the currently bound texture


  //texture target (2D means will generate texture on currently cound texture object), mipmap level (0 is base)
  //tells OpenGl what kind of format we want to store texture, width and height of window, legacy stuff always zero
  //format and data tzpe of source image, image data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);


  //if we use texture the result is stored as texture and is easily used in shaders
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);



//defining FramBuffer
  glGenFramebuffers(1, &framebuffer.handle);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.handle);

  //define attachments -> for us the texture as color attachment and the renderbuffer as depth attachment
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_framebuffer.handle, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_framebuffer.handle);

  //define which buffers to write. we only have one currently. creates an array with enums which all represent color attachments
  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout<<" Framebuffer was not applied. "<<std::endl;
  }

}

void ApplicationSolar::initializeSkybox() {

  //LearnOpenGL Cubemaps
/*
  uint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  pixel_data RIGHT = texture_loader::file(m_resource_path + "textures/dispair-ridge_rt.tga");
  pixel_data LEFT = texture_loader::file(m_resource_path + "textures/dispair-ridge_lf.tga");
  pixel_data TOP = texture_loader::file(m_resource_path + "textures/dispair-ridge_up.tga");
  pixel_data BOTTOM = texture_loader::file(m_resource_path + "textures/dispair-ridge_dn.tga");
  pixel_data BACK = texture_loader::file(m_resource_path + "textures/dispair-ridge_bk.tga");
  pixel_data FRONT = texture_loader::file(m_resource_path + "textures/dispair-ridge_ft.tga");

  GLsizei width = RIGHT.width;
  GLsizei height = RIGHT.height;


  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, RIGHT);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, LEFT);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, TOP);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, BOTTOM);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, BACK);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, FRONT);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
*/

}

// load models
void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL  | model::TEXCOORD);

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

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);


  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object 
  planet_object.num_elements = GLsizei(planet_model.indices.size());
}

void ApplicationSolar::initializeTexture(){

  int i = 0;
   for (auto current_node : scene_.geometry_nodes_) {

        //Texture1+i because we need inital value. Because we iterate through nodes, every planet will be assigned own texture object
        glActiveTexture(GL_TEXTURE1+i);
        pixel_data planet_texture = texture_loader::file(m_resource_path + "textures/" + current_node->get_texture_path());
        GLsizei width = planet_texture.width;
        GLsizei height = planet_texture.height;
        GLenum format = planet_texture.channels;
        GLenum type = planet_texture.channel_type;

        //TEXTURE OBJECT can be found in structs.hpp -> has handle and target
        texture_object texture;

        glGenTextures(1, &texture.handle);
        texture.target = GL_TEXTURE_2D;

        current_node->set_texture_obj(texture); /********************************important to bind here*/
        glBindTexture(GL_TEXTURE_2D, texture.handle);


        //sets wrap parameter for texture coordinate as desired T,S,R as texture coordinates
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //glTexImage2D(target, level, internalformat, widht, height, border, format, type, data)
        /*
          target = binding point for texture, usually GL_TEXTURE...
          level = number of levels of detail (0 if base image) --> if mipmap, we could show here
          internalformat: internal format of texture (get from texture file)
          width, height = dimensions of texture image if 2D
          border = thickness of border (0 for none)
          format = format of texel data should match internalformat
          type = data type of texel data
          data = pointer to texture
        */
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, planet_texture.ptr());

        i++;
      }

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
  } else if (key == GLFW_KEY_7  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    greyscale ? greyscale = false : greyscale = true; //transforms view on up key press 3 in x direction
    uploadView();
  } else if (key == GLFW_KEY_8  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    horizontalMirror ? horizontalMirror = false : horizontalMirror = true; //transforms view on up key press 3 in x direction
    uploadView();
  } else if (key == GLFW_KEY_9  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    verticalMirror ? verticalMirror = false : verticalMirror = true; //transforms view on up key press 3 in x direction
    uploadView();
  } else if (key == GLFW_KEY_0  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    blur ? blur = false : blur = true; //transforms view on up key press 3 in x direction
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
  initializeFrameBuffer(width, height);

}


// exe entry point
int main(int argc, char* argv[]) {
  Application::run<ApplicationSolar>(argc, argv, 3, 2);
}