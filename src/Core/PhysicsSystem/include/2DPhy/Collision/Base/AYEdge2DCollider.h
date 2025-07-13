#pragma once
#include "BasePhy/IAYCollider.h"

class AYEdge2DCollider : public IAYCollider {
public:
    explicit AYEdge2DCollider(const glm::vec2& vertex1 = { 0,0 },
        const glm::vec2& vertex2 = { 1,0 });

    // ��״����
    ShapeType getShapeType() const override { return ShapeType::Edge2D; }

    // λ��ƫ��
    void setOffset(const glm::vec2& offset) override;

    // ���з���
    const glm::vec2& getVertex1() const { return _vertex1; }
    const glm::vec2& getVertex2() const { return _vertex2; }
    void setVertices(const glm::vec2& v1, const glm::vec2& v2) { _vertex1 = v1; _vertex2 = v2; }

    bool isOneSided() const { return _isOneSided; }
    void setOneSided(bool oneSided) { _isOneSided = oneSided; }

    // ʵ�÷���
    float getLength() const;
    glm::vec2 getDirection() const;

private:
    glm::vec2 _vertex1;
    glm::vec2 _vertex2;
    bool _isOneSided = false;
};