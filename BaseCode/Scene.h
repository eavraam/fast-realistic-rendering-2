#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "VectorCamera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "PLYReader.h"
#include "Query.h"
#include "QueryPool.h"
#include "QuadTree.h"

#include <queue>
#include <stack>
#include <utility>
#include <unordered_set>

// Scene contains all the entities of our game.
// It is responsible for updating and render them.
class Scene
{

public:
	Scene();
	~Scene();

	void init();
	bool loadMesh(const char *filename);
	void update(int deltaTime);
	void render();

  	VectorCamera &getCamera();
	float sceneFps;

	// Calculate and reset the instance AABBs
    void calculateInstanceAABB(AABB& aabb, int i);
    void resetInstanceAABB(AABB& aabb, int i);

private:
	// General functions
	void initShaders();
	void computeModelViewMatrix();
	void renderRoom();

	// Debugging help
	void renderAABBCube(const glm::vec3& minPoint, const glm::vec3& maxPoint);
	void renderAABBCubeOccluded(const glm::vec3& minPoint, const glm::vec3& maxPoint);

	// Rendering
	// -----------------------------------------
	// // Frustum culling
	bool isAABBInsideFrustum(const AABB& aabb);
	// // Techniques
	void renderOnlyAABB();
	void renderDefault();
	void renderOcclusionCulling();
	void renderCHC();

	// CHC helper functions
	//
	//

private:
	// General
	VectorCamera camera;
	TriangleMesh *cube, *mesh;
	ShaderProgram basicProgram;
	ShaderProgram gouraudProgram;
	float currentTime;
	unsigned int currentFrame;
	AABB meshAABB;

	// Rendering matrices
	glm::mat3 normalMatrix;
	glm::mat4 modelview;
	glm::mat4 model;

	// AABB Cube params
	glm::vec3 center;
	glm::vec3 scale;
	glm::mat4 modelCube;

	// Numerical input for model irregular grid display
	int renderedModels;
  	int modelCopies = 252;
	// !! Important: Set all to 3 * modelCopies. Here, modelCopies=252 , so all: [756] !!
	float positions			[756];
	float rotationAxis		[756];
	float colors			[756];
	float rotationAngle 		[756];

	// Scene rendering data
    bool viewFrustumCulling;
	bool isAABBRendered;
	bool isOcclusionCulled;
	
	vector<Query> queries;
	vector<GLuint> queryIdx;
	bool firstTimeQueries;
	vector<QueryPool> qp;
	//QueryPool queryPool;

	// CHC helper elements
    // ...
	// ...

	// For the rendering mode radio button of the UI
	int renderingMode;
	enum renderingTechnique
	{
		DEFAULT,
		OCCLUSION_CULLING,
		ONLY_AABB,
		CHC
	};

	// For the rendering shader radio button of the UI
	int shaderMode;
	enum shadingTechnique
	{
		PHONG,
		GOURAUD
	};

	// Hard-coded grid layout
	int grid_layout[12] =
	{
		0, 0, 0, 0,
		0, 1, 0, 1,
		2, 1, 2, 1
	};
	// grid_layout array length + counter
	int grid_length = sizeof(grid_layout) / sizeof(grid_layout[0]);
	int grid_line_counter = 0;
};


#endif // _SCENE_INCLUDE

