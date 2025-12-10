#include "lsystemwidget.h"
#include "utils/shaderloader.h"
#include "utils/shapefactory.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>
#include <iostream>
#include <QTimer>

LSystemWidget::LSystemWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_shaderProgram(0)
    , m_cylinderVAO(0)
    , m_projLoc(0)
    , m_viewLoc(0)
    , m_modelLoc(0)
    , m_cylinderNumVertices(0)
    , m_hasTree(false)
    , m_zoom(1.0f)
    , m_rotationAngle(0.0f)
    , m_rotationTimer(nullptr)
{
    m_rotationTimer = new QTimer(this);
    connect(m_rotationTimer, &QTimer::timeout, this, &LSystemWidget::updateRotation);
    m_rotationTimer->start(16);
}

LSystemWidget::~LSystemWidget()
{
    if (m_rotationTimer) {
        m_rotationTimer->stop();
    }
    makeCurrent();
    m_shapeFactory.destroyShapes();
    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
    }
    doneCurrent();
}

void LSystemWidget::initializeGL()
{
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL in LSystemWidget: " << glewGetErrorString(err) << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

    try {
        m_shaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/phong.vert",
            ":/resources/shaders/phong.frag"
        );
    } catch (const std::runtime_error &e) {
        std::cerr << "Shader compile/link error in LSystemWidget: " << e.what() << std::endl;
    }

    m_projLoc = glGetUniformLocation(m_shaderProgram, "projMatrix");
    m_viewLoc = glGetUniformLocation(m_shaderProgram, "viewMatrix");
    m_modelLoc = glGetUniformLocation(m_shaderProgram, "modelMatrix");

    GLint mat_cAmbientLoc = glGetUniformLocation(m_shaderProgram, "material.cAmbient");
    GLint mat_cDiffuseLoc = glGetUniformLocation(m_shaderProgram, "material.cDiffuse");
    GLint mat_cSpecularLoc = glGetUniformLocation(m_shaderProgram, "material.cSpecular");
    GLint mat_shinyLoc = glGetUniformLocation(m_shaderProgram, "material.shininess");
    GLint k_aLoc = glGetUniformLocation(m_shaderProgram, "k_a");
    GLint k_dLoc = glGetUniformLocation(m_shaderProgram, "k_d");
    GLint k_sLoc = glGetUniformLocation(m_shaderProgram, "k_s");
    GLint cameraPosLoc = glGetUniformLocation(m_shaderProgram, "cameraPos");
    GLint numLightsLoc = glGetUniformLocation(m_shaderProgram, "numLights");

    glUseProgram(m_shaderProgram);
    glUniform1f(k_aLoc, 0.5f);
    glUniform1f(k_dLoc, 0.5f);
    glUniform1f(k_sLoc, 0.5f);
    glm::vec3 cameraPos(0.0f, 2.0f, 5.0f);
    glUniform3fv(cameraPosLoc, 1, &cameraPos[0]);
    glUniform1i(numLightsLoc, 0);
    glUseProgram(0);

    setupCylinder();
}

void LSystemWidget::setupCylinder()
{
    m_shapeFactory.initShapes(20, 20);
    const ShapeData &cylinderData = m_shapeFactory.getShapeData(PrimitiveType::PRIMITIVE_CYLINDER);
    
    m_cylinderVAO = cylinderData.vao;
    m_cylinderNumVertices = cylinderData.numVertices;
}

void LSystemWidget::generateTree(const TreeParameters& params)
{
    m_tree = TreeGenerator::generateTree(params);
    m_hasTree = true;
    update();
}

void LSystemWidget::setZoom(float zoom)
{
    m_zoom = zoom;
    update();
}

void LSystemWidget::updateRotation()
{
    m_rotationAngle += 0.5f;
    if (m_rotationAngle >= 360.0f) {
        m_rotationAngle -= 360.0f;
    }
    update();
}

glm::mat4 LSystemWidget::getCylinderTransform(const Segment& segment)
{
    glm::vec3 start = segment.start;
    glm::vec3 end = segment.end;
    glm::vec3 direction = segment.direction;
    float length = segment.length;
    float radius = segment.radius;
    
    glm::vec3 center = (start + end) * 0.5f;
    glm::vec3 defaultUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    glm::vec3 axis = glm::normalize(direction);
    
    glm::vec3 right;
    if (glm::abs(glm::dot(axis, defaultUp)) > 0.99f) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    } else {
        right = glm::normalize(glm::cross(axis, defaultUp));
    }
    glm::vec3 up = glm::normalize(glm::cross(right, axis));
    
    glm::vec3 defaultY = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 rotationAxis = glm::cross(defaultY, axis);
    float rotationAngle = std::acos(glm::clamp(glm::dot(defaultY, axis), -1.0f, 1.0f));
    
    glm::mat4 rotation = glm::mat4(1.0f);
    if (glm::length(rotationAxis) > 0.001f) {
        rotation = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::normalize(rotationAxis));
    } else if (glm::dot(defaultY, axis) < -0.99f) {
        rotation = glm::rotate(glm::mat4(1.0f), 3.14159265359f, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(radius * 2.0f, length, radius * 2.0f));
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), center);
    
    return translation * rotation * scale;
}

void LSystemWidget::renderTree()
{
    if (!m_hasTree || m_tree.allSegments.empty()) {
        return;
    }
    
    for (const auto& segment : m_tree.allSegments) {
        glm::mat4 model = getCylinderTransform(*segment);
        
        glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, &model[0][0]);
        
        GLint mat_cAmbientLoc = glGetUniformLocation(m_shaderProgram, "material.cAmbient");
        GLint mat_cDiffuseLoc = glGetUniformLocation(m_shaderProgram, "material.cDiffuse");
        GLint mat_cSpecularLoc = glGetUniformLocation(m_shaderProgram, "material.cSpecular");
        GLint mat_shinyLoc = glGetUniformLocation(m_shaderProgram, "material.shininess");
        
        glm::vec4 cAmbient(0.15f, 0.1f, 0.05f, 1.0f);
        glm::vec4 cDiffuse(0.3f, 0.2f, 0.1f, 1.0f);
        glm::vec4 cSpecular(0.2f, 0.2f, 0.2f, 1.0f);
        float shininess = 16.0f;
        
        glUniform4fv(mat_cAmbientLoc, 1, &cAmbient[0]);
        glUniform4fv(mat_cDiffuseLoc, 1, &cDiffuse[0]);
        glUniform4fv(mat_cSpecularLoc, 1, &cSpecular[0]);
        glUniform1f(mat_shinyLoc, shininess);
        
        glBindVertexArray(m_cylinderVAO);
        glDrawArrays(GL_TRIANGLES, 0, m_cylinderNumVertices);
        glBindVertexArray(0);
    }
}

void LSystemWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shaderProgram);

    float aspect = static_cast<float>(width()) / static_cast<float>(height());
    if (aspect <= 0) {
        aspect = 1.0f;
    }

    float fov = 45.0f / m_zoom;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 90.0f) fov = 90.0f;
    
    glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
    
    float cameraDistance = 5.0f * m_zoom;
    float cameraHeight = 2.0f * m_zoom;
    glm::vec3 cameraPos(
        cameraDistance * std::sin(glm::radians(m_rotationAngle)),
        cameraHeight,
        cameraDistance * std::cos(glm::radians(m_rotationAngle))
    );
    
    glm::mat4 view = glm::lookAt(
        cameraPos,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glUniformMatrix4fv(m_projLoc, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, &view[0][0]);

    if (m_hasTree) {
        renderTree();
    } else {
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, &model[0][0]);

        GLint mat_cAmbientLoc = glGetUniformLocation(m_shaderProgram, "material.cAmbient");
        GLint mat_cDiffuseLoc = glGetUniformLocation(m_shaderProgram, "material.cDiffuse");
        GLint mat_cSpecularLoc = glGetUniformLocation(m_shaderProgram, "material.cSpecular");
        GLint mat_shinyLoc = glGetUniformLocation(m_shaderProgram, "material.shininess");

        glm::vec4 cAmbient(0.2f, 0.2f, 0.3f, 1.0f);
        glm::vec4 cDiffuse(0.4f, 0.6f, 0.4f, 1.0f);
        glm::vec4 cSpecular(0.8f, 0.8f, 0.8f, 1.0f);
        float shininess = 32.0f;

        glUniform4fv(mat_cAmbientLoc, 1, &cAmbient[0]);
        glUniform4fv(mat_cDiffuseLoc, 1, &cDiffuse[0]);
        glUniform4fv(mat_cSpecularLoc, 1, &cSpecular[0]);
        glUniform1f(mat_shinyLoc, shininess);

        glBindVertexArray(m_cylinderVAO);
        glDrawArrays(GL_TRIANGLES, 0, m_cylinderNumVertices);
        glBindVertexArray(0);
    }

    glUseProgram(0);
}

void LSystemWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width * devicePixelRatio(), height * devicePixelRatio());
}

