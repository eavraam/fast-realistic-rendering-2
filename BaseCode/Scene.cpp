#include <iostream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glut.h"
#include "backends/imgui_impl_opengl3.h"

#include <fstream>
#include <string>
#include <random>

#include "Scene.h"


Scene::Scene()
{
	cube = NULL;
	mesh = NULL;
}

Scene::~Scene()
{
	if(cube != NULL)
		delete cube;
	if(mesh != NULL)
		delete mesh;
}

// Get the camera
VectorCamera& Scene::getCamera()
{
  return camera;
}

// Function to generate a random float value between min and max
float getRandomFloat(float min, float max) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}


// Initialize the scene. This includes the cube we will use to render
// the floor and walls, as well as the camera.
void Scene::init()
{
	// Init the rendering booleans
	viewFrustumCulling  = false;
	isAABBRendered 		= false;
	renderingMode 		= false;
	shaderMode			= false;
	firstTimeQueries 	= true;

	// For some reason, I can't initialize QueryPool queryPool in Scene.h
	// so I push it back inside a vector
	QueryPool queryPool;
	queryPool = QueryPool(modelCopies);
    queryPool.clear();
	qp.push_back(queryPool);
	
	// Init the Shaders
	initShaders();

	// Init current time
	currentTime = 0.0f;
	currentFrame = 0;

	// Load my mesh
	loadMesh("../models/bunny.ply");
	
	camera.init(glm::vec3(0.0f, 2.5f, 17.0f));

	if (mesh != NULL)
	{
		for (int i = 0; i < modelCopies; i++)
		{
			// Generate random position
			positions[i*3]   = getRandomFloat(-4.0f, 4.0f);
			positions[i*3+1] = 0.0f;
			positions[i*3+2] = getRandomFloat(-2.0f, 8.0f);

			// Generate random rotation
			rotationAxis[i*3]   = getRandomFloat(0.0f, 1.0f);
			rotationAxis[i*3+1] = getRandomFloat(0.0f, 1.0f);
			rotationAxis[i*3+2] = getRandomFloat(0.0f, 1.0f);
			rotationAngle[i*3]  = getRandomFloat(0.0f, 360.f);

			// Generate random color
			colors[i*3]   = getRandomFloat(0.0f, 1.0f);
			colors[i*3+1] = getRandomFloat(0.0f, 1.0f);
			colors[i*3+2] = getRandomFloat(0.0f, 1.0f);
		}
	}
}

// Loads the mesh into CPU memory and sends it to GPU memory (using GL)
bool Scene::loadMesh(const char *filename)
{
#pragma warning( push )
#pragma warning( disable : 4101)
	PLYReader reader;
#pragma warning( pop ) 

	if(mesh != NULL)
	{
		mesh->free();
		delete mesh;
	}
	mesh = new TriangleMesh();
	bool bSuccess = reader.readMesh(filename, *mesh);
	if (bSuccess)
	{
		meshAABB = mesh->getAABB();
		mesh->sendToOpenGL(basicProgram);
		mesh->sendToOpenGL(gouraudProgram);

		cube = new TriangleMesh();
		cube->buildCube();
		cube->sendToOpenGL(basicProgram);
		cube->sendToOpenGL(gouraudProgram);
	}
	else cout << "Couldn't load mesh " << filename << endl;
	
	return bSuccess;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
}

// Render the scene.
void Scene::render()
{
	// ImGui UI window
	// Set the next window position using normalized coordinates
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(280, 460), ImGuiCond_Always);
	if (ImGui::Begin("Settings")) {
		ImGui::Text("Key Commands:");
        ImGui::Text("F1: Toggle application/computer focus");
		ImGui::Text("F5: Fullscreen");
		ImGui::Text("Navigation: WASD/arrows");
        ImGui::Separator();
		ImGui::Text("Frustum Culling");
        ImGui::Checkbox("Enable/Disable Frustum Culling", &viewFrustumCulling);
        ImGui::Separator();
        ImGui::Text("Rendering Technique");
		ImGui::RadioButton("Default/Simple Rendering", &renderingMode, DEFAULT);
		ImGui::RadioButton("Occlusion Culling Rendering", &renderingMode, OCCLUSION_CULLING);
		ImGui::RadioButton("Only AABB Rendering", &renderingMode, ONLY_AABB);
		//ImGui::RadioButton("CHC Rendering", &renderingMode, CHC); // Under construction...
		ImGui::Separator();
        ImGui::Text("Debugging Help / AABB Rendering");
		ImGui::Checkbox("Enable/Disable rendered AABB", &isAABBRendered);
		ImGui::Checkbox("Enable/Disable culled AABB", &isOcclusionCulled);
		ImGui::Separator();
		ImGui::Text("Rendering Shader");
		ImGui::RadioButton("Phong Shader", &shaderMode, PHONG);
		ImGui::RadioButton("Gouraud Shader", &shaderMode, GOURAUD);
		ImGui::Separator();
        ImGui::Text("Performance");
		ImGui::Text("Total models: %d", modelCopies);
		ImGui::Text("Rendered models: %d", renderedModels);
		ImGui::Text("%g fps", sceneFps);
        
    }
    ImGui::End();

	// Mesh rendering
	if(mesh != NULL)
	{

		switch (renderingMode)
		{
		case (DEFAULT):
			// Render the mesh using the Default way
			renderDefault();
			break;
		case (OCCLUSION_CULLING):
			// Render the mesh using the Stop and Wait Query Technique
			renderOcclusionCulling();
			break;
		case (ONLY_AABB):
			// Render the mesh using the Default way
			renderOnlyAABB();
			break;
		default:
			break;
		}	
	}
}

// CHC Renderer
void Scene::renderOnlyAABB()
{
	// Rendering loop
	for (int i=0; i < modelCopies; i++)
	{
		// Adjust the instance's AABB to the mesh AABB
		calculateInstanceAABB(meshAABB, i);
		
		// Check if the model's AABB is inside the view frustum
		// If not, reset the instance AABB and go to the next one
		if (viewFrustumCulling && !isAABBInsideFrustum(meshAABB))
		{
			// Reset the meshAABB changes for the next model after this iteration break. 
			resetInstanceAABB(meshAABB, i);

			continue;
		}

		// AABB rendering
		renderAABBCube(meshAABB.min, meshAABB.max);

		// Reset the meshAABB changes for the next model
		// after properly rendering the model.
		resetInstanceAABB(meshAABB, i);
		
	}
}

// Default Renderer
void Scene::renderDefault()
{
	// Clear the previously rendered model counter
	renderedModels = 0;

	// Rendering loop
	for (int i=0; i < modelCopies; i++)
	{
		// Adjust the instance's AABB to the mesh AABB
		calculateInstanceAABB(meshAABB, i);
		
		// Check if the model's AABB is inside the view frustum
		// If not, reset the instance AABB and go to the next one
		if (viewFrustumCulling && !isAABBInsideFrustum(meshAABB))
		{
			// Reset the meshAABB changes for the next model after this iteration break. 
			resetInstanceAABB(meshAABB, i);

			continue;
		}

		// Toggle the AABB rendering
		if (isAABBRendered)
		{
			// Render the AABB
			renderAABBCube(meshAABB.min, meshAABB.max);
		}

		// Render the mesh
		// Compute  model matrix and i-related parameters (i.e., color)
		model = glm::translate(glm::mat4(1.0), glm::vec3(positions[i*3], positions[i*3+1], positions[i*3+2]));
		modelview = camera.getModelViewMatrix() * model;
		normalMatrix = glm::inverseTranspose(camera.getModelViewMatrix());

		// Select rendering shader
		switch (shaderMode)
		{
			case (PHONG):
				basicProgram.use();
				basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
				basicProgram.setUniform4f("color", colors[i*3], colors[i*3+1], colors[i*3+2], 1.0f);
				basicProgram.setUniformMatrix4f("modelview", modelview);
				basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
				mesh->render();
				break;
			case (GOURAUD):
				gouraudProgram.use();
				gouraudProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
				gouraudProgram.setUniformMatrix4f("modelview", modelview);
				gouraudProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
				mesh->render();
				break;
			default:
				break;
		}

		// Update the rendered model counter
		renderedModels++;

		// Reset the meshAABB changes for the next model
		// after properly rendering the model.
		resetInstanceAABB(meshAABB, i);
		
	}
}

// Occlusion Culling Renderer
void Scene::renderOcclusionCulling()
{

	// Clear the previously rendered model counter
	renderedModels = 0;

	// Initialize the queries
	// Below is a workaround of not being allowed to declare QueryPool queryPool in Scene.h
	// so I declare it in Scene::init and pass it through this workaround
	QueryPool qpStopAndWait;
	qpStopAndWait = qp[0];

    Query query = qpStopAndWait.getQuery();

    for (int i = 0; i < modelCopies; ++i) {
        
		// Adjust the instance's AABB to the mesh AABB
		calculateInstanceAABB(meshAABB, i);
		
		// Check if the model's AABB is inside the view frustum
		// If not, reset the instance AABB and go to the next one
		if (viewFrustumCulling && !isAABBInsideFrustum(meshAABB))
		{
			// Reset the meshAABB changes for the next model after this iteration break. 
			resetInstanceAABB(meshAABB, i);

			continue;
		}
		
		// Occlusion Querying
		query.begin();
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		// Render the AABB
		renderAABBCube(meshAABB.min, meshAABB.max);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		query.end();

		// Render if we 've got the query result
		if (query.isVisible() > 0) {

			// Toggle the AABB rendering of rendered meshes
			if (isAABBRendered)
			{
				// Render the AABB
				renderAABBCube(meshAABB.min, meshAABB.max);
			}

			// Render the mesh
			// Compute  model matrix and i-related parameters (i.e., color)
			model = glm::translate(glm::mat4(1.0), glm::vec3(positions[i*3], positions[i*3+1], positions[i*3+2]));
			modelview = camera.getModelViewMatrix() * model;
			normalMatrix = glm::inverseTranspose(camera.getModelViewMatrix());
			
			// Select rendering shader
			switch (shaderMode)
			{
				case (PHONG):
					basicProgram.use();
					basicProgram.setUniform4f("color", colors[i*3], colors[i*3+1], colors[i*3+2], 1.0f);
					basicProgram.setUniformMatrix4f("modelview", modelview);
					basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
					mesh->render();
					break;
				case (GOURAUD):
					gouraudProgram.use();
					gouraudProgram.setUniformMatrix4f("modelview", modelview);
					gouraudProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
					mesh->render();
					break;
				default:
					break;
			}

			// Update the rendered model counter
			renderedModels++;
		}
		else
		{
			if(isOcclusionCulled)
				renderAABBCubeOccluded(meshAABB.min, meshAABB.max);
		}
        
		// Reset the meshAABB changes for the next model after this iteration break. 
		resetInstanceAABB(meshAABB, i);
    }
}


// CHC Renderer
void Scene::renderCHC()
{

}
           

// Calculate the AABB for each model
void Scene::calculateInstanceAABB(AABB& aabb, int i)
{
    aabb.min.x += positions[i*3];
	aabb.min.y += positions[i*3 + 1];
	aabb.min.z += positions[i*3 + 2];

	aabb.max.x += positions[i*3];
	aabb.max.y += positions[i*3 + 1];
	aabb.max.z += positions[i*3 + 2];
}

// Reset the AABB for each model
void Scene::resetInstanceAABB(AABB& aabb, int i)
{
	aabb.min.x -= positions[i*3];
	aabb.min.y -= positions[i*3 + 1];
	aabb.min.z -= positions[i*3 + 2];

	aabb.max.x -= positions[i*3];
	aabb.max.y -= positions[i*3 + 1];
	aabb.max.z -= positions[i*3 + 2];
}

// Helper function to render the AABB cube
void Scene::renderAABBCube(const glm::vec3& minPoint, const glm::vec3& maxPoint)
{
    // Calculate the center and size of the AABB
    center = (minPoint + maxPoint) * 0.5f;
    scale = maxPoint - minPoint;

	// Calculate the modelview matrix
	modelCube = glm::mat4(1.0f);
	modelCube = glm::translate(modelCube, center);
	modelCube = glm::scale(modelCube, scale);
    //glm::mat4 modelCube = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);
	modelview = camera.getModelViewMatrix() * modelCube;
    
	// Rendering as wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    basicProgram.setUniform4f("color", 0.0f, 1.0f, 0.0f, 1.0f);		// Set the color to green
    basicProgram.setUniformMatrix4f("modelview", modelview);
    cube->render();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Helper function to render the AABB cube
void Scene::renderAABBCubeOccluded(const glm::vec3& minPoint, const glm::vec3& maxPoint)
{
    // Calculate the center and size of the AABB
    center = (minPoint + maxPoint) * 0.5f;
    scale = maxPoint - minPoint;

	// Calculate the modelview matrix
	modelCube = glm::mat4(1.0f);
	modelCube = glm::translate(modelCube, center);
	modelCube = glm::scale(modelCube, scale * glm::vec3(1.0f, 7.0f, 1.0f));
    //glm::mat4 modelCube = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);
	modelview = camera.getModelViewMatrix() * modelCube;
    
	// Rendering as wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    basicProgram.setUniform4f("color", 0.8f, 0.8f, 0.0f, 1.0f);
    basicProgram.setUniformMatrix4f("modelview", modelview);
    cube->render();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// View Frustum 
bool Scene::isAABBInsideFrustum(const AABB& aabb)
{
	// Fetch tha camera's frustum planes
	const Frustum& frustum = camera.getFrustum();

	// Iterate through each frustum plane
    for (const glm::vec4& frustumPlane : frustum.planes) {

		// Intersection-check boolean
        bool intersect = false;

        // Test AABB against each face of the frustum
        for (int i = 0; i < 6; ++i) {

			// Extract the x, y, z values from frustumPlane, representing the plane's normal
            glm::vec3 normal(frustumPlane);

            // Determine the closest AABB corner to the frustum plane
            glm::vec3 closestPoint;

            closestPoint.x = (normal.x >= 0.0f) ? aabb.min.x : aabb.max.x;
            closestPoint.y = (normal.y >= 0.0f) ? aabb.min.y : aabb.max.y;
            closestPoint.z = (normal.z >= 0.0f) ? aabb.min.z : aabb.max.z;

            // Compute the signed distance from the closest AABB corner to the frustum plane
            float signedDistance = glm::dot(closestPoint, normal) + frustumPlane.w;

            // If the signed distance is negative, the AABB is outside the frustum
            if (signedDistance < 0.0f) {
                intersect = true;
                break;
            }
        }

        // If the AABB does not intersect with any frustum face, it is outside the frustum
        if (!intersect) {
            return false;
        }
    }

    return true;
}

// Load, compile, and link the vertex and fragment shader
void Scene::initShaders()
{
	Shader vShader, fShader;

	// Setup the basic shader
	vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	basicProgram.init();
	basicProgram.addShader(vShader);
	basicProgram.addShader(fShader);
	basicProgram.link();
	if(!basicProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << basicProgram.log() << endl << endl;
	}
	basicProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();

	// Setup the Gouraud shader
	vShader.initFromFile(VERTEX_SHADER, "shaders/gouraud.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/gouraud.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	gouraudProgram.init();
	gouraudProgram.addShader(vShader);
	gouraudProgram.addShader(fShader);
	gouraudProgram.link();
	if(!gouraudProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << gouraudProgram.log() << endl << endl;
	}
	gouraudProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}
