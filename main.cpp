#include <Novice.h>
#include "MyMath.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include<imgui.h>

const char kWindowTitle[] = "LE2C_07_オオサキ_ハルキ_タイトル";

MyMath* myMath = new MyMath();

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result{};

	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;

	return result;
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHandleWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHandleWidth * 2.0f) / float(kSubdivision);

	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHandleWidth + (xIndex * kGridEvery);
		unsigned int color = 0xAAAAAAFF;
		Vector3 start{ x,0.0f,-kGridHandleWidth };
		Vector3 end{ x,0.0f,kGridHandleWidth };

		Vector3 startScreen = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);

		if (x == 0.0f) {
			color = BLACK;
		}
		Novice::DrawLine((int)startScreen.x, (int)startScreen.y, (int)endScreen.x, (int)endScreen.y, color);
	}
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHandleWidth + (zIndex * kGridEvery);
		unsigned int color = 0xAAAAAAFF;
		Vector3 start{ -kGridHandleWidth,0.0f,z };
		Vector3 end{ kGridHandleWidth,0.0f,z };

		Vector3 startScreen = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);

		if (z == 0.0f) {
			color = BLACK;
		}
		Novice::DrawLine((int)startScreen.x, (int)startScreen.y, (int)endScreen.x, (int)endScreen.y, color);

	}
}


Vector3 MultiplyPlane(float m1, Vector3 m2) {
	Vector3  result{};
	result.x = m1 * m2.x;
	result.y = m1 * m2.y;
	result.z = m1 * m2.z;

	return result;
}


struct Segment {
	Vector3 origin;//始点
	Vector3 diff;  //終点　差分ベクトル
};


//三角
struct Triangle {
	Vector3 vertices[3];
	Vector3 normal;
	float distance;
};

Vector3 Normalize(const Vector3& v) {
	Vector3 result;
	result.x = v.x / (float)sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	result.y = v.y / (float)sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	result.z = v.z / (float)sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	return result;
}

Vector3 Add(Vector3& m1, Vector3& m2) {
	Vector3 result;
	result.x = m1.x + m2.x;
	result.y = m1.y + m2.y;
	result.z = m1.z + m2.z;
	return result;
}

Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y,vector.x,0.0f };
	}
	return { 0.0f,-vector.z,vector.y };
}

Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result{};
	result.x = (v1.y * v2.z) - (v1.z * v2.y);
	result.y = (v1.z * v2.x) - (v1.x * v2.z);
	result.z = (v1.x * v2.y) - (v1.y * v2.x);
	return result;
}


void DrawTriangle(const Triangle& triangle ,const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {

	Vector3 T0 = Transform(Transform(triangle.vertices[0], viewProjectionMatrix), viewportMatrix);
	Vector3 T1 = Transform(Transform(triangle.vertices[1], viewProjectionMatrix), viewportMatrix);
	Vector3 T2 = Transform(Transform(triangle.vertices[2], viewProjectionMatrix), viewportMatrix);

	Novice::DrawLine(int(T0.x), int(T0.y), int(T1.x), int(T1.y), WHITE);
	Novice::DrawLine(int(T1.x), int(T1.y), int(T2.x), int(T2.y), WHITE);
	Novice::DrawLine(int(T2.x), int(T2.y), int(T0.x), int(T0.y), WHITE);
}


float Dot(const Vector3& v1, const Vector3& v2) {
	float result;
	result = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	return result;
}


bool IsCollision(const Segment& s1, const Triangle& v2) {

	Vector3 v0p = { v2.vertices[0].x - s1.origin.x,v2.vertices[0].y - s1.origin.y,v2.vertices[0].z - s1.origin.z };
	Vector3 v1p = { v2.vertices[1].x - s1.origin.x,v2.vertices[1].y - s1.origin.y,v2.vertices[1].z - s1.origin.z };
	Vector3 v2p = { v2.vertices[2].x - s1.origin.x,v2.vertices[2].y - s1.origin.y,v2.vertices[2].z - s1.origin.z };

	float t0 = (((v0p.x + v0p.y + v0p.z) * (s1.diff.x + s1.diff.y + s1.diff.z)) / ((s1.diff.x + s1.diff.y + s1.diff.z) * (s1.diff.x + s1.diff.y + s1.diff.z)));
	float t1 = (((v1p.x + v1p.y + v1p.z) * (s1.diff.x + s1.diff.y + s1.diff.z)) / ((s1.diff.x + s1.diff.y + s1.diff.z) * (s1.diff.x + s1.diff.y + s1.diff.z)));
	float t2 = (((v2p.x + v2p.y + v2p.z) * (s1.diff.x + s1.diff.y + s1.diff.z)) / ((s1.diff.x + s1.diff.y + s1.diff.z) * (s1.diff.x + s1.diff.y + s1.diff.z)));


	Vector3 vertex[3]{};


	vertex[0].x = s1.origin.x + (t0 * s1.diff.x);
	vertex[0].y = s1.origin.y + (t0 * s1.diff.y);	
	vertex[0].z = s1.origin.z + (t0 * s1.diff.z);

	vertex[1].x = s1.origin.x + (t1 * s1.diff.x);
	vertex[1].y = s1.origin.y + (t1 * s1.diff.y);
	vertex[1].z = s1.origin.z + (t1 * s1.diff.z);

	vertex[2].x = s1.origin.x + (t2 * s1.diff.x);
	vertex[2].y = s1.origin.y + (t2 * s1.diff.y);
	vertex[2].z = s1.origin.z + (t2 * s1.diff.z);
 

	Vector3 cross01 = Cross(v2.vertices[0], vertex[1]);
	Vector3 cross12 = Cross(v2.vertices[1], vertex[2]);
	Vector3 cross20 = Cross(v2.vertices[2], vertex[0]);



	if (Dot(cross01, v2.normal) >= 0.0f &&
		Dot(cross12, v2.normal) >= 0.0f &&
		Dot(cross20, v2.normal) >= 0.0f) {
		return true;
	}


	return false;
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 cameraPosition = { 0.0f ,0.0f,-20.0f };
	Vector3 cameraTranslate = { 0.0f,-1.0f,-6.49f };
	Vector3 cameraRotate = { -0.16f,0.0f,0.0f };


	Segment segment{ {-2.0f,-1.0f,0.0f} ,{3.0f,2.0f,2.0f} };
	
	Triangle triangle = {
		.vertices
		{
		{ -0.5f ,0.0f ,0.0f  },
		{ 0.0f , 1.0f , 0.0f  },
		{ 0.5f ,0.0f ,0.0f  }
		}
	};

	triangle.normal = { -0.5f,0.0f,0.0f };
	triangle.distance = 0.0f;

	
	uint32_t Color = WHITE;


	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///
		
		Matrix4x4 worldMatrix = myMath->MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 cameraMatrix = myMath->MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, cameraPosition);
		Matrix4x4 viewMatrix = myMath->Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = myMath->MakePerspectiveFovMatrix(0.45f, float(1280.0f) / float(720.0f), 0.1f, 100.0f);
		Matrix4x4 WorldViewProjectionMatrix = myMath->Multiply(worldMatrix, myMath->Multiply(viewMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = myMath->MakeViewportMatrix(0, 0, float(1280.0f), float(720.0f), 0.0f, 1.0f);


		DrawGrid(WorldViewProjectionMatrix, viewportMatrix);

		DrawTriangle(triangle,WorldViewProjectionMatrix, viewportMatrix);




		bool distanceFlag = IsCollision(segment,triangle);

		if (distanceFlag == true) {
			Color = RED;
		}
		else {
			Color = WHITE;
		}

		Vector3 start = Transform(Transform(segment.origin, WorldViewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform(Add(segment.origin, segment.diff), WorldViewProjectionMatrix), viewportMatrix);


		Novice::DrawLine((int)start.x, (int)start.y, (int)end.x, (int)end.y, Color);



		ImGui::Begin("window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);

		ImGui::DragFloat3("shaderCenter[0]", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("shaderRadius[0]", &segment.diff.x, 0.01f);

		ImGui::DragFloat3("triangle 0", &triangle.vertices[0].x, 0.01f);
		ImGui::DragFloat3("triangle 1", &triangle.vertices[1].x, 0.01f);
		ImGui::DragFloat3("triangle 2", &triangle.vertices[2].x, 0.01f);


		ImGui::End();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
