#include <iostream>
#include <string>
#include <cmath>
#include "VectorCamera.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>
#include <fstream>

#define PI 3.14159f


VectorCamera::VectorCamera()
{
	anglePitch = 0.f;
}

VectorCamera::~VectorCamera()
{
}


// Initialize the camera at initPosition looking in the Z- direction
// with a near plane of 0.01 and a far one of 100

void VectorCamera::init(const glm::vec3 &initPosition)
{
	position = initPosition;
	angleDirection = 180.f;
 	zNear = 0.01f;
	zFar = 100.0f;
	fov = 60.f / 180.f * PI;
	computeModelViewMatrix();
}

// Resizing the viewport may change the aspect ratio.
// The projection matrix may need updating.
	
void VectorCamera::resizeCameraViewport(int width, int height)
{
	aspectRatio = float(width) / float(height);
  	projection = glm::perspective(fov, aspectRatio, zNear, zFar);
	updateFrustum();
}

// Rotate the camera and recompute the modelview matrix.
// This takes into account rotations around the Y axis.
// "Look around"
void VectorCamera::rotateCamera(float rotation)
{
	angleDirection += rotation;
	if(angleDirection < 0.f)
		angleDirection += 360.f;
	if(angleDirection >= 360.f)
		angleDirection -= 360.f;

	computeModelViewMatrix();
}

// This one rotates the camera around the local X axis instead.
// "Look up or down"
void VectorCamera::changePitch(float rotation)
{
	anglePitch += rotation;
	if(anglePitch < -45.f)
		anglePitch = -45.f;
	if(anglePitch > 45.f)
		anglePitch = 45.f;
	computeModelViewMatrix();
}

// Move the camera forward in the direction it is looking in.

void VectorCamera::moveForward(float distance)
{
	glm::vec3 direction(sin(PI * angleDirection / 180.f), 0.f, cos(PI * angleDirection / 180.f));
	
	direction *= distance;
	position += direction;
	computeModelViewMatrix();
}

// Move the camera upward

void VectorCamera::moveUpward(float distance)
{
	position += glm::vec3(0.0f, distance, 0.0f);
	computeModelViewMatrix();
}

// Move the camera but in the direction of the local X axis (sideways).
// In FPS games this is known as strafing.

void VectorCamera::strafe(float distance)
{
	glm::vec3 direction(-cos(PI * angleDirection / 180.f), 0.f, sin(PI * angleDirection / 180.f));
	
	direction *= distance;
	position += direction;
	computeModelViewMatrix();
}

// Recompute the modelview matrix with the transformations needed to capture
// the current position and orientation of the vector camera

void VectorCamera::computeModelViewMatrix()
{
	glm::vec3 direction(sin(PI * angleDirection / 180.f), 0.f, cos(PI * angleDirection / 180.f));
	
	forward = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
    right = glm::cross(direction, up);

	modelview = glm::rotate(glm::mat4(1.f), PI * anglePitch / 180.f, glm::vec3(1.f, 0.f, 0.f));
	modelview = modelview * glm::lookAt(position, position + direction, glm::vec3(0.f, 1.f, 0.f));
	updateFrustum();
}

glm::mat4 &VectorCamera::getProjectionMatrix()
{
  return projection;
}

glm::mat4 &VectorCamera::getModelViewMatrix()
{
  return modelview;
}

// Computes the planes of the frustum in world space coordinates
void VectorCamera::updateFrustum()
{
	// x, y, z offsets for the frustum's near plane corners
	float xOffset = zNear * glm::tan(fov/2);
	float yOffset = xOffset / aspectRatio;
    float zOffset = -zNear;
	
	// Looking direction
	glm::vec3 direction(
		sin(angleDirection * PI / 180.f) * cos(anglePitch * PI / 180.f),
        sin(-anglePitch * PI / 180.f),
        cos(angleDirection * PI / 180.f) * cos(anglePitch * PI / 180.f)
	);

	// Near plane frustum corners
    glm::vec3 topLeft(-xOffset, +yOffset, zOffset);
    glm::vec3 topRight(+xOffset, +yOffset, zOffset);
    glm::vec3 bottomLeft(-xOffset, -yOffset, zOffset);
    glm::vec3 bottomRight(+xOffset, -yOffset, zOffset);

	// Directions of the planes
    glm::vec3 t = glm::normalize(glm::cross(topRight, topLeft));
    glm::vec3 b = glm::normalize(glm::cross(bottomLeft, bottomRight));
    glm::vec3 l = glm::normalize(glm::cross(topLeft, bottomLeft));
    glm::vec3 r = glm::normalize(glm::cross(bottomRight, topRight));

	// Transform the plane directions based on our camera
    glm::mat3 transform(right, glm::cross(-direction, right), -direction);

    t = transform * t;
    b = transform * b;
    l = transform * l;
    r = transform * r;

	// Frustum planes
    frustum.planes[0] = glm::vec4(t, -glm::dot(position, t));
    frustum.planes[1] = glm::vec4(b, -glm::dot(position, b));
    frustum.planes[2] = glm::vec4(l, -glm::dot(position, l));
    frustum.planes[3] = glm::vec4(r, -glm::dot(position, r));
    frustum.planes[4] = glm::vec4(-direction, -glm::dot(position + direction * zNear, -direction));
    frustum.planes[5] = glm::vec4(direction, -glm::dot(position + direction * zFar, direction));
}

// Path recording
void VectorCamera::beginRecording(const std::string &filePath, int duration)
{
    isRecording = true;
    recordingFilePath = filePath;
    recordingCheckpoints = 4 * duration;
    recordingTimeSinceLastCheckpoint = 250;
    recordingPositions.reserve(recordingCheckpoints + 1);
    recordingLookDirections.reserve(recordingCheckpoints + 1);
}

void VectorCamera::endRecording()
{
    std::ofstream fout(recordingFilePath);
    if (fout.is_open()) {
        fout << recordingPositions.size() << '\n';

        for (const auto &position : recordingPositions) {
            fout << position.x << ' ' << position.y << ' ' << position.z << '\n';
        }

        fout << '\n';

        for (const auto &direction : recordingLookDirections) {
            fout << direction.x << ' ' << direction.y << ' ' << direction.z << '\n';
        }
    }

    isRecording = false;
    recordingPositions.clear();
    recordingLookDirections.clear();
}

int VectorCamera::beginReplay(const std::string &filePath)
{
    replayingTime = 0;

    std::ifstream fin(filePath);
    if (!fin.is_open()) return 0;

    fin >> replayingCheckpoints;

    replayingPositions.resize(replayingCheckpoints);
    for (int i = 0; i < replayingCheckpoints; ++i) {
        fin >> replayingPositions[i].x >> replayingPositions[i].y >> replayingPositions[i].z;
    }

    replayingLookDirections.resize(replayingCheckpoints);
    for (int i = 0; i < replayingCheckpoints; ++i) {
        fin >> replayingLookDirections[i].x >> replayingLookDirections[i].y >> replayingLookDirections[i].z;
    }

    isReplaying = true;

    return replayingCheckpoints - 1;
}

void VectorCamera::endReplay()
{
    isReplaying = false;
    replayingPositions.clear();
    replayingLookDirections.clear();
}

