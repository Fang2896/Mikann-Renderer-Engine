//
// Created by fangl on 2023/9/22.
//

#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <utility>

#include <QDebug>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QVector>
#include <QMatrix4x4>

#include "resource_manager.hpp"
#include "shape_data.hpp"
#include "object.hpp"
#include "gl_configure.hpp"
#include "data_structures.hpp"


enum class DataType { kOnlyPosition, kPosTexNor };

enum class DrawMode {
    POINTS,
    LINES,
    LINE_STRIP,
    LINE_LOOP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
};

class Shape : public Object {
   public:
    explicit Shape(QString  geometryType);
    ~Shape();

    void init(QString shaderName) override;
    void init() override;
    void draw() override;

    void updateDiffuseTexture(const QString& tPath);
    void updateSpecularTexture(const QString& tPath);
    void updateShapeData(const QVector<float>& posData,
                         const QVector<unsigned int>& indexData);
    void updateShapeMaterial(ShapeMaterial mat = ShapeMaterial());

   private:
    GLFunctions_Core *glFunc;

    QVector<Vertex> vertices;
    QVector<unsigned int> indices;
    QString geometryType;
    ShapeMaterial baseMaterial;
    std::shared_ptr<Texture2D> diffuseTexture;
    std::shared_ptr<Texture2D> specularTexture;

    GLuint VAO{};
    GLuint VBO{};
    GLuint EBO{};

    void initShapeData(const QVector<float>& posData,
                    const QVector<unsigned int>& indexData);
    void convertDataFloatToVertex(const QVector<float>& data);
};

#endif  //SHAPE_HPP
