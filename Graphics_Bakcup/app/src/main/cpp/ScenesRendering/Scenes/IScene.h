#pragma once

#include <vector>
#include <memory>
#include <openxr/openxr.h>

class IRenderableGeometry;
class Material;

struct SceneObject {
    XrPosef pose;
    XrVector3f scale;
    XrVector3f color;
    std::shared_ptr<IRenderableGeometry> geometry;
    std::shared_ptr<Material> material;
};

class IScene {
public:
    virtual ~IScene() = default;
    
    virtual void Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual const std::vector<SceneObject>& GetObjects() const = 0;
    
    virtual void OnActivate() {}
    virtual void OnDeactivate() {}
};
