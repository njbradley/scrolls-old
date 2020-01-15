// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "blocks.h"
using namespace glm;

#include "entity.h"
#include "blocks.h"
#include "ui.h"





/*
// Initial position : on +Z
glm::vec3 position = glm::vec3( 25, 45, 5 ); 
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
*/
float initialFoV = 85.0f;


float speed = 40.0f; // 3 units / second
float mouseSpeed = 0.005f;

void mouse_button_call(GLFWwindow*, int, int, int);
bool mouse;
int button;

class Player: public Entity {
	
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	vec3 pointing;
	World* world;
	
	public:
		bool autojump = false;
		
		Player(vec3 pos, vec3 box1, vec3 box2, World* newworld):
			Entity(pos, box1, box2), world(newworld) {
			glfwSetMouseButtonCallback(window, mouse_button_call);
		}
			
		mat4 getViewMatrix(){
			return ViewMatrix;
		}
		mat4 getProjectionMatrix(){
			return ProjectionMatrix;
		}
		
		void right_mouse() {
			//cout << "riht" << endl;
			vec3 v = position;
			double tiny = 0.0000001;
			double x = position.x;
			double y = position.y;
			double z = position.z;
			Block* target = world->raycast(&x, &y, &z, pointing.x + tiny, pointing.y + tiny, pointing.z + tiny, 10);
			if (target == nullptr) {
				return;
			}
			x -= pointing.x*0.001;
			y -= pointing.y*0.001;
			z -= pointing.z*0.001;
			Block* newblock = world->get_global((int)x - (x<0), (int)y - (y<0), (int)z - (z<0), 1);
			if (newblock == nullptr) {
				return;
			}
			while (newblock->scale != 1) {
				char val = newblock->get();
				newblock->subdivide([=] (int x, int y, int z, Pixel* pix) {
					pix->value = val;
				});
				delete newblock;
				newblock = world->get_global((int)x, (int)y, (int)z, 1);
			}
			newblock->set(2);
			//world->mark_render_update(pair<int,int>((int)position.x/world->chunksize - (position.x<0), (int)position.z/world->chunksize - (position.z<0)));
		}
		
		void left_mouse() {
			//cout << "lefftt" << endl;
			vec3 v = position;
			double tiny = 0.0000001;
			double x = position.x;
			double y = position.y;
			double z = position.z;
			Block* target = world->raycast(&x, &y, &z, pointing.x + tiny, pointing.y + tiny, pointing.z + tiny, 10);
			if (target == nullptr) {
				return;
			}
			
			while (target->scale != 1) {
				char val = target->get();
				target->subdivide([=] (int x, int y, int z, Pixel* pix) {
					pix->value = val;
				});
				delete target;
				target = world->get_global((int)x, (int)y, (int)z, 1);
			}
			target->set(0);
			//world->mark_render_update(pair<int,int>((int)position.x/world->chunksize - (position.x<0), (int)position.z/world->chunksize - (position.z<0)));
		}
		
		
		void mouse_button() {
			cout << "hello" << endl;
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				
			} else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				
			}
		}
		
		void computeMatricesFromInputs(World* nworld){
			world = nworld;
		
			// glfwGetTime is called only once, the first time this function is called
			static double lastTime = glfwGetTime();
		
			// Compute time difference between current and last frame
			double currentTime = glfwGetTime();
			float deltaTime = float(currentTime - lastTime);
		
			// Get mouse position
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
		
			// Reset mouse position for next frame
			glfwSetCursorPos(window, 1024/2, 768/2);
		
			// Compute new orientation
			angle += vec2(mouseSpeed * float(1024/2 - xpos ),  mouseSpeed * float( 768/2 - ypos ));
			//cout << horizontalAngle << ' ' << verticalAngle << endl;
			if (angle.y > 1.55f) {
				angle.y = 1.55f;
			}
			if (angle.y < -1.55) {
				angle.y = -1.55f;
			}
			if (angle.x > 6.28f) {
				angle.x = 0;
			}
			if (angle.x < 0) {
				angle.x = 6.28;
			}
			
			// Direction : Spherical coordinates to Cartesian coordinates conversion
			glm::vec3 direction(
				cos(angle.y) * sin(angle.x), 
				sin(angle.y),
				cos(angle.y) * cos(angle.x)
			);
			pointing = direction;
			// Right vector
			glm::vec3 right = glm::vec3(
				sin(angle.x - 3.14f/2.0f), 
				0,
				cos(angle.x - 3.14f/2.0f)
			);
			vec3 forward = -glm::vec3(
				-cos(angle.x - 3.14f/2.0f), 
				0,
				sin(angle.x - 3.14f/2.0f)
			);
			
			
			// Up vector
			glm::vec3 up = glm::cross( right, forward );
			
			
			float nspeed = speed;
			if (flying) {
				nspeed *= 3;
			}
			
			// Move forward
			if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
				vec3 dir_const = vec3(1,0,0)*(float)consts[0] + vec3(0,0,1)*(float)consts[2] + vec3(-1,0,0)*(float)consts[3] + vec3(0,0,-1)*(float)consts[5];
				if (length(dir_const-forward) < 0.9f and (consts[0] or consts[2] or consts[3] or consts[5]) and autojump and not consts[6]) {
					vel.y = 6;
					if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS) {
						vel.y = 10;
					}
				}
				vel += forward * deltaTime * nspeed;
			}
			// Move backward
			if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
				vel -= forward * deltaTime * nspeed;
			}
			// Strafe right
			if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
				vel += right * deltaTime * nspeed;
			}
			// Strafe left
			if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
				vel -= right * deltaTime * nspeed;
			}
			if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS){
				if (flying) {
					position += up * deltaTime * speed;
				} else if (consts[4]) {
					vel.y = 10;// = vec3(0,10,0);//up * deltaTime * speed;
				}
			}
			if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS){
				if (flying) {
					position -= up * deltaTime * speed;
				}
			}
			
			if (mouse) {
				if (button == 0) {
					left_mouse();
				} else if (button == 1) {
					right_mouse();
				}
				mouse = false;
			}
			
			
			float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.
			
			
			// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
			ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 1000.0f);
			// Camera matrix
			ViewMatrix       = glm::lookAt(
										position,           // Camera is here
										position+direction, // and looks here : at the same position, plus "direction"
										up                  // Head is up (set to 0,-1,0 to look upside-down)
								   );
		
			// For the next frame, the "last time" will be "now"
			lastTime = currentTime;
		}
		
		void render_ui(RenderVecs * uivecs) {
			///hearts
			float scale = 0.1f;
			int i;
			for (i = 0; i < health; i ++) {
				draw_image(uivecs, 2, i*scale, 1-scale, scale, scale);
			}
			for (; i < 10; i ++) {
				draw_image(uivecs, 1, i*scale, 1-scale, scale, scale);
			}
			draw_image(uivecs, 3, -0.5f, -1, 1, 0.1f*aspect_ratio);
		}
};

Player* player;

void mouse_button_call(GLFWwindow* window, int nbutton, int action, int mods) {
	if (action == GLFW_PRESS) {
		mouse = true;
		button = nbutton;
	}
}




