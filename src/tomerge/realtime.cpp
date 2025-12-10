#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "glm/ext/matrix_clip_space.hpp"
#include "settings.h"
#include "src/shaderloader.h"
#include <glm/gtx/string_cast.hpp>



// ================== Rendering the Scene!

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    for (auto &[_, vbo]: m_vbos){
        glDeleteBuffers(1, &vbo);
    }
    for (auto &[_, vao]: m_vaos){
        glDeleteVertexArrays(1, &vao);
    }

    m_vbos.clear();
    m_vaos.clear();
    m_shapes_map.clear();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glDeleteTextures(1, &m_fbo_col_texture);
    glDeleteTextures(1, &m_fbo_depth_texture);
    // glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteFramebuffers(1, &m_particleVAO);
    glDeleteFramebuffers(1, &m_particleVBO);
    glDeleteFramebuffers(1, &m_particleInstanceVBO);


    this->doneCurrent();

}

// bloom stuff



// particle stuff
void Realtime::initializeParticles(){
    m_particles.resize(m_maxParticles);

    // Generate instance-data VBO (new!)
    glGenBuffers(1, &m_particleInstanceVBO);

    // base quad for particle
    float particle_quad[] = {
        // positions   // texcoords
        -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  1.0f, 1.0f,

        -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &m_particleVAO);
    glGenBuffers(1, &m_particleVBO);

    glBindVertexArray(m_particleVAO);

    // Quad VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

    // quad pos, atrribute 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)0
        );

    // quad tex coords, atrribute 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)(2 * sizeof(float))
        );

    // bind buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_particleInstanceVBO);

    // location 2, position
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 3, GL_FLOAT, GL_FALSE,
        sizeof(ParticleInstance),
        (void*)offsetof(ParticleInstance, pos)
        );
    glVertexAttribDivisor(2, 1);  // advance once per instance

    // locaiton 3, size
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, 1, GL_FLOAT, GL_FALSE,
        sizeof(ParticleInstance),
        (void*)offsetof(ParticleInstance, size)
        );
    glVertexAttribDivisor(3, 1);

    // location 4, color
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
        4, 4, GL_FLOAT, GL_FALSE,
        sizeof(ParticleInstance),
        (void*)offsetof(ParticleInstance, color)
        );
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);

    // location 5, instance
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(
        5, 1, GL_INT,
        sizeof(ParticleInstance),
        (void*)offsetof(ParticleInstance, type)
        );
    glVertexAttribDivisor(5, 1);

    // load in the shaders
    m_particleShader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/particles.vert",
        ":/resources/shaders/particles.frag"
        );
    glUseProgram(m_particleShader);
    glUniform1i(glGetUniformLocation(m_particleShader, "sprite"), 0);
    glUseProgram(0);

    // dirt texture
    QImage dirtImg(":/resources/textures/dirtparticle1.png");
    dirtImg = dirtImg.convertToFormat(QImage::Format_RGBA8888);

    glGenTextures(1, &m_dirtParticleTexture);
    glBindTexture(GL_TEXTURE_2D, m_dirtParticleTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 dirtImg.width(), dirtImg.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, dirtImg.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // wisp texture
    QImage wispImg(":/resources/textures/wisp.png");
    wispImg = wispImg.convertToFormat(QImage::Format_RGBA8888).mirrored();

    glGenTextures(1, &m_wispParticleTexture);
    glBindTexture(GL_TEXTURE_2D, m_wispParticleTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 wispImg.width(), wispImg.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, wispImg.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Realtime::updateParticles(float dt) {

    m_aliveFogInstances.clear();
    m_aliveDirtInstances.clear();
    m_aliveFogInstances.reserve(m_maxParticles);
    m_aliveDirtInstances.reserve(m_maxParticles);

    for (Particle &p : m_particles) {
        if (p.Life > 0.0f) {
            p.Life -= dt;

            if (p.pType == ParticleType::PARTICLE_FOG_WISP) {
                // horizontal drift
                float driftSpeed = 0.4f;
                p.Position += p.DriftDir * driftSpeed * dt;

                // vertical bob
                float bobSpeed   = 1.2f;
                float bobAmount  = 0.25f;
                p.Position.y += sin((p.StartLife - p.Life) * bobSpeed) * bobAmount * dt;

                // spread
                glm::vec3 spread(
                    ((rand() % 100) / 100.f - 0.5f) * 0.002f,
                    ((rand() % 100) / 100.f - 0.5f) * 0.001f,
                    ((rand() % 100) / 100.f - 0.5f) * 0.002f
                    );
                p.Velocity += spread * dt;

                // fading
                float lifeRatio = p.Life / p.StartLife;
                if (lifeRatio > 0.8f) {
                    float t = (1.0f - lifeRatio) / 0.2f;
                    t = glm::clamp(t, 0.0f, 1.0f);
                    p.Color.a = glm::mix(0.0f, 0.45f, t);
                } else {
                    float t = lifeRatio / 0.8f;
                    t = glm::clamp(t, 0.0f, 1.0f);
                    p.Color.a = glm::mix(0.45f, 0.0f, 1.0f - t);
                }
            } else {
                // dirt fade
                p.Color.a -= dt * 1.2f;
            }

            p.Position += p.Velocity * dt;

            // Build instance data
            ParticleInstance inst;
            inst.pos   = p.Position;
            inst.size  = p.Size;
            inst.color = p.Color;

            if (p.pType == ParticleType::PARTICLE_FOG_WISP) {
                m_aliveFogInstances.push_back(inst);
            } else {
                m_aliveDirtInstances.push_back(inst);
            }
        }
    }
}

void Realtime::spawnDirtParticles(float dt) {
    if (!isMoving())
        return;
    m_particleSpawnTimer -= dt;

    if (m_particleSpawnTimer <= 0.0f) {
        spawnSingleDirtParticle();
        m_particleSpawnTimer = m_particleSpawnInterval;  // reset timer
    }
}

void Realtime::spawnSingleDirtParticle() {
    Particle &p = m_particles[m_nextParticle];

    glm::vec3 feet = getCameraFeetPosition();

    float spread = 0.45f;
    float rX = ((rand() % 100) / 100.0f - 0.5f) * spread;
    float rZ = ((rand() % 100) / 100.0f - 0.5f) * spread;

    p.Position = feet + glm::vec3(rX, 0.0f, rZ);
    p.Velocity = glm::vec3(0.0f, 1.0f, 0.0f);
    p.Color = glm::vec4(0.4f, 0.3f, 0.2f, 1.0f);
    p.Life = 0.7f;
    p.Size = 0.15f;


    m_nextParticle = (m_nextParticle + 1) % m_maxParticles;
}

void Realtime::spawnFogWisp(float dt) {
    m_wispSpawnTimer -= dt;
    if (m_wispSpawnTimer > 0.f) return;


    // random spawn
    m_wispSpawnTimer = m_wispSpawnInterval +
                      ((rand() % 100) / 100.f) * 1.2f;

    Particle &p = m_particles[m_nextParticle];
    m_nextParticle = (m_nextParticle + 1) % m_maxParticles;

    p.Life = 8.f + ((rand() % 100) / 100.f) * 4.f;
    p.StartLife = p.Life;

    p.pType = ParticleType::PARTICLE_FOG_WISP;

    glm::vec3 camPos = m_cam.invViewMatrix * glm::vec4(0, 0, 0, 1);

    // distance from player
    float minDist = 25.f;
    float maxDist = 35.f;

    glm::vec3 forward = m_cam.getLookVec();
    float baseAngle = atan2(forward.z, forward.x);

    // 55 degree cone in front of user
    float spread = glm::radians(55.0f);

    float angle = baseAngle + ( ((rand() % 100) / 100.f - 0.5f) * 2.0f * spread );
    float dist  = minDist + ((rand() % 100) / 100.f) * (maxDist - minDist);

    float x = cos(angle) * dist;
    float z = sin(angle) * dist;

    // height
    float y = ((rand() % 100) / 100.f) * 2.f - 1.f; // -1 to +1

    p.Position = camPos + glm::vec3(x, y, z);

    float driftAngle = ((rand() % 100) / 100.f) * 6.283185f; // draft angle
    p.DriftDir = glm::vec3(cos(driftAngle), 0.0f, sin(driftAngle));

    // Very slow initial drift
    p.Velocity = glm::vec3(
        ((rand() % 100) / 100.f - 0.5f) * 0.01f,
        ((rand() % 100) / 100.f - 0.5f) * 0.005f, // very small vertical change
        ((rand() % 100) / 100.f - 0.5f) * 0.01f
        );

    p.Color = glm::vec4(0.8f, 0.82f, 0.85f, 0.0f);

    // Longer life
    p.Size = 3.0f + ((rand() % 100) / 100.f) * 1.0f; // 3–4
}

bool Realtime::isMoving() {
    return ( m_keyMap[Qt::Key_W] || m_keyMap[Qt::Key_A] || m_keyMap[Qt::Key_S] || m_keyMap[Qt::Key_D]);
}

void Realtime::drawParticles() {

    size_t fogCount  = m_aliveFogInstances.size();
    size_t dirtCount = m_aliveDirtInstances.size();

    if (fogCount == 0 && dirtCount == 0)
        return;

    glUseProgram(m_particleShader);

    // cameras
    glUniformMatrix4fv(glGetUniformLocation(m_particleShader, "view"), 1, GL_FALSE, &m_cam.viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_particleShader, "proj"), 1, GL_FALSE, &m_cam.projMatrix[0][0]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // transparent particles
    glDepthMask(GL_FALSE);

    glBindVertexArray(m_particleVAO);

    // wisps
    if (fogCount > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_particleInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     fogCount * sizeof(ParticleInstance),
                     m_aliveFogInstances.data(),
                     GL_DYNAMIC_DRAW);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_wispParticleTexture);
        glUniform1i(glGetUniformLocation(m_particleShader, "sprite"), 0);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(fogCount));
    }

    // dirt
    if (dirtCount > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_particleInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     dirtCount * sizeof(ParticleInstance),
                     m_aliveDirtInstances.data(),
                     GL_DYNAMIC_DRAW);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_dirtParticleTexture);
        glUniform1i(glGetUniformLocation(m_particleShader, "sprite"), 0);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(dirtCount));
    }

    glBindVertexArray(0);

    glDepthMask(GL_TRUE); // restore depth writes
    glDisable(GL_BLEND);
    glUseProgram(0);
}

glm::vec3 Realtime::getCameraFeetPosition() {
    glm::vec3 camPos = m_cam.invViewMatrix * glm::vec4(0, 0, 0, 1);
    // move downward to "feet" level
    camPos.y -= 1.0f;
    return camPos;
}




// bloom



// proj 6 stuff

void Realtime::chooseLUT(){
    // now we do one for color grading

    // available colors:
    // neutral, fancy (it's blue), blackandwhite, warm (warmer red tone), horror

    QImage lutImage = QImage(":/resources/luts/neutral.png");
    if (m_lut_curr == 1){
        lutImage = QImage(":/resources/luts/fancy.png");
    }
    else if (m_lut_curr == 2){
        lutImage = QImage(":/resources/luts/warm.png");
    }
    else if (m_lut_curr == 3){
        lutImage = QImage(":/resources/luts/blackandwhite.png");
    }

    QImage lut = lutImage.convertToFormat(QImage::Format_RGBA8888);
    // lut = lut.convertToFormat(QImage::Format_RGBA8888).mirrored();

    // qDebug() << "LUT resolution:" << lut.width() << "x" << lut.height();

    // For a 512x512 8x8 grid of 64x64 tiles,
    m_lutSize = 16.0f;

    glGenTextures(1, &m_fbo_lut_texture);
    glBindTexture(GL_TEXTURE_2D, m_fbo_lut_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 lut.width(), lut.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, lut.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(0, m_fbo_lut_texture);
}

void Realtime::makeFBO(){


    // color texture first

    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_col_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_col_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glBindTexture(GL_TEXTURE_2D, 0);

    // now we need to do depth as a texture, not just a renderbuffer?
    glGenTextures(1, &m_fbo_depth_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_fbo_width, m_fbo_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

glBindTexture(GL_TEXTURE_2D, 0);
    // do this for bloom
    glGenTextures(1, &m_bright_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_bright_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glBindTexture(GL_TEXTURE_2D, 0);

    // color grading
    chooseLUT();

    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind
    // glGenRenderbuffers(1, &m_fbo_renderbuffer);
    // glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    // glBindRenderbuffer(0, m_fbo_renderbuffer);

    // Task 18: Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Task 21: Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_col_texture, 0);

    // do same for depth
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fbo_depth_texture, 0);

    // do same for extra bloom
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_bright_texture, 0);


    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    // Task 22: Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Realtime::makeBloomFBOs(){
    glGenFramebuffers(2, pingpongFBOs);
    glGenTextures(2, pingpongBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBOs[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_FLOAT, NULL
            );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i], 0
            );
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Realtime::bouncePingPong(){
    bool horizontal = true, first_iteration = true;
    int amount = 10;
    glUseProgram(m_blur_shader);
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBOs[horizontal]);
        glUniform1i(glGetUniformLocation(m_blur_shader, "horizontal"), horizontal);

        glActiveTexture(GL_TEXTURE0);

        glBindTexture(
            GL_TEXTURE_2D, first_iteration ? m_bright_texture : pingpongBuffers[!horizontal]
            );
        glBindVertexArray(m_fullscreen_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}


// Task 31: Update the paintTexture function signature
void Realtime::paintTexture(GLuint col_texture, GLuint depth_texture, GLuint lut_texture, GLuint bloom_texture, bool postProcess){
    glUseProgram(m_texture_shader);
    // Task 32: Set your bool uniform on whether or not to filter the texture drawn

    glUniform1i(glGetUniformLocation(m_texture_shader, "postProcessing"), postProcess);


    glBindVertexArray(m_fullscreen_vao);
    // Task 10: Bind "texture" to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, col_texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depth_texture);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, lut_texture);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, bloom_texture);



    glDrawArrays(GL_TRIANGLES, 0, 6); // bug was here only drawing 3 vertices
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void Realtime::initializePostProcessingPipeline() {
    // screen and fbo
    m_screen_width  = size().width()  * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width  = m_screen_width;
    m_fbo_height = m_screen_height;

    // load shaders
    m_texture_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/texture.vert",
        ":/resources/shaders/texture.frag"
        );

    // Tell the shader that "tex" uses texture unit 0
    // also tell that the depth is 2
    glUseProgram(m_texture_shader);
    glUniform1i(glGetUniformLocation(m_texture_shader, "colorTex"), 0);
    glUniform1i(glGetUniformLocation(m_texture_shader, "depthTex"), 1);
    glUniform1i(glGetUniformLocation(m_texture_shader, "lutTex"), 2);
    // glUniform1f(glGetUniformLocation(m_texture_shader, "lutSize"), float(m_lutSize));
    glUniform1i(glGetUniformLocation(m_texture_shader, "bloomBlur"), 3);


    glUniform1f(glGetUniformLocation(m_texture_shader, "near"), settings.nearPlane);
    glUniform1f(glGetUniformLocation(m_texture_shader, "far"),  settings.farPlane);

    glUniform1i(glGetUniformLocation(m_texture_shader, "ppMode"), settings.ppMode);

    glUseProgram(0);

    // --- FULLSCREEN QUAD GEOMETRY ---
    std::vector<GLfloat> fullscreen_quad_data = {
        // pos               uv
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
        1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        1.0f, -1.0f, 0.0f,  1.0f, 0.0f
    };

    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 fullscreen_quad_data.size() * sizeof(GLfloat),
                 fullscreen_quad_data.data(),
                 GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // layout(location = 0) = vec3 pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        5 * sizeof(GLfloat), (void*)0
        );

    // layout(location = 1) = vec2 uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))
        );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    makeFBO();

    //
    makeBloomFBOs();

    glUseProgram(m_texture_shader);
    glUniform1f(glGetUniformLocation(m_texture_shader, "lutSize"), m_lutSize);
    glUseProgram(0);
}


void Realtime::rebuildGeometryNumShapes(int numShapes){
    if (!m_glmInit) return;

    for (auto &[_, vbo]: m_vbos){
        glDeleteBuffers(1, &vbo);
    }
    for (auto &[_, vao]: m_vaos){
        glDeleteVertexArrays(1, &vao);
    }

    m_vbos.clear();
    m_vaos.clear();
    m_shapes_map.clear();

    float scale = 1.0f / std::sqrt(numShapes);

    // got this scale from chat
    // float scale = 1.0f / (1.0f + 0.25f * std::log2(std::max(1, safeN)));


    int curr_param1 = int(m_param1 * scale);
    int curr_param2 = int(m_param2 * scale);



    // remake shapes
    Sphere sphere;
    sphere.updateParams(curr_param1, curr_param2);
    // std::cout << m_param1 << m_param2 << std::endl;
    m_shapes_map[PrimitiveType::PRIMITIVE_SPHERE] = sphere.generateShape();
    // std::cout << "made sphere" << std::endl;

    Cylinder cylinder;
    cylinder.updateParams(curr_param1, curr_param2);
    m_shapes_map[PrimitiveType::PRIMITIVE_CYLINDER] = cylinder.generateShape();

    Cone cone;
    cone.updateParams(curr_param1, curr_param2);
    m_shapes_map[PrimitiveType::PRIMITIVE_CONE] = cone.generateShape();

    Cube cube;
    cube.updateParams(curr_param1);
    m_shapes_map[PrimitiveType::PRIMITIVE_CUBE] = cube.generateShape();

    for (auto &[type, data] : m_shapes_map) {
        GLuint vbo, vao;

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

        // unbind
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vbos[type] = vbo;
        m_vaos[type] = vao;
    }
}

void Realtime::rebuildGeometry(){
    if (!m_glmInit) return;

    for (auto &[_, vbo]: m_vbos){
        glDeleteBuffers(1, &vbo);
    }
    for (auto &[_, vao]: m_vaos){
        glDeleteVertexArrays(1, &vao);
    }

    m_vbos.clear();
    m_vaos.clear();
    m_shapes_map.clear();

    // remake shapes
    Sphere sphere;
    sphere.updateParams(m_param1, m_param2);
    // std::cout << m_param1 << m_param2 << std::endl;
    m_shapes_map[PrimitiveType::PRIMITIVE_SPHERE] = sphere.generateShape();
    // std::cout << "made sphere" << std::endl;

    Cylinder cylinder;
    cylinder.updateParams(m_param1, m_param2);
    m_shapes_map[PrimitiveType::PRIMITIVE_CYLINDER] = cylinder.generateShape();

    Cone cone;
    cone.updateParams(m_param1, m_param2);
    m_shapes_map[PrimitiveType::PRIMITIVE_CONE] = cone.generateShape();

    Cube cube;
    cube.updateParams(m_param1);
    m_shapes_map[PrimitiveType::PRIMITIVE_CUBE] = cube.generateShape();

    for (auto &[type, data] : m_shapes_map) {
        GLuint vbo, vao;

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

        // unbind
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vbos[type] = vbo;
        m_vaos[type] = vao;
    }
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);


    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_defaultFBO = 2;
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    initializePostProcessingPipeline();

    // for bloom
    m_blur_shader = ShaderLoader::createShaderProgram(":/resources/shaders/blur.vert", ":/resources/shaders/blur.frag");

    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");


    m_glmInit = true;
    // makeScene();
    // rebuildGeometryNumShapes(m_renderData.shapes.size());

    rebuildGeometry();
    // RenderShapeData testShape;
    // testShape.primitive.type = PrimitiveType::PRIMITIVE_SPHERE;
    // testShape.ctm = glm::mat4(1.0f);
    // m_renderData.shapes.push_back(testShape);


    // particle stuff
    initializeParticles();


    update();

}

void Realtime::renderSceneGeometry(){
    // std::cout << glm::to_string(m_view) << std::endl;
    // view matrix
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "viewMatrix"), 1, GL_FALSE, &m_cam.viewMatrix[0][0]);
    // projection matrix
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "projMatrix"), 1, GL_FALSE, &m_cam.projMatrix[0][0]);

    glUniform1f(glGetUniformLocation(m_shader, "ka"), m_ka);
    // std::cout << "ka" << m_ka << std::endl;

    glUniform1f(glGetUniformLocation(m_shader, "kd"), m_kd);
    glUniform1f(glGetUniformLocation(m_shader, "ks"), m_ks);


    if (!m_renderData.lights.empty()) {
        const SceneLightData &light = m_renderData.lights[0];
        glUniform3fv(glGetUniformLocation(m_shader, "lightPos"), 1, &light.pos[0]);
    }

    // get all the lights in that b*tch
    int lightCount = static_cast<int>(m_renderData.lights.size());
    glUniform1i(glGetUniformLocation(m_shader, "lightCount"), lightCount);
    for (int i = 0; i < lightCount; i++) {
        SceneLightData lightData = m_renderData.lights[i];
        std::string path = "lights[" + std::to_string(i) + "]";
        glUniform1i(glGetUniformLocation(m_shader, (path + ".type").c_str()), static_cast<int>(lightData.type));
        glUniform4fv(glGetUniformLocation(m_shader, (path + ".pos").c_str()), 1, &lightData.pos[0]);
        glUniform4fv(glGetUniformLocation(m_shader, (path + ".dir").c_str()), 1, &lightData.dir[0]);
        glUniform4fv(glGetUniformLocation(m_shader, (path + ".color").c_str()), 1, &lightData.color[0]);
        glUniform1f(glGetUniformLocation(m_shader, (path + ".angle").c_str()), lightData.angle);
        glUniform3fv(glGetUniformLocation(m_shader, (path + ".atten").c_str()), 1, &lightData.function[0]);
        glUniform1f(glGetUniformLocation(m_shader, (path + ".penumbra").c_str()), lightData.penumbra);
    }


    glm::vec3 camPos = m_cam.invViewMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform3f(glGetUniformLocation(m_shader, "camPos"), camPos.x, camPos.y, camPos.z);

    for (const RenderShapeData &shape : m_renderData.shapes) {
        PrimitiveType type = shape.primitive.type;

        if (m_vaos.find(type) == m_vaos.end()) continue; // skip unknown

        glBindVertexArray(m_vaos[type]);

        glm::mat4 model = shape.ctm;

        // model matrix and inv transpose model matrix
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "modelMatrix"), 1, GL_FALSE, &model[0][0]);
        glm::mat3 iTModelMatrix = transpose(inverse(glm::mat3(model)));
        glUniformMatrix3fv(glGetUniformLocation(m_shader, "iTModelMatrix"), 1, GL_FALSE, &iTModelMatrix[0][0]);

        glUniform1f(glGetUniformLocation(m_shader, "shininess"), shape.primitive.material.shininess);
        glUniform4fv(glGetUniformLocation(m_shader, "cAmb"), 1, &shape.primitive.material.cAmbient[0]);

        glUniform4fv(glGetUniformLocation(m_shader, "cDiff"), 1, &shape.primitive.material.cDiffuse[0]);
        glUniform4fv(glGetUniformLocation(m_shader, "cSpec"), 1, &shape.primitive.material.cSpecular[0]);


        int vertexCount = m_shapes_map[type].size() / 6;
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    // Clear screen color and depth before painting

    // from lab 11
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_screen_width, m_screen_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClearColor(0.2f, 0.3f, 0.5f, 1.0f);

    glUseProgram(m_shader);

    renderSceneGeometry();


    // particles stuff
    // drawParticles();

    // bouncePingPong();


    glUseProgram(0);

    glUseProgram(m_texture_shader);
    glUniform1i(glGetUniformLocation(m_texture_shader, "ppMode"), settings.ppMode);
    glUniform1i(glGetUniformLocation(m_texture_shader, "postProcessing"), settings.ppMode != 0); // ppMode 0 means off
    glUseProgram(0);

    chooseLUT();


    // do the fbo stuff

    // Task 25: Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    // Task 26: Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_texture_shader);
    glUniform1f(glGetUniformLocation(m_texture_shader, "near"), m_cam.nearPlane);
    glUniform1f(glGetUniformLocation(m_texture_shader, "far"),  m_cam.farPlane);
    glUseProgram(0);

    // glUseProgram(m_texture_shader);
    // glUniform1f(glGetUniformLocation(m_texture_shader, "lutSize"), m_lutSize);
    // glUseProgram(0);

    glUseProgram(m_texture_shader);
    glUniform1f(glGetUniformLocation(m_texture_shader, "offset"),
                m_ppTime * 2.0f * 3.14159f * 0.75f);
    glUseProgram(0);

    glUseProgram(m_texture_shader);
    glUniform1f(glGetUniformLocation(m_texture_shader, "time"),
                m_ppTime);
    glUseProgram(0);

    // Task 27: Call paintTexture to draw our FBO color attachment texture | Task 31: Set bool parameter to true
    GLuint bloomTex = pingpongBuffers[0];
    paintTexture(m_fbo_col_texture, m_fbo_depth_texture, m_fbo_lut_texture, bloomTex, true);


}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_cam.resizeWH(w, h);

    glDeleteTextures(1, &m_fbo_col_texture);
    glDeleteTextures(1, &m_fbo_depth_texture);
    // glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;
    // Task 34: Regenerate your FBOs
    makeFBO();
}

void Realtime::sceneChanged() {
    bool success = SceneParser::parse(settings.sceneFilePath, m_renderData);
    if (!success) {
        std::cout << "Error during scenefile parse" << std::endl;
    }
    m_cam = Camera(m_renderData.cameraData, size().width(), size().height());
    m_cam.updatePlanes(settings.nearPlane, settings.farPlane);
    m_ka = m_renderData.globalData.ka;
    m_kd = m_renderData.globalData.kd;
    m_ks = m_renderData.globalData.ks;

    // m_view = m_cam.viewMatrix;
    // m_invView = m_cam.invViewMatrix;
    // m_proj = m_cam.projMatrix;

    // rebuildGeometryNumShapes(m_renderData.shapes.size());

    rebuildGeometry();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    // std::cout << "test" << std::endl;
    m_cam.updatePlanes(settings.nearPlane, settings.farPlane);
    // m_proj = m_cam.projMatrix;
    m_param1 = settings.shapeParameter1;
    m_param2 = settings.shapeParameter2;
    m_lut_curr = settings.lutChoice;

    makeCurrent();

    // rebuildGeometryNumShapes(m_renderData.shapes.size());

    rebuildGeometry();
    doneCurrent();
    update(); // asks for a PaintGL() call to occur
    // std::cout << "test" << std::endl;
}

// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate

        float sens = 0.01f; // sensitivity
        float horiz = -sens * deltaX;
        float vert = -sens * deltaY;

        // Rotate the camera based on mouse movement
        m_cam.rotate(horiz, true);
        m_cam.rotate(vert, false);

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // update particles

    // updateParticles(deltaTime);
    // spawnDirtParticles(deltaTime);
    // spawnFogWisp(deltaTime);

    // Use deltaTime and m_keyMap here to move around

    if (m_keyMap[Qt::Key_W]){
        m_cam.movement(m_cam.getLookVec() * 5.0f * deltaTime);
    }
    if (m_keyMap[Qt::Key_S]) {
        m_cam.movement(-m_cam.getLookVec() * 5.0f * deltaTime);
    }
    if (m_keyMap[Qt::Key_Space]){
        m_cam.movement(glm::vec3(0.0f, 1.0f, 0.0f) * 5.0f * deltaTime);
    }
    if (m_keyMap[Qt::Key_Control]){
        m_cam.movement(glm::vec3(0.0f, -1.0f, 0.0f) * 5.0f * deltaTime);
    }
    if (m_keyMap[Qt::Key_D]){
        m_cam.movement(m_cam.getRightVec() * 5.0f * deltaTime);
    }
    if (m_keyMap[Qt::Key_A]){
        m_cam.movement(-m_cam.getRightVec() * 5.0f * deltaTime);
    }

    // added for part 6
    // m_ppTime += deltaTime;
    m_ppTime = fmod(m_ppTime + deltaTime, 10000.0f); // capped cause it might be jittery after a long long long time


    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // GLint viewport[4];
    // glGetIntegerv(GL_VIEWPORT, viewport);
    // int fixedWidth = viewport[2];
    // int fixedHeight = viewport[3];

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
