#include "H_Include.h"

#define M_PI 3.14159265358979323846

class Vector
{

public:
	float x;
	float y;
	float z;
};

class C_Math
{
public:
	float ATAN2(float x, float y)
	{
		if (y < 0){
			return -ATAN2(x, -y);
		}
		else if (x < 0){
			return M_PI - atan(-y / x);
		}
		else if (x > 0){
			return atan(y / x);
		}
		else if (y != 0){
			return M_PI / 2;
		}
	}

	float VectorLength(float *v)
	{
		return (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	}

	void AngleVectors(float *angles, float *forward, float *right, float *up)
	{
		float angle;
		static float sp, sy, cp, cy;
		angle = angles[0] * (M_PI / 180);
		sp = sin(angle);
		cp = cos(angle);
		angle = angles[1] * (M_PI / 180);
		sy = sin(angle);
		cy = cos(angle);

		if (forward)
		{
			forward[0] = cp*cy;
			forward[1] = cp*sy;
			forward[2] = -sp;
		}

		if (right || up)
		{
			static float sr, cr;
			angle = angles[2] * (M_PI / 180);
			sr = sin(angle);
			cr = cos(angle);

			if (right)
			{
				right[0] = -1 * sr*sp*cy + -1 * cr*-sy;
				right[1] = -1 * sr*sp*sy + -1 * cr*cy;
				right[2] = -1 * sr*cp;
			}

			if (up)
			{
				up[0] = cr*sp*cy + -sr*-sy;
				up[1] = cr*sp*sy + -sr*cy;
				up[2] = cr*cp;
			}
		}
	}

	float  VectorNormalize(Vector& vec)
	{
		float radius = sqrtf(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
		float iradius = 1.f / (radius + FLT_EPSILON);
		vec.x *= iradius;
		vec.y *= iradius;
		vec.z *= iradius;
		return radius;
	}

	void VectorAngles(const float *forward, float *angles)
	{
		float tmp, yaw, pitch;
		if (forward[1] == 0 && forward[0] == 0)
		{
			yaw = 0;
			if (forward[2] > 0)
				pitch = 270;
			else
				pitch = 90;
		}
		else
		{
			yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;
			tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
			pitch = (atan2(-forward[2], tmp) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	}

	void NormalizeAngles(float *angle)
	{
		for (int axis = 2; axis >= 0; --axis)
		{
			while (angle[axis] > 180.f)
			{
				angle[axis] -= 360.f;
			}
			while (angle[axis] < -180.f)
			{
				angle[axis] += 360.f;
			}
		}
	}

}; extern C_Math g_Math;