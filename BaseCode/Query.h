#ifndef _QUERY_INCLUDE
#define _QUERY_INCLUDE

#include <GL/glew.h>
#include <GL/gl.h>

class Query
{
public:
    Query(GLuint id);
    void begin() const;
    void end() const;
    bool isVisible() const;
    bool resultIsReady() const;
private:
    GLuint id;
};

#endif
// #include <GL/glew.h>
// #include <GL/glut.h>
// #include <GL/gl.h>

// class Query {
// public:
//     Query();
//     ~Query();

//     void beginQuery() const;
//     void endQuery() const;
//     bool isResultAvailable() const;
//     bool getQueryResult() const;
//     GLuint queryID;

// private:
// };