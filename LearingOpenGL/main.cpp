#include<GL\glew.h>
#include<GL\freeglut.h>
#include<SOIL.h>
#include"glm.h"
#include"matrix.h"

//把shader源程序保存到字符数组
static char* readShaderSource(const char* shaderFile)
{
	FILE* fp;
	errno_t err = fopen_s(&fp, shaderFile, "r");
	if (err) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);

	buf[size] = NULL;
	fclose(fp);

	return buf;
}
//初始化shader
GLuint initShader(const char* vShaderFile, const char* fShaderFile)
{
	struct Shader
	{
		const char* filename;
		GLenum type;
		GLcharARB* source;
	}
	shaders[2] = {
		{ vShaderFile, GL_VERTEX_SHADER, NULL },
		{ fShaderFile, GL_FRAGMENT_SHADER, NULL }
	};
	//创建程序
	GLuint program = glCreateProgram();

	for (int i = 0; i < 2; ++i)
	{
		Shader& s = shaders[i];
		s.source = readShaderSource(s.filename);

		if (shaders[i].source == NULL)
		{
			cerr << "failed to read " << s.filename << endl;
			//exit(EXIT_FAILURE);
		}
		//创建shader对象、绑定shader源程序、编译shader
		GLuint shader = glCreateShader(s.type);
		glShaderSource(shader, 1, (const GLcharARB**)&s.source, NULL);
		glCompileShader(shader);
		//shader编译的错误检测
		GLint compile;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile);
		if (!compile)
		{
			cerr << "failed to compile " << s.filename << endl;
			GLint logSize;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
			char* logMsg = new char[logSize];
			glGetShaderInfoLog(shader, logSize, NULL, logMsg);
			cerr << logMsg << endl;
			delete[] logMsg;
			//exit(EXIT_FAILURE);
		}
		delete[] s.source;
		//将shader绑定到程序上
		glAttachShader(program, shader);
	}
	//链接程序，将着色器对象链接成一个可执行文件
	glLinkProgram(program);
	//链接程序的错误检测
	GLint link;
	glGetProgramiv(program, GL_COMPILE_STATUS, &link);
	if (!link)
	{
		cerr << "failed to link " << endl;
		GLint logSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetProgramInfoLog(program, logSize, NULL, logMsg);
		cerr << logMsg << endl;
		delete[] logMsg;
		//exit(EXIT_FAILURE);
	}
	return program;
}

typedef vec4 color4;
typedef vec4 point4;

char* modelPath = "luffy.obj";
GLMmodel* model = glmReadOBJ(modelPath);
GLMgroup* group = model->groups;
#define T(x) model->triangles[(x)]

int num = model->numtriangles * 3;

point4* points = new point4[num];
vec3* normals = new vec3[num];
vec2* texCoords = new vec2[num];

point4* vGround = new point4[6];
vec3* nGround = new vec3[6];
vec2* tGround = new vec2[6];

GLuint model_view;
GLuint projection;

float theta = 0;

//顶点数组对象
GLuint VAO[2];
//缓冲区对象
GLuint buffer[2];
//纹理对象
GLuint texture[2];

//初始化 
void init()
{

	//顶点数组对象
	glGenVertexArrays(2, VAO);
	//缓冲区对象
	glGenBuffers(2, buffer);
	//纹理对象
	glGenTextures(2, texture);

	//人物
	glmUnitize(model);
	int index = 0;
	while (group)
	{
		for (int i = 0;i < group->numtriangles;++i)
		{
			points[3 * index] = point4(model->vertices[3 * T(group->triangles[i]).vindices[0]],
				model->vertices[3 * T(group->triangles[i]).vindices[0] + 1],
				model->vertices[3 * T(group->triangles[i]).vindices[0] + 2],
				1.0);

			points[3 * index + 1] = point4(model->vertices[3 * T(group->triangles[i]).vindices[1]],
				model->vertices[3 * T(group->triangles[i]).vindices[1] + 1],
				model->vertices[3 * T(group->triangles[i]).vindices[1] + 2],
				1.0);

			points[3 * index + 2] = point4(model->vertices[3 * T(group->triangles[i]).vindices[2]],
				model->vertices[3 * T(group->triangles[i]).vindices[2] + 1],
				model->vertices[3 * T(group->triangles[i]).vindices[2] + 2],
				1.0);

			normals[3 * index] = vec3(model->normals[3 * T(group->triangles[i]).nindices[0]],
				model->normals[3 * T(group->triangles[i]).nindices[0] + 1],
				model->normals[3 * T(group->triangles[i]).nindices[0] + 2]);

			normals[3 * index + 1] = vec3(model->normals[3 * T(group->triangles[i]).nindices[1]],
				model->normals[3 * T(group->triangles[i]).nindices[1] + 1],
				model->normals[3 * T(group->triangles[i]).nindices[1] + 2]);

			normals[3 * index + 2] = vec3(model->normals[3 * T(group->triangles[i]).nindices[2]],
				model->normals[3 * T(group->triangles[i]).nindices[2] + 1],
				model->normals[3 * T(group->triangles[i]).nindices[2] + 2]);
	
				texCoords[3 * index] = vec2(model->texcoords[2 * T(group->triangles[i]).tindices[0]],
					model->texcoords[2 * T(group->triangles[i]).tindices[0] + 1]);

				texCoords[3 * index + 1] = vec2(model->texcoords[2 * T(group->triangles[i]).tindices[1]],
					model->texcoords[2 * T(group->triangles[i]).tindices[1] + 1]);

				texCoords[3 * index + 2] = vec2(model->texcoords[2 * T(group->triangles[i]).tindices[2]],
					model->texcoords[2 * T(group->triangles[i]).tindices[2] + 1]);
		
			index++;
		}
		group = group->next;
	}
	
	//地面
	vGround[0] = point4(-0.5,0.5,0.5,1.0);
	vGround[1] = point4(-0.5,-0.5,0.5,1.0);
	vGround[2] = point4(0.5,-0.5,0.5,1.0);
	vGround[3] = point4(-0.5, 0.5, 0.5, 1.0);
	vGround[4] = point4(0.5, -0.5, 0.5, 1.0);
	vGround[5] = point4(0.5, 0.5, 0.5, 1.0);

	nGround[0] = vec3(0, 1, 0);
	nGround[1] = vec3(0, 1, 0);
	nGround[2] = vec3(0, 1, 0);
	nGround[3] = vec3(0, 1, 0);
	nGround[4] = vec3(0, 1, 0);
	nGround[5] = vec3(0, 1, 0);

	tGround[0] = vec2(0,0);
	tGround[1] = vec2(0, 1);
	tGround[2] = vec2(1, 1);
	tGround[3] = vec2(0, 0);
	tGround[4] = vec2(1, 1);
	tGround[5] = vec2(1, 0);

	//使用程序，将程序变成激活状态
	GLuint program = initShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	//人物
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point4)+sizeof(vec3)+sizeof(vec2))*num, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)*num, points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)*num, sizeof(vec3)*num, normals);
	glBufferSubData(GL_ARRAY_BUFFER, (sizeof(point4)+sizeof(vec3))*num, sizeof(vec2)*num, texCoords);
	//初始化顶点shader中顶点属性
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(point4)*num));

	GLuint vTexCoords= glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoords);
	glVertexAttribPointer(vTexCoords, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)((sizeof(point4) + sizeof(vec3))*num));

	//地面
	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point4) + sizeof(vec3) + sizeof(vec2))*6, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)*6, vGround);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)*6, sizeof(vec3)*6, nGround);
	glBufferSubData(GL_ARRAY_BUFFER, (sizeof(point4) + sizeof(vec3))*6, sizeof(vec2)*6, tGround);
	//初始化顶点shader中顶点属性
     vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(point4)*6));

	vTexCoords = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoords);
	glVertexAttribPointer(vTexCoords, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)((sizeof(point4) + sizeof(vec3))*6));

	














	//初始化关照参数
	point4 light_position(0, -0.2, 2, 0);
	color4 light_diffuse(1, 1, 1, 1);
	color4 light_specular(1, 1, 1, 1);
	//环境光
	color4 light_ambient(0.2, 0.2, 0.2, 1);
	//材质
	color4 material_ambient(1, 1, 1, 1); 
	color4 material_diffuse(0.8, 0.8, 0.8, 1);
	color4 material_specular(1, 1, 1, 1);
	float material_shiness = 100;
	
	color4 diffuse_product = light_diffuse*material_diffuse;
	color4 specular_product = light_specular*material_specular;
	color4 ambient_product = light_ambient*material_ambient;
	//初始化uniform变量
	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, specular_product);
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position);
	glUniform1f(glGetUniformLocation(program, "Shininess"), material_shiness);
	glUniform1i(glGetUniformLocation(program, "texture"), 0);

	model_view = glGetUniformLocation(program, "model_view");
	projection = glGetUniformLocation(program, "projection");

	glEnable(GL_DEPTH_TEST);
	
	glClearColor(1, 1, 1, 1.0);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	point4 eye(3*sin(theta), -0.5,3*cos(theta) ,1);
	point4 at(0.0, 0.0, 0.0, 1.0);
	vec4 up(0.0, 1.0, 0.0, 0.0);
	//模视变换矩阵
	mat4 mv = LookAt(eye, at, up);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
	//投影矩阵
	mat4 p = Perspective(40, 1, 0.1, 100);
	glUniformMatrix4fv(projection, 1, GL_TRUE, p);

	texture[0] = SOIL_load_OGL_texture("luffy.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindVertexArray(VAO[0]);
	glDrawArrays(GL_TRIANGLES, 0, num);

	//texture[1] = SOIL_load_OGL_texture("ground.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,1);
	//glBindTexture(GL_TEXTURE_2D, texture[1]);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glBindVertexArray(VAO[1]);
	//glDrawArrays(GL_TRIANGLES, 0, 6);

	glutSwapBuffers();
}

void idle()
{
	theta += 0.0005;
	if (theta > 360.0)
	{
		theta -= 360.0;
	}
	glutPostRedisplay();
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutCreateWindow("OpenGL");

	glewInit();
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}