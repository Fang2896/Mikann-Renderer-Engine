//
// Created by fangl on 2023/9/19.
//

#ifndef GL_MANAGER_HPP
#define GL_MANAGER_HPP

#include <qglobal.h>
#if defined(Q_OS_MAC)
#include <QOpenGLFunctions_4_1_Core>  // Mac-specific version
using GLFunctions_Core = QOpenGLFunctions_4_1_Core;
#elif defined(Q_OS_WIN)
#include <QOpenGLFunctions_4_3_Core>  // Windows-specific version
using GLFunctions_Core = QOpenGLFunctions_4_3_Core;
#endif

#include <QVector3D>
#include <QOpenGLWidget>
#include <QElapsedTimer>

#include "camera.hpp"
#include "shape.hpp"
#include "resource_manager.hpp"


class GLManager : public QOpenGLWidget {
   public:
    explicit GLManager(QWidget* parent = nullptr, int width = 800,
                       int height = 400);
    ~GLManager() override;

   protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

   private:  // functions
    void initObjects();
    void initResources();
    void initConfigureVariables();
    void initOpenGLSettings();

    void drawCoordinate();

    void handleInput(GLfloat dt);
    void updateRenderData();
    static void checkGLVersion();

   private:  // key variables
    GLFunctions_Core* glFunc = nullptr;
    std::unique_ptr<Camera> m_camera;

    GLuint coordVAO;
    GLuint coordVBO;
    QVector<float> coordData;

    QElapsedTimer eTimer;

   private:  // configure variables
    GLboolean isLineMode;
    QVector3D backGroundColor;

   private:  // control variables
    GLboolean keys[1024];
    GLboolean shiftDown;
    GLfloat defaultCameraMoveSpeed;

    GLboolean isFirstMouse;
    GLboolean isRightMousePress;
    GLint lastX;
    GLint lastY;

    GLfloat deltaTime;
    GLfloat lastFrame;

    QMatrix4x4 projection;
    QMatrix4x4 view;
};

#endif  //GL_MANAGER_HPP
