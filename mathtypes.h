#pragma once

static float DegreesToRadians(float degrees)
{
	return float((M_PI * degrees) / 180.0f);
}

struct Vector3
{
	float x;
	float y;
	float z;

	Vector3 operator- (const Vector3& rhs) const
	{
		return Vector3{ x - rhs.x, y - rhs.y, z - rhs.z };
	}

	Vector3 operator+ (const Vector3& rhs) const
	{
		return Vector3{ x + rhs.x, y + rhs.y, z + rhs.z };
	}

	Vector3 Normalize()
	{
		float length = sqrtf((x * x) + (y * y) + (z * z));
		if (length > 0)
		{
			return Vector3{ x / length, y / length, z / length };
		}
		return *this;
	}

	float Dot(const Vector3& rhs)
	{
		return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
	}
};

using Color3 = Vector3;
using Ray = Vector3;

struct Sphere
{
	Vector3 center;
	float radius;
	Color3 color;

	bool Intersect(const Vector3& rayorig, const Vector3& raydir, float& t0, float& t1) const 
    { 
        Vector3 l = center - rayorig; 

        float tca = l.Dot(raydir); 

		if (tca < 0.0f)
		{
			return false;
		}

        float d2 = l.Dot(l) - tca * tca; 
		 
		if (d2 > (radius * radius))
		{
			return false;
		}
		
        float thc = sqrtf((radius * radius) - d2); 

        t0 = tca - thc; 
        t1 = tca + thc; 
 
        return true; 
    } 
};