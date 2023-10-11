#include "Query.h"

Query::Query(GLuint id)
    : id(id)
    {}

void Query::begin() const
{
    glBeginQuery(GL_ANY_SAMPLES_PASSED, id);
}

void Query::end() const
{
    glEndQuery(GL_ANY_SAMPLES_PASSED);
}

bool Query::isVisible() const
{
    GLint param;
    glGetQueryObjectiv(id, GL_QUERY_RESULT, &param);
    return param == GL_TRUE;
}

bool Query::resultIsReady() const
{
    GLint param;
    glGetQueryObjectiv(id, GL_QUERY_RESULT_AVAILABLE, &param);
    return param == GL_TRUE;
}
// #include "Query.h"

// Query::Query()
// {
//     glGenQueries(1, &queryID);
// }

// Query::~Query()
// {
//     glDeleteQueries(1, &queryID);
// }

// void Query::beginQuery() const
// {
//     glBeginQuery(GL_ANY_SAMPLES_PASSED, queryID);
// }

// void Query::endQuery() const
// {
//     glEndQuery(GL_ANY_SAMPLES_PASSED);
// }

// bool Query::getQueryResult() const
// {
//     GLuint result;
//     glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &result);
//     return result == GL_TRUE;
// }

// bool Query::isResultAvailable() const
// {
//     GLuint available;
//     glGetQueryObjectuiv(queryID, GL_QUERY_RESULT_AVAILABLE, &available);
//     return available == GL_TRUE;
// }