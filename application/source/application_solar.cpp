#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

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

#include <math.h>	// log function _ ass1

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
{
  initializeGeometry();
  initializeShaderPrograms();
}

void ApplicationSolar::render() const {
  // bind shader to upload uniforms
  glUseProgram(m_shaders.at("planet").handle);

  // iterate over all planets to draw _ ass1
  for (int i = 0; i < 10; i++) {

	  // to be dealt together with 'Earth' 
	  if (planets[i].name == "Moon")
		  continue;

	  // calculates and uploads the Model & Normal Matrix
	  upload_planet_transforms(planets[i]);
  }
}

// calculates and uploads the Model & Normal Matrix _ ass1
void ApplicationSolar::upload_planet_transforms(planet aPlanet) const {
	
	// scale the planet values to fit in the screen
	double planetSize = log(aPlanet.size); planetSize = planetSize * planetSize / 200;
	float planetDist = float(log(aPlanet.dist)); planetDist = planetDist * planetDist / 2 - 5;
	float planetSpeed = float(log(aPlanet.speed) / 3);

	// preventing the values of the sun to be infinity
	if (aPlanet.name == "Sun") {
		planetDist = 0.0f;
		planetSpeed = 0.0f;
	}

	// get model matrix
	glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime() * planetSpeed), glm::fvec3{0.0f, 1.0f, 0.0f});
	model_matrix = glm::translate(model_matrix, glm::fvec3{ 0.0f, 0.0f, planetDist});
	model_matrix = glm::scale(model_matrix, glm::fvec3{ planetSize, planetSize, planetSize });
	glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
	                   1, GL_FALSE, glm::value_ptr(model_matrix));

	//extra matrix for normal transformation to keep them orthogonal to surface
	glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
	glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
		1, GL_FALSE, glm::value_ptr(normal_matrix));

	// bind the VAO to draw
	glBindVertexArray(planet_object.vertex_AO);

	// draw bound vertex array using bound shader
	glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

	// add 'Moon' in case of 'Earth'
	if (aPlanet.name == "Earth") {

		// scale the moon values
		float moonSpeed = float(planets[4].speed *2);
		double moonSize = log(planets[4].size) / 20;
		float moonDist = float(planets[4].dist * 5);

		// get model matrix
		glm::fmat4 moon_matrix = glm::rotate(model_matrix, float(glfwGetTime() * -moonSpeed), glm::fvec3{ 0.0f, 1.0f, 0.0f });
		moon_matrix = glm::translate(moon_matrix, glm::fvec3{ 0.0f, 0.0f, moonDist });
		moon_matrix = glm::scale(moon_matrix, glm::fvec3{ moonSize, moonSize, moonSize });
		glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
			1, GL_FALSE, glm::value_ptr(moon_matrix));
		
		//extra matrix for normal transformation to keep them orthogonal to surface
		glm::fmat4 moon_normal = glm::inverseTranspose(glm::inverse(m_view_transform) * moon_matrix);
		glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
			1, GL_FALSE, glm::value_ptr(moon_normal));

		// bind the VAO to draw
		glBindVertexArray(planet_object.vertex_AO);

		// draw bound vertex array using bound shader
		glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
	}
}

void ApplicationSolar::updateView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();
  
  // bind new shader
  glUseProgram(m_shaders.at("planet").handle);
  
  updateView();
  updateProjection();
}

// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods) {
  
	//wider
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
    updateView();
  }	// smaller
  else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
    updateView();
  }	// left  
  else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
	  m_view_transform = glm::translate(m_view_transform, glm::fvec3{-1.0f, 0.0f, 0.0f });
	  updateView();
  }	// right
  else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
	  m_view_transform = glm::translate(m_view_transform, glm::fvec3{ 1.0f, 0.0f, 0.0f });
	  updateView();
  }	// down
  else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
	  m_view_transform = glm::translate(m_view_transform, glm::fvec3{ 0.0f, -1.0f, 0.0f });
	  updateView();
  }	// up
  else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
	  m_view_transform = glm::translate(m_view_transform, glm::fvec3{ 0.0f, 1.0f, 0.0f });
	  updateView();
  }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling
}

// load shader programs
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/simple.vert",
                                           m_resource_path + "shaders/simple.frag"});
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

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}