#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <string>
#include <cmath>

struct MyBoundingBox {
	Vector3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
	Vector3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
};


struct Object {

	Model model = {};
	Vector3 pos = { 0,0,0 };
	Vector3 rotationAxis = { 0,1,0 };
	float rotationAngle = 0;
	MyBoundingBox collider = {};
	bool colliding = false;
	bool needUpdate = true;
	Color color = BLUE;
};

MyBoundingBox GetBoundingBox(const Object& object)
{
	MyBoundingBox box;
	box.min = { FLT_MAX, FLT_MAX, FLT_MAX };
	box.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (int i = 0; i < object.model.meshCount; i++)
	{
		Mesh mesh = object.model.meshes[i];

		for (int j = 0; j < mesh.vertexCount; j++)
		{
			Vector3 vertex = { mesh.vertices[j * 3 + 0], mesh.vertices[j * 3 + 1], mesh.vertices[j * 3 + 2] };
			Vector3 rotated = Vector3RotateByAxisAngle(vertex, object.rotationAxis, object.rotationAngle * DEG2RAD);

			box.min.x = fminf(box.min.x, rotated.x);
			box.min.y = fminf(box.min.y, rotated.y);
			box.min.z = fminf(box.min.z, rotated.z);
			box.max.x = fmaxf(box.max.x, rotated.x);
			box.max.y = fmaxf(box.max.y, rotated.y);
			box.max.z = fmaxf(box.max.z, rotated.z);
		}
	}
	return box;
}

bool BoundingBoxCollision(MyBoundingBox& a, Vector3 aPos, MyBoundingBox& b, Vector3 bPos)
{
	if (a.max.x + aPos.x < b.min.x + bPos.x) return false;
	if (a.min.x + aPos.x > b.max.x + bPos.x) return false;
	if (a.max.y + aPos.y < b.min.y + bPos.y) return false;
	if (a.min.y + aPos.y > b.max.y + bPos.y) return false;
	if (a.max.z + aPos.z < b.min.z + bPos.z) return false;
	if (a.min.z + aPos.z > b.max.z + bPos.z) return false;
	return true;
}


bool IsPointInside(Vector3 point, Object object) {

	Vector3 localPoint = Vector3Subtract(point, object.pos);

	for (int m = 0; m < object.model.meshCount; m++)
	{
		Mesh& mesh = object.model.meshes[m];

		if (mesh.vertexCount <= 0 || mesh.normals == nullptr || mesh.vertices == nullptr) return false;

		for (int j = 0; j < mesh.vertexCount; j++)
		{
			Vector3 vertex = {
				mesh.vertices[j * 3 + 0],
				mesh.vertices[j * 3 + 1],
				mesh.vertices[j * 3 + 2]
			};

			Vector3 normal = {
				mesh.normals[j * 3 + 0],
				mesh.normals[j * 3 + 1],
				mesh.normals[j * 3 + 2]
			};

			float angleRad = object.rotationAngle * DEG2RAD;
			Vector3 vertexRot = Vector3RotateByAxisAngle(vertex, object.rotationAxis, angleRad);
			Vector3 normalRot = Vector3RotateByAxisAngle(normal, object.rotationAxis, angleRad);

			Vector3 toPoint = Vector3Subtract(localPoint, vertexRot);

			float dot = Vector3DotProduct(normalRot, toPoint);

			if (dot < -EPSILON)
			{
				return false;
			}
		}
	}

	return true;
}


int main()
{
	const int screenWidth = 1600;
	const int screenHeight = 900;

	Vector3 gridSize = { 5.0f, 5.0f, 5.0f };

	InitWindow(screenWidth, screenHeight, "TP03_Algebra");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	Camera camera = { 0 };
	camera.position = { 20.0f, 15.0f, 20.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	std::vector<Model> models;

	Model cube = LoadModel("res/cube.obj");
	models.push_back(cube);
	Model decahedron = LoadModel("res/decahedron.obj");
	models.push_back(decahedron);
	Model dodecahedron = LoadModel("res/dodecahedron.obj");
	models.push_back(dodecahedron);
	Model icosahedron = LoadModel("res/icosahedron.obj");
	models.push_back(icosahedron);
	Model octahedron = LoadModel("res/octahedron.obj");
	models.push_back(octahedron);
	Model tetrahedron = LoadModel("res/tetrahedron.obj");
	models.push_back(tetrahedron);

	std::vector<Object> objects;

	for (int i = 0; i < 50; i++)
	{
		Object objRand;

		objRand.model = models[GetRandomValue(0, models.size() - 1)];
		objRand.pos = { (float)(GetRandomValue(-50, 50)), 0.0f, (float)(GetRandomValue(-50, 50)) };
		objRand.collider = GetBoundingBox(objRand);
		objects.push_back(objRand);
	}

	int currentObject = 0;
	Object& obj = objects[currentObject];

	objects[currentObject] = obj;
	obj = objects[currentObject];

	DisableCursor();

	while (!WindowShouldClose())
	{
		UpdateCamera(&camera, CAMERA_FREE);
		float deltaTime = GetFrameTime();
		float moveSpeed = 10.0f;
		float rotateSpeed = 50.0f;

		for (int i = 0; i < objects.size(); i++)
		{
			objects[i].colliding = false;
			objects[i].color = BLUE;
		}

		if (IsKeyPressed(KEY_TAB))
		{
			objects[currentObject] = obj;
			currentObject++;
			if (currentObject >= objects.size())
			{
				currentObject = 0;
			}

			obj = objects[currentObject];
		}

		if (IsKeyPressed(KEY_LEFT_SHIFT))
		{
			objects[currentObject] = obj;
			currentObject--;
			if (currentObject < 0)
			{
				currentObject = objects.size() - 1;
			}

			obj = objects[currentObject];
		}

		if (IsKeyDown(KEY_T))
		{
			obj.pos.z -= moveSpeed * deltaTime;
		}
		if (IsKeyDown(KEY_G))
		{
			obj.pos.z += moveSpeed * deltaTime;
		}

		if (IsKeyDown(KEY_F))
		{
			obj.pos.x -= moveSpeed * deltaTime;
		}
		if (IsKeyDown(KEY_H))
		{
			obj.pos.x += moveSpeed * deltaTime;
		}

		if (IsKeyDown(KEY_U))
		{
			obj.pos.y -= moveSpeed * deltaTime;
		}
		if (IsKeyDown(KEY_J))
		{
			obj.pos.y += moveSpeed * deltaTime;
		}

		if (IsKeyDown(KEY_R))
		{
			obj.rotationAngle += rotateSpeed * deltaTime;
			obj.needUpdate = true;
		}

		if (IsKeyDown(KEY_Y))
		{
			obj.rotationAngle -= rotateSpeed * deltaTime;
			obj.needUpdate = true;
		}

		if (obj.needUpdate)
		{
			obj.collider = GetBoundingBox(obj);
			obj.needUpdate = false;
		}

		int collisions = 0;

		std::vector<Vector3> points;

		for (int i = 0; i < objects.size(); i++)
		{
			if (objects[i].needUpdate)
			{
				objects[i].collider = GetBoundingBox(objects[i]);
				objects[i].needUpdate = false;
			}

			for (int j = i + 1; j < objects.size(); j++) {

				if (BoundingBoxCollision(objects[j].collider, objects[j].pos, objects[i].collider, objects[i].pos))
				{
					for (int x = -gridSize.x/2; x < gridSize.x/2; x++)
					{
						for (int y = -gridSize.y / 2; y < gridSize.y/2; y++) {

							for (int z = -gridSize.z / 2; z < gridSize.z/2; z++) {

								Vector3 point = { x,y,z };
								point = Vector3Add(point,objects[i].pos);
								point.x = (int)point.x;
								point.y = (int)point.y;
								point.z = (int)point.z;


								points.push_back(point);

								if (IsPointInside(point, objects[i]) && IsPointInside(point, objects[j])) {
									objects[i].colliding = true;
									objects[j].colliding = true;
									objects[i].color = RED;
									objects[j].color = RED;
									collisions++;
								}
							}
						}
					}
				}
			}
		}

		obj.color = GREEN;

		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		DrawGrid(100, 1.0f);

		for (int i = 0; i < objects.size(); i++)
		{
			if (i == currentObject) continue;
			DrawModelEx(objects[i].model, objects[i].pos, objects[i].rotationAxis, objects[i].rotationAngle, { 1.0f,1.0f,1.0f }, objects[i].color);
			DrawModelWiresEx(objects[i].model, objects[i].pos, objects[i].rotationAxis, objects[i].rotationAngle, { 1.0f,1.0f,1.0f }, BLACK);

			DrawCubeWiresV(objects[i].pos, Vector3Subtract(objects[i].collider.max, objects[i].collider.min), MAGENTA);
		}
		DrawModelEx(obj.model, obj.pos, obj.rotationAxis, obj.rotationAngle, { 1.0f,1.0f,1.0f }, obj.color);
		DrawModelWiresEx(obj.model, obj.pos, obj.rotationAxis, obj.rotationAngle, { 1.0f,1.0f,1.0f }, BLACK);
		DrawCubeWiresV(obj.pos, Vector3Subtract(obj.collider.max, obj.collider.min), MAGENTA);

		for (int i = 0; i < points.size()-1; i+=2)
		{
			DrawSphere(points[i],0.25f,MAGENTA);
		}

		EndMode3D();

		DrawText(TextFormat("Colisions: %d", collisions), 10, 10, 20, BLACK);
		DrawText("WASD to Move. Mouse to Look", 10, 35, 15, DARKGRAY);
		DrawText("TFGH to move object. U/J up/down. R/Y to rotate", 10, 55, 15, DARKGRAY);
		DrawText("tab for next, shift for before", 10, 75, 15, DARKGRAY);

		EndDrawing();
	}

	UnloadModel(cube);
	UnloadModel(icosahedron);

	CloseWindow();

	return 0;
}