﻿//
// Created by fangl on 2023/9/19.
//

#include "gl_manager.hpp"

const QVector3D CAMERA_POSITION(0.0f, 0.5f, 3.0f);


GLManager::GLManager(QWidget* parent, int width, int height)
    : QOpenGLWidget(parent) {
    coordData = ShapeData::getCoordinateVertices();
    this->setGeometry(10, 20, width, height);
}

GLManager::~GLManager() = default;

/********* OpenGL Functions *********/
void GLManager::initializeGL() {
    initConfigureVariables();
    initOpenGLSettings();
    initShaders();    // shader

    // TODO: 这里可以从coordinate改成各种绘制精灵？
    initCoordinate();
    initShaderValue();

    // member mangers
    // object manager, resource manager...
    m_camera = std::make_unique<Camera>(CAMERA_POSITION, defaultCameraMoveSpeed);

    objectVec.push_back(std::make_shared<Shape>("cube"));
    objectVec[0]->init();
    objectVec[0]->setPosition({-3, 0, 0});

    objectVec.push_back(std::make_shared<Model>(
        "E:/ToyPrograms/GL/MikannRendererEngine/Mikann-Renderer-Engine/assets/models/nanosuit/nanosuit.obj"));
    objectVec[1]->init();
    objectVec[1]->setPosition(QVector3D(3,0,0));
    objectVec[1]->setScale(0.2f);

    objectVec.push_back(std::make_shared<Shape>("cube"));
    objectVec[2]->init("defaultTextureShader");
    objectVec[2]->setPosition({0, 0, 3});
    auto* tempShapePtr = dynamic_cast<Shape*>(objectVec[2].get());
    if(tempShapePtr) {
        // 调用 Shape 的函数
        tempShapePtr->updateDiffuseTexture(
            "E:/ToyPrograms/GL/MikannRendererEngine/Mikann-Renderer-Engine/assets/textures/box_diffuse.png");
        tempShapePtr->updateSpecularTexture(
            "E:/ToyPrograms/GL/MikannRendererEngine/Mikann-Renderer-Engine/assets/textures/box_specular.png");
    } else {
        qFatal("Temp Shape Ptr Empty!");
    }

    // Model must be initialized after shader!
    // TODO: 后面要加一个可以动态改变shader的？ updateShader函数. 目前先暂时满足于动态改变shader值
//    m_testModel = std::make_unique<Model>("../assets/models/nanosuit/nanosuit.obj");
//    m_testModel->init("defaultModelShader");
//    m_testCube = std::make_unique<Shape>("cube");
//    m_testCube->init("defaultShapeShader");

    // start timer
    eTimer.start();
}

void GLManager::resizeGL(int w, int h) {
    glFunc->glViewport(0, 0, w, h);
}

void GLManager::paintGL() {
    if(this->isLineMode)
        glFunc->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glFunc->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // 这个是可有可无？
    glFunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFunc->glClearColor(backGroundColor.x(),
                         backGroundColor.y(),
                         backGroundColor.z(), 1.0f);  // 例如：清除为黑色

    // time and position data
    GLfloat currentFrame = (GLfloat)eTimer.elapsed() / 100;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    
    this->handleInput(deltaTime);
    this->updateRenderData();

    ResourceManager::getShader("coordShader").use();
    drawCoordinate();

    drawObjects();
//    ResourceManager::getShader(m_testModel->getShaderName()).use();
//    m_testModel->draw();
//    ResourceManager::getShader(m_testCube->getShaderName()).use();
//    m_testCube->draw();
}

/********* Other Functions *********/
void GLManager::initCoordinate() {
    glFunc->glGenBuffers(1, &coordVBO);
    glFunc->glBindBuffer(GL_ARRAY_BUFFER, coordVBO);
    glFunc->glBufferData(GL_ARRAY_BUFFER, coordData.size() * sizeof(float), coordData.data(), GL_STATIC_DRAW);

    glFunc->glGenVertexArrays(1, &coordVAO);
    glFunc->glBindVertexArray(coordVAO);

    glFunc->glBindBuffer(GL_ARRAY_BUFFER, coordVBO);

    glFunc->glEnableVertexAttribArray(0);
    glFunc->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)3 * sizeof(float), (void*)nullptr);
    glFunc->glBindVertexArray(0);

    qDebug() << "======= Done Init Coordinate ========";
}

void GLManager::drawCoordinate() {
    glFunc->glBindVertexArray(coordVAO);
    glFunc->glDrawArrays(GL_LINES, 0, (GLsizei)coordData.size() / 3);
    glFunc->glBindVertexArray(0);
}

void GLManager::drawObjects() {
    for(const auto& obj : objectVec) {
        ResourceManager::getShader(obj->getShaderName()).use();
        obj->draw();
    }

}

// TODO: 以后glManager只需要遍历所有的shader，然后为他们设置MVP矩阵，和一些全局变量就行
//  比如：光照信息，视线方向，等等，然后其他的因素，比如说transform，物体的material，就object自己内部实现就行
//  然后还能自己设置，改变很多信息，无敌

// TODO: 这里要注意，虽然shader的文件数是固定的，但是我们需要为每一个object都设置一个shader？（可能）
//  不然就只能在draw那里，每帧都设置一遍（目前先这样实现吧）（时间换空间。而且管理多个shader感觉也好麻烦）
void GLManager::initShaders() {
    ResourceManager::loadShader("coordShader",
                                ":/shaders/assets/shaders/coordShader.vert",
                                ":/shaders/assets/shaders/coordShader.frag");
    ResourceManager::loadShader("defaultModelShader",
                                ":/shaders/assets/shaders/defaultModelShader.vert",
                                ":/shaders/assets/shaders/defaultModelShader.frag");
    ResourceManager::loadShader("defaultShapeShader",
                                ":/shaders/assets/shaders/defaultShapeShader.vert",
                                ":/shaders/assets/shaders/defaultShapeShader.frag");
    ResourceManager::loadShader("defaultTextureShader",
                                ":/shaders/assets/shaders/defaultTextureShader.vert",
                                ":/shaders/assets/shaders/defaultTextureShader.frag");

    qDebug() << "======= Done Init Shaders ========";
}

// 在物体初始化后，或者增加物体，改变这里边的参数后调用！
void GLManager::initShaderValue() {
    initLightInfo();    // 要在shaderInit之前

    // TODO: 这里的光照信息可以抽出来
    const Shader& tempShapeShader = ResourceManager::getShader("defaultShapeShader").use();
    tempShapeShader.setVector3f("directLight.direction", directLight.direction);
    tempShapeShader.setVector3f("directLight.ambientColor", directLight.ambientColor);
    tempShapeShader.setVector3f("directLight.diffuseColor", directLight.diffuseColor);
    tempShapeShader.setVector3f("directLight.specularColor", directLight.specularColor);
    tempShapeShader.setFloat("directLight.intensity", directLight.intensity);

    const Shader& tempModelShader = ResourceManager::getShader("defaultModelShader").use();
    tempModelShader.setVector3f("directLight.direction", directLight.direction);
    tempModelShader.setVector3f("directLight.ambientColor", directLight.ambientColor);
    tempModelShader.setVector3f("directLight.diffuseColor", directLight.diffuseColor);
    tempModelShader.setVector3f("directLight.specularColor", directLight.specularColor);
    tempModelShader.setFloat("directLight.intensity", directLight.intensity);

    const Shader& tempTextureShader = ResourceManager::getShader("defaultTextureShader").use();
    tempTextureShader.setVector3f("directLight.direction", directLight.direction);
    tempTextureShader.setVector3f("directLight.ambientColor", directLight.ambientColor);
    tempTextureShader.setVector3f("directLight.diffuseColor", directLight.diffuseColor);
    tempTextureShader.setVector3f("directLight.specularColor", directLight.specularColor);
    tempTextureShader.setFloat("directLight.intensity", directLight.intensity);

    // coordinate matrix configuration （因为坐标位置是不变的）
    QMatrix4x4 model;
    model.setToIdentity();
    model.scale(5.0f);
    ResourceManager::getShader("coordShader").use().setMatrix4f("model", model);
}

void GLManager::updateRenderData() {
    if(this->isLineMode)
        glFunc->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glFunc->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    projection.setToIdentity();
    projection.perspective(m_camera->zoom, (GLfloat)width() / (GLfloat)height(), 0.1f, 200.f);
    view = m_camera->getViewMatrix();

    ResourceManager::getShader("coordShader").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("coordShader").use().setMatrix4f("view", view);

    ResourceManager::getShader("defaultModelShader").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("defaultModelShader").use().setMatrix4f("view", view);
    ResourceManager::getShader("defaultShapeShader").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("defaultShapeShader").use().setMatrix4f("view", view);
    ResourceManager::getShader("defaultTextureShader").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("defaultTextureShader").use().setMatrix4f("view", view);

    // 为管理的objects设置model
    for(const auto& obj : objectVec) {
        ResourceManager::getShader(obj->getShaderName()).use().setMatrix4f("model", obj->getTransform());
    }

    ResourceManager::getShader("defaultModelShader").use().setVector3f("viewPos", m_camera->position);
    ResourceManager::getShader("defaultShapeShader").use().setVector3f("viewPos", m_camera->position);
    ResourceManager::getShader("defaultTextureShader").use().setVector3f("viewPos", m_camera->position);
}

void GLManager::checkGLVersion() {
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context) {
        QSurfaceFormat format = context->format();
        int majorVersion = format.majorVersion();
        int minorVersion = format.minorVersion();
        qDebug() << "OpenGL Version:" << majorVersion << "." << minorVersion;
    } else {
        qDebug() << "OpenGL Context Empty.";
    }
}

void GLManager::initConfigureVariables() {
    isLineMode = GL_FALSE;
    backGroundColor = QVector3D(0.35f, 0.35f, 0.35f);

    defaultCameraMoveSpeed = 0.2f;
    shiftDown = GL_FALSE;
    isFirstMouse = GL_TRUE;
    isRightMousePress = GL_FALSE;

    deltaTime = 0.0f;
    lastFrame = 0.0f;
    lastX = (int)((float)width() / 2.0f);
    lastY = (int)((float)height() / 2.0f);

    for (auto& key : keys) {
        key = GL_FALSE;
    }
}

void GLManager::initLightInfo() {
    directLight = DirectLight();

    // TODO: 灯光管理。。。
}

void GLManager::initOpenGLSettings() {
    checkGLVersion();
    glFunc = QOpenGLContext::currentContext()->versionFunctions<GLFunctions_Core>();
    if (!glFunc) {
        qFatal("Requires OpenGL >= 4.3");
    }
    glFunc->initializeOpenGLFunctions();
    glFunc->glEnable(GL_DEPTH_TEST);
    glFunc->glClearColor(backGroundColor.x(),
                         backGroundColor.y(),
                         backGroundColor.z(), 1.0f);
    glFunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    qDebug() << "======= Done Init OpenGL Settings ========";
}

/********* Event Functions *********/
void GLManager::keyPressEvent(QKeyEvent *event) {
    if(isFirstMouse)
        return;

    GLuint key = event->key();
    if(key < 1024)
        this->keys[key] = GL_TRUE;
    else if(key == Qt::Key_Shift)
        shiftDown = GL_TRUE;
}

void GLManager::keyReleaseEvent(QKeyEvent *event) {
    GLuint key = event->key();
    if(key < 1024)
        this->keys[key] = GL_FALSE;
    else if(key == Qt::Key_Shift)
        shiftDown = GL_FALSE;
}

void GLManager::mouseMoveEvent(QMouseEvent *event) {
    if(!isRightMousePress)
        return;

    GLint xPos = event->pos().x();
    GLint yPos = event->pos().y();
    if (isFirstMouse){
        qDebug() << "First Mouse Click : " << xPos << ", " << yPos;
        lastX = xPos;
        lastY = yPos;
        isFirstMouse = GL_FALSE;
    }

    GLint xOffset = xPos - lastX;
    GLint yOffset = lastY - yPos; // reversed since y-coordinates go from bottom to top
    lastX = xPos;
    lastY = yPos;
    m_camera->handleMouseMovement((GLfloat)xOffset, (GLfloat)yOffset);
}

void GLManager::wheelEvent(QWheelEvent *event) {
    QPoint offset = event->angleDelta();
    m_camera->handleMouseScroll((float)offset.y() / 20.0f);
}

void GLManager::mousePressEvent(QMouseEvent *event) {
    setFocus();
    if(event->button() == Qt::RightButton) {
        m_camera->movementSpeed *= 3.0f;
        isRightMousePress = GL_TRUE;;
    }
}

void GLManager::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton){
        isRightMousePress = GL_FALSE;
        isFirstMouse = GL_TRUE;
    }
}

void GLManager::handleInput(GLfloat dt) {
    if (keys[Qt::Key_W])
        m_camera->handleKeyboard(CameraMove::FORWARD, dt);
    if (keys[Qt::Key_S])
        m_camera->handleKeyboard(CameraMove::BACKWARD, dt);
    if (keys[Qt::Key_A])
        m_camera->handleKeyboard(CameraMove::LEFT, dt);
    if (keys[Qt::Key_D])
        m_camera->handleKeyboard(CameraMove::RIGHT, dt);
    if (keys[Qt::Key_E])
        m_camera->handleKeyboard(CameraMove::UP, dt);
    if (keys[Qt::Key_Q])
        m_camera->handleKeyboard(CameraMove::DOWN, dt);

    if (shiftDown)
        m_camera->movementSpeed = 2.5f * defaultCameraMoveSpeed;
    else
        m_camera->movementSpeed = defaultCameraMoveSpeed;
}

