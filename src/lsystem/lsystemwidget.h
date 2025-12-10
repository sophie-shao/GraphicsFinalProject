#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <QOpenGLWidget>
#include <QTimer>
#include <glm/glm.hpp>
#include "utils/shapefactory.h"
#include "axialtree.h"
#include "treegenerator.h"

class LSystemWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    LSystemWidget(QWidget *parent = nullptr);
    ~LSystemWidget();
    
    void generateTree(const TreeParameters& params);
    void setZoom(float zoom);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    GLuint m_shaderProgram;
    GLuint m_cylinderVAO;
    GLint m_projLoc;
    GLint m_viewLoc;
    GLint m_modelLoc;
    
    ShapeFactory m_shapeFactory;
    int m_cylinderNumVertices;
    
    AxialTree m_tree;
    bool m_hasTree;
    float m_zoom;
    float m_rotationAngle;
    QTimer *m_rotationTimer;
    
    void setupCylinder();
    void renderTree();
    glm::mat4 getCylinderTransform(const Segment& segment);
    void updateRotation();
};

