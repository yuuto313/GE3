#pragma once

// Vectorをまとめたヘッダー

/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 final {
	float x;
	float y;

	// オペレーター

	Vector2 operator+=(Vector2 position) {
		return Vector2(this->x += position.x, this->y += position.y);
	}

};

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 final {
	float x;
	float y;
	float z;

	// オペレーター
	Vector3 operator+(const Vector3& other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	Vector3 operator+=(const Vector3& other) {
		return Vector3(x += other.x, y += other.y, z += other.z);
	}

	Vector3 operator-(const Vector3& other) const
	{
		return Vector3(this->x - other.x, this->y - other.y, this->z - other.z);
	}

	Vector3 operator*(const float other) {
		return Vector3(this->x * other, this->y * other, this->z * other);
	}


};

#pragma once

/// <summary>
/// 4次元ベクトル
/// </summary>
struct Vector4 final {
	float x;
	float y;
	float z;
	float w;
};

