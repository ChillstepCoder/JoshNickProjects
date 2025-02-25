// IPhysicsUserData.h

#pragma once
class IPhysicsUserData {
public:
  virtual ~IPhysicsUserData() = default;
  virtual bool isCar() const = 0;
};
