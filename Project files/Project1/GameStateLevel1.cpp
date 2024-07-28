
#include "GameStateLevel1.h"
#include "CDT.h"

#include <iostream>
#include <cmath>
#include <cstdlib>
using namespace std;

// -------------------------------------------
// Defines
// -------------------------------------------

#define MESH_MAX					32				// The total number of Mesh (Shape)
#define TEXTURE_MAX					33				// The total number of texture
#define GAME_OBJ_INST_MAX			1024			// The total number of different game object instance
#define NUM_FURNITURE				1024

	
enum GAMEOBJ_TYPE
{
	// list of game object types
	TYPE_FURNITURE = 0,
	TYPE_BACKGROUND = 28,
	TYPE_PANEL, //29
	TYPE_SCROLL, //30
	TYPE_PANELTOP, //31
	TYPE_ORIENTATION //32
	
};

#define FLAG_INACTIVE		0
#define FLAG_ACTIVE			1



// -------------------------------------------
// Structure definitions
// -------------------------------------------

struct GameObj
{
	CDTMesh*		mesh;
	CDTTex*			tex;
	int				type;				// enum GAMEOBJ_TYPE
	int				flag;				// 0 - inactive, 1 - active
	glm::vec3		position;			// usually we will use only x and y
	glm::vec3		velocity;			// NOT NEEDED
	glm::vec3		scale;				// usually we will use only x and y
	float			orientation;		// 0 radians is 3 o'clock, PI/2 radian is 12 o'clock
	glm::mat4		modelMatrix;
	int				selected;
};


// -------------------------------------------
// Level variable, static - visible only in this file
// -------------------------------------------

static CDTMesh		sMeshArray[MESH_MAX];							// Store all unique shape/mesh in your game
static int			sNumMesh;
static CDTTex		sTexArray[TEXTURE_MAX];							// Corresponding texture of the mesh
static int			sNumTex;
static GameObj		sGameObjInstArray[GAME_OBJ_INST_MAX];			// Store all game object instance that arent furniture
static int			sNumGameObj;

bool				isDragging = false;
bool				itemSelected = false;
int					selectedItemIndex = -1;

double				xpos = 0.0f;
double				ypos = 0.0f;

int					scrollCount = 0; //keep track of position in panel (range of objects to instantiate)

int					upperRange = 0;
int					lowerRange = 7;

// functions to create/destroy a game object instance
static GameObj*		gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient, int textureIndex, int meshIndex);
static void			gameObjInstDestroy(GameObj &pInst);
void  				scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void				mouseClick(GLFWwindow* window, int button, int action, int mods);




// -------------------------------------------
// Game object instant functions
// -------------------------------------------

GameObj* gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient, int textureIndex, int meshIndex)
{
	// loop through all object instance array to find the free slot

		for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
			GameObj* pInst = sGameObjInstArray + i;

			if (pInst->flag == FLAG_INACTIVE) {

				pInst->mesh = sMeshArray + meshIndex;
				pInst->tex = sTexArray + textureIndex;
				pInst->type = type;
				pInst->flag = FLAG_ACTIVE;
				pInst->position = pos;
				pInst->velocity = vel;
				pInst->scale = scale;
				pInst->orientation = orient;
				pInst->modelMatrix = glm::mat4(1.0f);
				pInst->selected = FLAG_INACTIVE;

				sNumGameObj++;
				return pInst;
			}
		}
	

	// Cannot find empty slot => return 0
	return NULL;
}


void gameObjInstDestroy(GameObj &pInst)
{
	// Lazy deletion, not really delete the object, just set it as inactive
	if (pInst.flag == FLAG_INACTIVE)
		return;

	sNumGameObj--;

	pInst.flag = FLAG_INACTIVE;
}


// -------------------------------------------
// Game states function
// -------------------------------------------

void GameStateLevel1Load(void){

	// clear the Mesh array
	memset(sMeshArray, 0, sizeof(CDTMesh) * MESH_MAX);
	sNumMesh = 0;

	//+ clear the Texture array

	memset(sTexArray, 0, sizeof(CDTTex) * TEXTURE_MAX);
	sNumTex = 0;

	//+ clear the game object instance array
	memset(sGameObjInstArray, 0, sizeof(GameObj) * GAME_OBJ_INST_MAX);
	sNumGameObj = 0;


	// --------------------------------------------------------------------------
	// Create all of the unique meshes/textures and put them in MeshArray/TexArray
	//		- The order of mesh should follow enum GAMEOBJ_TYPE 
	/// --------------------------------------------------------------------------

	// Temporary variable for creating mesh
	CDTMesh* pMesh;
	CDTTex* pTex;
	std::vector<CDTVertex> vertices;
	CDTVertex v1, v2, v3, v4;

	// Create Ship mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("wall.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("roomframe.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("tile1.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("tile2.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("bigbedGrey.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("bigbedBrown.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("bigbedBlue.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("smallBedBlue.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("smallBedBrown.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("smallBedGrey.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("armchair.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("sofa2.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("sofa3.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("sofagrey.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("sofablue.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("sofabrown.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("bench.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("diningTable1.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("diningTable2.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("diningTable3.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("desk.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("desk2.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("cabinetTV.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("plant.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("singlePlant.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("carpet1.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("carpet2.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("carpet3.png");
	//TYPE BACKGROUND

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("gridbackground.png");

	//TYPE PANEL

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("panel.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("scroll.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("panelTop.png");

	pTex = sTexArray + sNumTex++;
	*pTex = TextureLoad("orientation.png");


	printf("Level1: Load\n");
}


void	scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

	scrollCount = yoffset;
	cout << scrollCount << endl;
}



void mouseClick(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {

		glfwGetCursorPos(window, &xpos, &ypos);
		isDragging = true;
	}
	else if(button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {

		isDragging = false;
		itemSelected = false;
		selectedItemIndex = -1;

	}
}


void GameStateLevel1Init(void){

	gameObjInstCreate(TYPE_BACKGROUND, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(GetWindowWidth(), GetWindowHeight(), 1.0f), 0.0f, 28, 0);
	gameObjInstCreate(TYPE_PANEL, glm::vec3(-620.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(400.0f, 900.0f, 1.0f), 0.0f, 29, 0);
	
	//instatiate furniture selection panel

	float row = 325.0f;
	float col1 = -730.0f;
	float col2 = -570.0f;

	int count = 0;

	while (count <= 26) {
		gameObjInstCreate(TYPE_FURNITURE, glm::vec3(col1, row, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(100.0f, 100.0f, 1.0f), 0.0f, count, 0);
		gameObjInstCreate(TYPE_FURNITURE, glm::vec3(col2, row, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(100.0f, 100.0f, 1.0f), 0.0f, count+1, 0);
		count += 2;
		row -= 165;
	}
	
	gameObjInstCreate(TYPE_ORIENTATION, glm::vec3(770.0f, 210.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(122.0f, 550.0f, 1.0f), 0.0f, 32, 0);
	gameObjInstCreate(TYPE_PANELTOP, glm::vec3(-620.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(460.0f, 1000.0f, 1.0f), 0.0f, 31, 0);
	gameObjInstCreate(TYPE_SCROLL, glm::vec3(-440.0f, 330.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(14.0f, 160.0f, 1.0f), 0.0f, 30, 0);


	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouseClick);

	printf("Level1: Init\n");
}



void GameStateLevel1Update(double dt, long frame, int &state){

	//SCROLL PANEL

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		if (pInst->type == TYPE_SCROLL) {

			while (pInst->position.y <= 330.0f && pInst->position.y >= -325.0f) {
				pInst->position.y += scrollCount * 19;

				for (int i = 0; i <= 29; i++) {
					GameObj* pInst2 = sGameObjInstArray + i;

					if (pInst->position.y >= 330.0f || pInst->position.y <= -325.0f) {
						break; // Stop scrolling TYPE_FURNITURE objects
					}

					if (pInst2->type == TYPE_FURNITURE) {
						pInst2->position.y -= scrollCount * 45;
					}
				}
				scrollCount = 0;
				break;
			}
			
			if (pInst->position.y > 330.0f) {
				pInst->position.y = 330.0f;
			}
			else if (pInst->position.y < -325.0f) {
				pInst->position.y = -325.0f;
			}

		}
	}


	glfwGetCursorPos(window, &xpos, &ypos);

	//mouse position to world space

	xpos -= GetWindowWidth() / 2;
	ypos -= GetWindowHeight() / 2;
	ypos *= -1;

	// Cam zoom UI, for Debugging
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS){
		ZoomIn(0.1f);
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){
		ZoomOut(0.1f);
	}


	//-----------------------------------------
	// Check for collsion when mouse clicked and WASD keys to change orientation
	//-----------------------------------------


	if (isDragging) {

		if (!itemSelected) {

			for (int i = GAME_OBJ_INST_MAX; i >= 0; i--) {
				GameObj* pInst = sGameObjInstArray + i;

				// skip inactive object
				if (pInst->flag == FLAG_INACTIVE) {
					continue;
				}

				if (pInst->type == TYPE_FURNITURE) {

					bool collide = true;

					//+ Check for collsion

					float distanceX = glm::abs(xpos - pInst->position.x);
					float distanceY = glm::abs(ypos - pInst->position.y);


					if (distanceX > (pInst->scale.x / 2) || distanceY > (pInst->scale.y / 2)) {
						pInst->selected = FLAG_INACTIVE;
						collide = false;
					}


					if (collide) {

						if (i < 30) {
							GameObj* pNewInst = gameObjInstCreate(TYPE_FURNITURE, pInst->position, pInst->velocity, pInst->scale, pInst->orientation, i-2, 0.0f);
							pNewInst->selected = FLAG_ACTIVE;
							itemSelected = true;
							break;
						}
						else {
							pInst->selected = FLAG_ACTIVE;
							itemSelected = true;
							printf("detected collision\n");
							break;
						}
					}
				}

			}
		}
		else {

			int selectedItemIndex = -1;

			for (int i = GAME_OBJ_INST_MAX; i >= 0; i--) {
				GameObj* pInst = sGameObjInstArray + i;

				if (pInst->type == TYPE_FURNITURE) {
					if (pInst->selected == FLAG_ACTIVE) {

						if (selectedItemIndex == -1) {
							selectedItemIndex = i;
						}
						else {
							// If there is more than one selected object, deselect them all
							pInst->selected = FLAG_INACTIVE;
						}
					}
				}
				
			}

			if (selectedItemIndex != -1) {
			
				//UPDATE POSITION TO MOUSE POSITION
				
				GameObj* pInst = sGameObjInstArray + selectedItemIndex;
				pInst->position.x = xpos;
				pInst->position.y = ypos;

				//CHANGE ORIENTATION

				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
					pInst->orientation = 0.0f;
				}

				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
					pInst->orientation = PI;
				}

				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
					pInst->orientation = PI/2;
				}

				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
					pInst->orientation = -PI/2;
				}

				if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
					pInst->scale *= 1.02;
				}

				if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
					pInst->scale *= 0.98;
				}

				if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
					gameObjInstDestroy(*pInst);
					selectedItemIndex = -1;
				}
			}
		}

	}


	//-----------------------------------------
	// Update modelMatrix of all game obj
	//-----------------------------------------
	
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++){
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		glm::mat4 rMat = glm::mat4(1.0f);
		glm::mat4 sMat = glm::mat4(1.0f);
		glm::mat4 tMat = glm::mat4(1.0f);

		// Compute the scaling matrix
		sMat = glm::scale(glm::mat4(1.0f), pInst->scale);

		//+ Compute the rotation matrix, we should rotate around z axis 
		rMat = glm::rotate(glm::mat4(1.0f), pInst->orientation, glm::vec3(0.0f, 0.0f, 1.0f));

		//+ Compute the translation matrix
		tMat = glm::translate(glm::mat4(1.0f), pInst->position);

		// Concatenate the 3 matrix to from Model Matrix
		pInst->modelMatrix = tMat * sMat * rMat;
	}

}

void GameStateLevel1Draw(void){

	// Clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw all game object instance in the sGameObjInstArray
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++){
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		// 4 steps to draw sprites on the screen
		//	1. SetRenderMode()
		//	2. SetTexture()
		//	3. SetTransform()
		//	4. DrawMesh()

		SetRenderMode(CDT_TEXTURE, 1.0f);
		SetTexture(*pInst->tex, 0.0f, 0.0f);
		SetTransform(pInst->modelMatrix);
		DrawMesh(*pInst->mesh);
	}


	// Swap the buffer, to present the drawing
	glfwSwapBuffers(window);
}

void GameStateLevel1Free(void){

	//+ call gameObjInstDestroy for all object instances in the sGameObjInstArray & furnitureInstArray
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObj* pInst = sGameObjInstArray + i;
		pInst->flag = FLAG_INACTIVE;
	}


	// reset camera
	ResetCam();

	printf("Level1: Free\n");
}

void GameStateLevel1Unload(void){

	// Unload all meshes in MeshArray
	for (int i = 0; i < sNumMesh; i++){
		UnloadMesh(sMeshArray[i]);
	}

	printf("Level1: Unload\n");
}
