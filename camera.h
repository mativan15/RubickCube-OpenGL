#ifndef PROJECT11_CAMERA_H_INCLUDED
#define PROJECT11_CAMERA_H_INCLUDED

#include "transformations.h"
#include <cmath>

// Basic 3D vector.
class Vec3 {
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

inline Vec3 operator+(const Vec3& a, const Vec3& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Vec3 operator-(const Vec3& a, const Vec3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline Vec3 operator*(const Vec3& v, float s) {
    return {v.x * s, v.y * s, v.z * s};
}

inline Vec3 operator/(const Vec3& v, float s) {
    return {v.x / s, v.y / s, v.z / s};
}

inline float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline float length(const Vec3& v) {
    return std::sqrt(dot(v, v));
}

inline Vec3 normalize(const Vec3& v) {
    const float len = length(v);

    if (len < 1e-8f) {
        return {0.0f, 0.0f, 0.0f};
    }

    return v / len;
}

inline float radians(float deg) {
    return deg * (PI / 180.0f);
}

// Equivalent to gluLookAt.
inline Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& upVector) {
    const Vec3 f = normalize(center - eye);
    const Vec3 s = normalize(cross(f, upVector));
    const Vec3 u = cross(s, f);

    Mat4 view = Mat4::identity();

    view.m[0] = s.x;
    view.m[1] = s.y;
    view.m[2] = s.z;

    view.m[4] = u.x;
    view.m[5] = u.y;
    view.m[6] = u.z;

    view.m[8]  = -f.x;
    view.m[9]  = -f.y;
    view.m[10] = -f.z;

    view.m[12] = -dot(s, eye);
    view.m[13] = -dot(u, eye);
    view.m[14] =  dot(f, eye);

    return view;
}

inline Mat4 gluLookAt(const Vec3& eye, const Vec3& center, const Vec3& upVector) {
    return lookAt(eye, center, upVector);
}

// Equivalent to gluPerspective.
inline Mat4 perspective(float fovyDeg, float aspect, float zNear, float zFar) {
    const float f = 1.0f / std::tan(radians(fovyDeg) * 0.5f);

    Mat4 p{};

    p.m[0] = f / aspect;
    p.m[5] = f;
    p.m[10] = (zFar + zNear) / (zNear - zFar);
    p.m[11] = -1.0f;
    p.m[14] = (2.0f * zFar * zNear) / (zNear - zFar);

    return p;
}

inline Mat4 gluPerspective(float fovyDeg, float aspect, float zNear, float zFar) {
    return perspective(fovyDeg, aspect, zNear, zFar);
}

enum class CameraMove {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera {
public:
    Vec3 position{0.0f, 0.0f, 4.0f};

    Vec3 front{0.0f, 0.0f, -1.0f};
    Vec3 up{0.0f, 1.0f, 0.0f};
    Vec3 right{1.0f, 0.0f, 0.0f};
    Vec3 worldUp{0.0f, 1.0f, 0.0f};

    float yawDeg = -90.0f;
    float pitchDeg = 0.0f;

    float moveSpeed = 2.5f;
    float mouseSensitivity = 0.1f;
    float zoomFovDeg = 45.0f;

    Vec3 orbitTarget{0.0f, 0.0f, 0.0f};
    float orbitDistance = 4.0f;
    float orbitYawDeg = -90.0f;
    float orbitPitchDeg = 0.0f;
    float orbitSpeedDeg = 80.0f;

    bool useDirectView = false;
    Vec3 directCenter{0.0f, 0.0f, 0.0f};

    Camera() {
        updateVectors();
    }

    explicit Camera(const Vec3& startPos) : position(startPos) {
        updateVectors();
        syncOrbitFromPosition();
    }

    Mat4 viewMatrix() const {
        if (useDirectView) {
            return lookAt(position, directCenter, up);
        }

        return lookAt(position, position + front, up);
    }

    void move(CameraMove dir, float deltaTime) {
        useDirectView = false;

        const float v = moveSpeed * deltaTime;

        if (dir == CameraMove::Forward) {
            position = position + front * v;
        }

        if (dir == CameraMove::Backward) {
            position = position - front * v;
        }

        if (dir == CameraMove::Left) {
            position = position - right * v;
        }

        if (dir == CameraMove::Right) {
            position = position + right * v;
        }

        if (dir == CameraMove::Up) {
            position = position + worldUp * v;
        }

        if (dir == CameraMove::Down) {
            position = position - worldUp * v;
        }

        syncOrbitFromPosition();
    }

    void rotateFromMouse(float xOffset, float yOffset, bool constrainPitch = true) {
        useDirectView = false;

        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;

        yawDeg += xOffset;
        pitchDeg += yOffset;

        if (constrainPitch) {
            if (pitchDeg > 89.0f) {
                pitchDeg = 89.0f;
            }

            if (pitchDeg < -89.0f) {
                pitchDeg = -89.0f;
            }
        }

        updateVectors();
        syncOrbitFromPosition();
    }

    void zoomFromScroll(float yOffset) {
        zoomFovDeg -= yOffset;

        if (zoomFovDeg < 1.0f) {
            zoomFovDeg = 1.0f;
        }

        if (zoomFovDeg > 90.0f) {
            zoomFovDeg = 90.0f;
        }
    }

    // Sets an exact camera view, useful for F/B/L/R/U/Q.
    void setDirectView(const Vec3& eye, const Vec3& center, const Vec3& upVector) {
        useDirectView = true;

        position = eye;
        directCenter = center;
        orbitTarget = center;
        orbitDistance = length(eye - center);

        front = normalize(center - eye);

        Vec3 safeUp = upVector;

        if (length(cross(front, safeUp)) < 1e-6f) {
            safeUp = {0.0f, 0.0f, 1.0f};
        }

        right = normalize(cross(front, safeUp));
        up = normalize(cross(right, front));

        syncOrbitFromPosition();
    }

    // Keeps compatibility with yaw/pitch orbit placement.
    void setOrbit(float yaw, float pitch, float distance = 4.0f) {
        useDirectView = false;

        orbitYawDeg = yaw;
        orbitPitchDeg = pitch;
        orbitDistance = distance;

        normalizeOrbitAngles();
        updateOrbitPosition();
    }

    // Trackball orbit: A/D around world Y, W/S around camera right.
    void orbit(float deltaYaw, float deltaPitch) {
        useDirectView = false;

        Vec3 offset = position - orbitTarget;

        if (length(offset) < 1e-6f) {
            offset = {0.0f, 0.0f, orbitDistance};
        }

        if (std::fabs(deltaYaw) > 1e-6f) {
            offset = rotateAroundAxis(offset, worldUp, radians(deltaYaw));
        }

        if (std::fabs(deltaPitch) > 1e-6f) {
            Vec3 pitchAxis = right;

            if (length(pitchAxis) < 1e-6f) {
                pitchAxis = {1.0f, 0.0f, 0.0f};
            }

            offset = rotateAroundAxis(offset, pitchAxis, radians(deltaPitch));
        }

        position = orbitTarget + offset;
        orbitDistance = length(offset);

        updateViewFromPosition();
    }

    void normalizeOrbitAngles() {
        while (orbitYawDeg > 360.0f) {
            orbitYawDeg -= 360.0f;
        }

        while (orbitYawDeg < -360.0f) {
            orbitYawDeg += 360.0f;
        }

        while (orbitPitchDeg > 360.0f) {
            orbitPitchDeg -= 360.0f;
        }

        while (orbitPitchDeg < -360.0f) {
            orbitPitchDeg += 360.0f;
        }
    }

    void updateOrbitPosition() {
        const float yaw = radians(orbitYawDeg);
        const float pitch = radians(orbitPitchDeg);

        position.x = orbitTarget.x + orbitDistance * std::cos(pitch) * std::cos(yaw);
        position.y = orbitTarget.y + orbitDistance * std::sin(pitch);
        position.z = orbitTarget.z + orbitDistance * std::cos(pitch) * std::sin(yaw);

        updateViewFromPosition();
    }

    void syncOrbitFromPosition() {
        Vec3 offset = position - orbitTarget;
        orbitDistance = length(offset);

        if (orbitDistance < 1e-6f) {
            orbitDistance = 4.0f;
            offset = {0.0f, 0.0f, orbitDistance};
        }

        Vec3 dir = normalize(offset);

        orbitPitchDeg = std::asin(dir.y) * 180.0f / PI;
        orbitYawDeg = std::atan2(dir.z, dir.x) * 180.0f / PI;
    }

    // Rodrigues rotation formula.
    Vec3 rotateAroundAxis(const Vec3& v, const Vec3& axis, float angleRad) {
        Vec3 a = normalize(axis);

        float c = std::cos(angleRad);
        float s = std::sin(angleRad);

        return v * c + cross(a, v) * s + a * dot(a, v) * (1.0f - c);
    }

    // Updates front/right/up without flipping near vertical views.
    void updateViewFromPosition() {
        front = normalize(orbitTarget - position);

        Vec3 projectedUp = up - front * dot(up, front);

        if (length(projectedUp) < 1e-6f) {
            projectedUp = worldUp - front * dot(worldUp, front);
        }

        if (length(projectedUp) < 1e-6f) {
            projectedUp = {0.0f, 0.0f, 1.0f};
        }

        up = normalize(projectedUp);
        right = normalize(cross(front, up));
        up = normalize(cross(right, front));

        syncOrbitFromPosition();
    }

private:
    void updateVectors() {
        const float yaw = radians(yawDeg);
        const float pitch = radians(pitchDeg);

        Vec3 f{
            std::cos(yaw) * std::cos(pitch),
            std::sin(pitch),
            std::sin(yaw) * std::cos(pitch)
        };

        front = normalize(f);
        right = normalize(cross(front, worldUp));
        up = normalize(cross(right, front));
    }
};

#endif