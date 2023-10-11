#ifndef _VECTOR_CAMERA_INCLUDE
#define _VECTOR_CAMERA_INCLUDE

#include <glm/glm.hpp>

#include <array>
#include <string>
#include <vector>


// View Frustum
struct Frustum
{
    std::array<glm::vec4, 6> planes;
};

// VectorCamera contains the properies of the camera the scene is using
// It is responsible for computing the associated GL matrices
class VectorCamera
{

public:
	VectorCamera();
	~VectorCamera();
	
	// Functions
	void init(const glm::vec3 &initPosition);
	
	void resizeCameraViewport(int width, int height);
	void rotateCamera(float rotation);
	void changePitch(float rotation);
	void moveForward(float distance);
	void moveUpward(float distance);
	void strafe(float distance);

	glm::mat4 &getProjectionMatrix();
	glm::mat4 &getModelViewMatrix();

	const Frustum &getFrustum() const {return frustum;}

	// Recording
	void beginRecording(const std::string &filePath, int duration);
    void endRecording();
    int beginReplay(const std::string &filePath);
    void endReplay();

	void recordCameraPose(const glm::vec3& position, const glm::vec3& forward);
	void exportCameraPath(const std::string& filePath);
	
	// Variables
	Frustum frustum;

private:
  void computeModelViewMatrix();
  void updateFrustum();

private:
	// Projection params
	float fov;
	float zNear;
	float zFar;
	float aspectRatio;

	// Directions
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	// Camera parameters
	glm::vec3 position;
	float angleDirection, anglePitch;
	glm::mat4 projection, modelview;	// OpenGL matrices

	// Path recording
	bool isRecording;
	int recordingCheckpoints;
    int recordingTimeSinceLastCheckpoint;
	std::string recordingFilePath;
	std::vector<glm::vec3> recordingPositions;
    std::vector<glm::vec3> recordingLookDirections;

	// Path playback
    bool isReplaying;
    int replayingCheckpoints;
    int replayingTime;
    std::vector<glm::vec3> replayingPositions;
    std::vector<glm::vec3> replayingLookDirections;

};


#endif // _VECTOR_CAMERA_INCLUDE

