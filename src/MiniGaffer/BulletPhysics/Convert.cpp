#include <Mini/BulletPhysics/Convert.h>


namespace MiniGaffer
{
    btTransform Convert(const Imath::M44f& transform)
    {
        btMatrix3x3 rotationPart {transform[0][0], transform[0][1], transform[0][2],
                                  transform[1][0], transform[1][1], transform[1][2],
                                  transform[2][0], transform[2][1], transform[2][2]};
        btVector3 translationPart (transform.translation().x, transform.translation().y, transform.translation().z);

        return btTransform(rotationPart, translationPart);
    }

    Imath::M44f Convert(const btTransform& transform)
    {
        Imath::Quatf  quat { transform.getRotation().w(), transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z() };
        Imath::M44f result = quat.toMatrix44();

        result[3][0] = transform.getOrigin().x();
        result[3][1] = transform.getOrigin().y();
        result[3][2] = transform.getOrigin().z();
        return result;
    }
}