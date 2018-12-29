#include "../Externals/Include/Include.h"
#include "../Externals/Include/assimp/Importer.hpp"
#include "../Externals/Include/assimp/scene.h"
#include "../Externals/Include/assimp/postprocess.h"
#include "../Externals/Include/Camera.h"
#include "../Externals/Include/Program.h"

#define _CRT_SECURE_NO_WARNINGS
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define SKYBOX_SIZE 500
#define WINDOW_WIDTH 600//1440
#define WINDOW_HEIGHT 600//900
#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024

GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

using namespace glm;
using namespace std;

// pos(0, 0, 0) front(-1, -1, 0)
Camera camera(vec3(0.0f, 0.0f, 5.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0, 0.0, -1.0), WINDOW_WIDTH, WINDOW_HEIGHT, 80.0);

// define some stucte
struct Shape
{
	GLuint vao;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_texcoord;
	GLuint ibo;
	int drawCount;
	int materialID;
};

struct Material
{
	GLuint diffuse_tex;
};

struct MVP_mat
{
	mat4 proj;
	mat4 view;
	mat4 model;
	string model_name;
	string view_name;
	string proj_name;
};

GLint um4mv;
GLint um4p;

// model
// mat4 proj;
// mat4 view;
// mat4 model;
// GLuint program;

// object
Program obj_program;
vector<Shape> obj_shapes;
vector<Material> obj_materials;
mat4 obj_proj;
mat4 obj_view;
mat4 obj_model;

MVP_mat obj_mvp;

// FB
GLuint programFB;
GLuint window_vao;
GLuint window_buffer;
GLuint FBO;
GLuint depthRBO;
GLuint FBODataTexture;

// cubemap
// GLuint cubemap_program;
Program cubemap_program;
GLuint cubemap_vao;
GLuint cubemap_vbo;
mat4 cubemap_proj;
mat4 cubemap_view;
mat4 cubemap_model;
GLuint cubemap_tex;

// quad
Program quad_program;
GLuint quad_vao;
GLuint quad_vbo;
mat4 quad_model;
// get quad_view and quad_proj by camera.
MVP_mat quad_mvp;

// depth tex
Program depth_program;
GLuint depth_fbo;
GLuint depth_tex;
MVP_mat depth_mvp;

// depth map
Program depthmap_program;

// debug depth
Program debug_depth_program;


static const GLfloat cubemap_positions[] =
{
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

// 4096 * 4096
vector<std::string> faces = 
{
	"cubemaps/face-r.png",
	"cubemaps/face-l.png",
	"cubemaps/face-t.png",
	"cubemaps/face-d.png",
	"cubemaps/face-b.png",
	"cubemaps/face-f.png"
};

// 512 * 512
vector<std::string> faces_s =
{
	"cubemaps_s/face-r.png",
	"cubemaps_s/face-l.png",
	"cubemaps_s/face-t.png",
	"cubemaps_s/face-d.png",
	"cubemaps_s/face-b.png",
	"cubemaps_s/face-f.png"
};

static const GLfloat quad_positions[] = 
{
 	 1,  0, -1,
	-1,  0, -1,
	 1,  0,  1,
	-1,  0, -1,
	 1,  0,  1,
	-1,  0,  1
};

static const GLfloat window_positions[] =
{
	1.0f,-1.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,0.0f,
	-1.0f,1.0f,0.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f
};

// debug
GLuint quadVAO = 0;
GLuint quadVBO;
GLfloat quadVertices[] = {
	// Positions        // Texture Coords
	-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
	1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
};

// proto-type
void My_Reshape(int width, int height);

/*
char** loadShaderSource(const char* file)
{
    FILE* fp = fopen(file, "rb");
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *src = new char[sz + 1];
    fread(src, sizeof(char), sz, fp);
    src[sz] = '\0';
    char **srcp = new char*[1];
    srcp[0] = src;
    return srcp;
}

void freeShaderSource(char** srcp)
{
    delete[] srcp[0];
    delete[] srcp;
}
*/

// define a simple data structure for storing texture image raw data
typedef struct _TextureData
{
    _TextureData(void) :
        width(0),
        height(0),
        data(0)
    {
    }

    int width;
	int height;
    unsigned char* data;
} TextureData;

// load a png image and return a TextureData structure with raw data
// not limited to png format. works with any image format that is RGBA-32bit
TextureData loadPNG(const char* const pngFilepath, int channels)
{
    TextureData texture;
    int components;

    // load the texture with stb image, force RGBA (4 components required)
    stbi_uc *data = stbi_load(pngFilepath, &texture.width, &texture.height, &components, channels);

    // is the image successfully loaded?
    if (data != NULL)
    {
        // copy the raw data
        size_t dataSize = texture.width * texture.height * channels * sizeof(unsigned char);
        texture.data = new unsigned char[dataSize];
        memcpy(texture.data, data, dataSize);

        // mirror the image vertically to comply with OpenGL convention
        for (size_t i = 0; i < texture.width; ++i)
        {
            for (size_t j = 0; j < texture.height / 2; ++j)
            {
                for (size_t k = 0; k < channels; ++k)
                {
                    size_t coord1 = (j * texture.width + i) * channels + k;
                    size_t coord2 = ((texture.height - j - 1) * texture.width + i) * channels + k;
                    std::swap(texture.data[coord1], texture.data[coord2]);
                }
            }
        }

        // release the loaded image
        stbi_image_free(data);
    }

    return texture;
}

/*
GLuint My_LoadShader(const char *vsName, const char *fsName)
{
	// Create Shader Program
	GLuint program = glCreateProgram();

	// Create customize shader by tell openGL specify shader type 
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	// Load shader file
	char** vsSource = loadShaderSource(vsName);
	char** fsSource = loadShaderSource(fsName);

	// Assign content of these shader files to those shaders we created before
	glShaderSource(vs, 1, vsSource, NULL);
	glShaderSource(fs, 1, fsSource, NULL);

	// Free the shader file string(won't be used any more)
	freeShaderSource(vsSource);
	freeShaderSource(fsSource);

	// Compile these shaders
	glCompileShader(vs);
	glCompileShader(fs);

	// Assign the program we created before with these shaders
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	// Tell OpenGL to use this shader program now
	// glUseProgram(program);

	return program;
}
*/

void My_LoadModels(const char *objName, vector<Shape> &obj_shapes, vector<Material> &obj_materials)
{
	const aiScene *scene = aiImportFile(objName, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene == NULL) {
		std::cout << "error scene load\n";
		return ;
	}
	else
		std::cout << "load " << objName << " sucess\n"
				  << "number of meshs = " << scene->mNumMeshes << "\n"
				  << "number of materials = " << scene->mNumMaterials << "\n";

	//Material material;
	aiString texturePath;
	//total = scene->mNumMeshes;
	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial *material = scene->mMaterials[i];
		Material materials;
		aiString texturePath;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
		{
			TextureData tdata = loadPNG(texturePath.C_Str(), 4);
			if (tdata.data != NULL)
				cout << "load material[ " << i << " ] : " << texturePath.C_Str() << " sucess\n";
			else
			{
				cout << "load material[ " << i << " ] fail\n";
				continue;
			}
			glGenTextures(1, &materials.diffuse_tex);
			glBindTexture(GL_TEXTURE_2D, materials.diffuse_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			cout << "material[ " << i << " ].texturePath is not found\n";
		}
		obj_materials.push_back(materials);

	}
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh *mesh = scene->mMeshes[i];
		Shape shape;
		glGenVertexArrays(1, &shape.vao);
		glBindVertexArray(shape.vao);

		vector<float>vertices;
		vector<float>texCoords;
		vector<float>normals;

		glGenBuffers(1, &shape.vbo_position);
		glGenBuffers(1, &shape.vbo_texcoord);
		glGenBuffers(1, &shape.vbo_normal);
		for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
		{
			vertices.push_back(mesh->mVertices[v].x);
			vertices.push_back(mesh->mVertices[v].y);
			vertices.push_back(mesh->mVertices[v].z);
			texCoords.push_back(mesh->mTextureCoords[0][v].x);
			texCoords.push_back(mesh->mTextureCoords[0][v].y);

			normals.push_back(mesh->mNormals[v].x);
			normals.push_back(mesh->mNormals[v].y);
			normals.push_back(mesh->mNormals[v].z);
		}

		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_position);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_texcoord);
		glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), &texCoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_normal);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		// ibo
		vector<unsigned int>indices;
		glGenBuffers(1, &shape.ibo);
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
		{

			indices.push_back(mesh->mFaces[f].mIndices[0]);
			indices.push_back(mesh->mFaces[f].mIndices[1]);
			indices.push_back(mesh->mFaces[f].mIndices[2]);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


		// glVertexAttribPointer / glEnableVertexArray calls¡K
		shape.materialID = mesh->mMaterialIndex;
		shape.drawCount = mesh->mNumFaces * 3;
		// save shape¡K

		obj_shapes.push_back(shape);
		cout << "shape[ " << i << " ].materialID = " << shape.materialID << "\n";
	}

	aiReleaseImport(scene);
}

GLuint load_cubemap(vector<std::string> faces)
{
	GLuint texture_ID;
	glGenTextures(1, &texture_ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_ID);

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		TextureData texture;
		texture = loadPNG(faces[i].c_str(), 3);
		
		if (texture.data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
					   	 texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.data);
			std::cout << "load " << faces[i] << " in Positive_x + " << i << "\n";
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	return texture_ID;
}

void My_Init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	if (false) {
		// ==============================================
		// cubemap 
		// cubemap_program = My_LoadShader("cubemap_vertex.vs.glsl", "cubemap_fragment.fs.glsl");
		cubemap_program.set_program("cubemap_vertex.vs.glsl", "cubemap_fragment.fs.glsl");

		// cubemap vao and vbo
		glGenVertexArrays(1, &cubemap_vao);
		glBindVertexArray(cubemap_vao);

		glGenBuffers(1, &cubemap_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, cubemap_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubemap_positions), cubemap_positions, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		// Get the id of inner variable 'proj' and 'view' in shader programs
		// cubemap_proj = glGetUniformLocation(cubemap_program, "proj");
		// cubemap_view = glGetUniformLocation(cubemap_program, "view");
		// cubemap_model = glGetUniformLocation(cubemap_program, "model");

		// cubemap texture
		// cubemap_tex = load_cubemap(faces);
		cubemap_tex = load_cubemap(faces_s);

		// seamless
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
	/*
	// ==============================================
	// object
	obj_program.set_program("object_vertex.vs.glsl", "object_fragment.fs.glsl");

	My_LoadModels("nanosuit.obj", obj_shapes, obj_materials);

	// ==============================================
	// quad
	quad_program.set_program("quad_vertex.vs.glsl", "quad_fragment.fs.glsl");

	// quad vao and vbo
	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_positions), quad_positions, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	*/
	/*
	// ==============================================
	// depth map
	depth_program.set_program("depth_vertex.vs.glsl", "depth_fragment.fs.glsl");
	debug_depth_program.set_program("debug_depth_vertex.vs.glsl", "debug_depth_fragment.fs.glsl");
	
	// frame buffer
	glGenFramebuffers(1, &depth_fbo);
	
	// depth texture
	glGenTextures(1, &depth_tex);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT,
				 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// bind depth texture to frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Setup plane VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	*/

	// others
	My_Reshape(WINDOW_WIDTH, WINDOW_HEIGHT);
	// glutWarpPointer(camera.aspect_w / 2, camera.aspect_h / 2);
}

// if need to send other uniform variables, send it before use draw_obj().
void draw_obj(Program program, GLuint vao, MVP_mat mvp, GLenum target, GLuint texture, bool mode, int count)
{
	program.use();
	
	glBindVertexArray(vao);

	program.set_mat4(mvp.model_name, mvp.model);
	program.set_mat4(mvp.view_name, mvp.view);
	program.set_mat4(mvp.proj_name, mvp.proj);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(target, texture);

	if (mode == 0)
	{
		glDrawArrays(GL_TRIANGLES, 0, count);
	}
	else if (mode == 1)
	{
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
}

void My_Display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera.get_delta_time();
	
	/*
	// ==============================================
	// draw cubemap
	// glUseProgram(cubemap_program);
	cubemap_program.use();

	glBindVertexArray(cubemap_vao);

	// model
	cubemap_model = scale(mat4(), vec3(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE));
	// glUniformMatrix4fv(cubemap_model, 1, GL_FALSE, value_ptr(model));
	cubemap_view = camera.get_view_matrix();
	cubemap_proj = camera.get_proj_matrix();

	// glUniformMatrix4fv(cubemap_view, 1, GL_FALSE, value_ptr(view));
	// glUniformMatrix4fv(cubemap_proj, 1, GL_FALSE, value_ptr(proj));
	cubemap_program.set_mat4("cubemap_model", cubemap_model);
	cubemap_program.set_mat4("cubemap_view", cubemap_view);
	cubemap_program.set_mat4("cubemap_proj", cubemap_proj);

	glActiveTexture(GL_TEXTURE0);
	// glUniform1i(glGetUniformLocation(cubemap_program.program_ID, "cubemap"), 0);
	cubemap_program.set_int("cubemap", 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_tex);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	*/
	// ==============================================
	// object
	/*
	obj_program.use();

	for (int i = 0; i < obj_shapes.size(); ++i)
	{
		// vao
		glBindVertexArray(obj_shapes[i].vao);

		// texture 
		// int materialID = Shapes[i].materialID;
		// glBindTexture(GL_TEXTURE_2D, Materials[materialID].diffuse_tex);
		// glActiveTexture(GL_TEXTURE0);

		// mvp
		// glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view *  model));
		// glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(proj));
		mat4 obj_model_scale = scale(mat4(), vec3(0.5, 0.35, 0.5));
		mat4 obj_model_trans = translate(mat4(), vec3(-10, -13, -8));
		obj_model = obj_model_trans * obj_model_scale;
		obj_view = camera.get_view_matrix();
		obj_proj = camera.get_proj_matrix();
		obj_program.set_mat4("obj_model", obj_model);
		obj_program.set_mat4("obj_view", obj_view);
		obj_program.set_mat4("obj_proj", obj_proj);

		// normal transformation
		mat4 obj_normal_trans = transpose(inverse(obj_model));
		obj_program.set_mat4("obj_normal_trans", obj_normal_trans);

		// environment map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_tex);

		// draw
		glDrawElements(GL_TRIANGLES, obj_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}
	*/

	/*
	for (int i = 0; i < obj_shapes.size(); ++i)
	{
		obj_mvp.model = translate(mat4(), vec3(-10, -13, -8)) * scale(mat4(), vec3(0.5, 0.35, 0.5));
		obj_mvp.view = camera.get_view_matrix();
		obj_mvp.proj = camera.get_proj_matrix();

		obj_mvp.model_name = string("obj_model");
		obj_mvp.view_name = string("obj_view");
		obj_mvp.proj_name = string("obj_proj");

		// normal transformation
		mat4 obj_normal_trans = transpose(inverse(obj_mvp.model));
		obj_program.set_mat4("obj_normal_trans", obj_normal_trans);
		
		draw_obj(obj_program, obj_shapes[i].vao, obj_mvp, GL_TEXTURE_CUBE_MAP, cubemap_tex, 1, obj_shapes[i].drawCount);
	}
	*/

	// ==============================================
	// quad
	/*
	quad_program.use();

	glBindVertexArray(quad_vao);

	float quad_size = 15;
	mat4 quad_model_scale = scale(mat4(), vec3(quad_size, quad_size, quad_size));
	mat4 quad_model_trans = translate(mat4(), vec3(-8, -13.5, 0));
	quad_model = quad_model_trans * quad_model_scale;
	quad_program.set_mat4("quad_model", quad_model);
	quad_program.set_mat4("quad_view", camera.get_view_matrix());
	quad_program.set_mat4("quad_proj", camera.get_proj_matrix());

	glDrawArrays(GL_TRIANGLES, 0, 6);
	*/

	/*
	float quad_size = 15;
	quad_mvp.model = translate(mat4(), vec3(-8, -13.5, 0)) * scale(mat4(), vec3(quad_size, quad_size, quad_size));
	quad_mvp.view = camera.get_view_matrix();
	quad_mvp.proj = camera.get_proj_matrix();

	quad_mvp.model_name = string("quad_model");
	quad_mvp.view_name = string("quad_view");
	quad_mvp.proj_name = string("quad_proj");

	draw_obj(quad_program, quad_vao, quad_mvp, NULL, NULL, 0, 6);
	*/

	// ==============================================
	// 1. render depth texture
	// clear buffers
	/*
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// draw obj's depth
	depth_program.use();

	vec3 obj_pos = vec3(-10, -13, -8);
	vec3 eye_pos = vec3(-31.75, 26.05, -97.72);
	const float shadow_range = 15.0f;

	depth_mvp.model = translate(mat4(), obj_pos) * scale(mat4(), vec3(0.5, 0.35, 0.5));
	depth_mvp.view = lookAt(eye_pos, obj_pos, vec3(0, 1, 0));
	depth_mvp.proj = ortho(-shadow_range, shadow_range, -shadow_range, shadow_range, 0.0f, 50.0f);
	depth_mvp.model_name = string("obj_model");
	depth_mvp.view_name = string("light_view");
	depth_mvp.proj_name = string("light_proj");

	for (int i = 0; i < obj_shapes.size(); ++i)
	{
		draw_obj(depth_program, obj_shapes[i].vao, depth_mvp, NULL, NULL, 1, obj_shapes[i].drawCount);
	}

	// draw quad's depth
	float quad_size = 15;
	depth_mvp.model = translate(mat4(), vec3(-8, -13.5, 0)) * scale(mat4(), vec3(quad_size, quad_size, quad_size));

	draw_obj(depth_program, quad_vao, depth_mvp, NULL, NULL, 0, 6);

	// 2. render depth texture to other quad
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	debug_depth_program.use();
	
	glBindVertexArray(quadVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depth_tex);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	*/

	glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	// view
	// view = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));

	// proj
	// float viewportAspect = (float)width / (float)height;
	// proj = perspective(radians(60.0f), viewportAspect, 0.1f, 1000.0f);
}

void My_Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(timer_speed, My_Timer, val);
}

void My_Mouse(int button, int state, int x, int y)
{
	if(state == GLUT_DOWN)
	{
		printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
	}
	else if(state == GLUT_UP)
	{
		printf("Mouse %d is released at (%d, %d)\n", button, x, y);
	}
}

void My_Keyboard(unsigned char key, int x, int y)
{
	camera.camera_movement(key);
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	/*
	cout << "camera position: " << camera.Position[0] << " " << camera.Position[1] << " " << camera.Position[2] << "\n"
		<< "camera front: " << camera.Front[0] << " " << camera.Front[1] << " " << camera.Front[2] << "\n"
		<< "delta time: " << camera.delta_time << "\n"
		<< "last frame: " << camera.last_frame << "\n";
	*/
}

void My_SpecialKeys(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch(id)
	{
	case MENU_TIMER_START:
		if(!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}

void detect_mouse(int x, int y)
{
	camera.camera_rotation(x, y);
	// cout << "in detect_mouse:" << x << " " << y << "\n";
}

void detect_mouse_scroll(int wheel, int dir, int x, int y)
{
	camera.camera_zoom(dir);
	// cout << "in detect_scroll:" << dir << " " << x << " " << y << "\n";
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
    // Change working directory to source code path
    chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("AS4_Framework"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
    glPrintContextInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0); 

	// camera
	glutPassiveMotionFunc(detect_mouse);
	glutMouseWheelFunc(detect_mouse_scroll);
	// glutSetCursor(GLUT_CURSOR_NONE);

	// Enter main event loop.
	glutMainLoop();

	return 0;
}
