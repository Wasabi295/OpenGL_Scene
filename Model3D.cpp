#include "Model3D.hpp"

#include <unordered_map>

namespace gps {

	void Model3D::LoadModel(std::string fileName) {
		std::string basePath = fileName.substr(0, fileName.find_last_of('/')) + "/";
		ReadOBJ(fileName, basePath);
	}

	void Model3D::LoadModel(std::string fileName, std::string basePath) {
		ReadOBJ(fileName, basePath);
	}

	void Model3D::Draw(gps::Shader shaderProgram) {
		for (int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shaderProgram);
	}

	void Model3D::ReadOBJ(std::string fileName, std::string basePath) {

		std::cout << "Loading : " << fileName << std::endl;
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		int materialId;

		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), basePath.c_str(), GL_TRUE);

		if (!err.empty()) {
			std::cerr << err << std::endl;
		}

		if (!ret) {
			exit(1);
		}

		std::cout << "# of shapes    : " << shapes.size() << std::endl;
		std::cout << "# of materials : " << materials.size() << std::endl;

		
		for (size_t s = 0; s < shapes.size(); s++) {

			struct TempMesh {
				std::vector<gps::Vertex> vertices;
				std::vector<GLuint> indices;
				std::vector<gps::Texture> textures;
				gps::Material material;
			};

			auto buildMaterialAndTextures = [&](int matId, gps::Material& outMat, std::vector<gps::Texture>& outTex) {
				
				outMat.ambient = glm::vec3(0.3f);
				outMat.diffuse = glm::vec3(0.8f);
				outMat.specular = glm::vec3(0.5f);
				outMat.shininess = 32.0f;
				outTex.clear();

				if (matId < 0 || matId >= (int)materials.size()) return;
				const auto& m = materials[matId];

				outMat.ambient = glm::vec3(m.ambient[0], m.ambient[1], m.ambient[2]);
				outMat.diffuse = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
				outMat.specular = glm::vec3(m.specular[0], m.specular[1], m.specular[2]);
				outMat.shininess = m.shininess;

				
				if (!m.ambient_texname.empty()) {
					outTex.push_back(LoadTexture(basePath + m.ambient_texname, "ambientTexture"));
				}
				if (!m.diffuse_texname.empty()) {
					outTex.push_back(LoadTexture(basePath + m.diffuse_texname, "diffuseTexture"));
				}
				if (!m.specular_texname.empty()) {
					outTex.push_back(LoadTexture(basePath + m.specular_texname, "specularTexture"));
				}
				};

			
			std::unordered_map<int, TempMesh> perMaterial;

			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				int fv = shapes[s].mesh.num_face_vertices[f];
				int matId = -1;
				if (f < shapes[s].mesh.material_ids.size()) matId = shapes[s].mesh.material_ids[f];

				
				auto it = perMaterial.find(matId);
				if (it == perMaterial.end()) {
					TempMesh t;
					buildMaterialAndTextures(matId, t.material, t.textures);
					it = perMaterial.emplace(matId, std::move(t)).first;
				}
				TempMesh& dst = it->second;

				for (size_t v = 0; v < (size_t)fv; v++) {
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					float vx = attrib.vertices[3 * idx.vertex_index + 0];
					float vy = attrib.vertices[3 * idx.vertex_index + 1];
					float vz = attrib.vertices[3 * idx.vertex_index + 2];

					float nx = 0.0f, ny = 1.0f, nz = 0.0f;
					if (idx.normal_index >= 0) {
						nx = attrib.normals[3 * idx.normal_index + 0];
						ny = attrib.normals[3 * idx.normal_index + 1];
						nz = attrib.normals[3 * idx.normal_index + 2];
					}

					float tx = 0.0f, ty = 0.0f;
					if (idx.texcoord_index >= 0) {
						tx = attrib.texcoords[2 * idx.texcoord_index + 0];
						ty = attrib.texcoords[2 * idx.texcoord_index + 1];
					}

					gps::Vertex currentVertex;
					currentVertex.Position = glm::vec3(vx, vy, vz);
					currentVertex.Normal = glm::vec3(nx, ny, nz);
					currentVertex.TexCoords = glm::vec2(tx, ty);

					dst.vertices.push_back(currentVertex);
					dst.indices.push_back((GLuint)dst.vertices.size() - 1);
				}

				index_offset += fv;
			}

			
			for (auto& kv : perMaterial) {
				TempMesh& t = kv.second;
				if (t.vertices.empty() || t.indices.empty()) continue;
				gps::Mesh mesh(t.vertices, t.indices, t.textures);
				mesh.material = t.material;
				meshes.push_back(mesh);
			}
		}
	}

	gps::Texture Model3D::LoadTexture(std::string path, std::string type) {

		for (int i = 0; i < loadedTextures.size(); i++) {
			if (loadedTextures[i].path == path) {
				return loadedTextures[i];
			}
		}

		gps::Texture currentTexture;
		currentTexture.id = ReadTextureFromFile(path.c_str());
		currentTexture.type = std::string(type);
		currentTexture.path = path;

		loadedTextures.push_back(currentTexture);

		return currentTexture;
	}

	GLuint Model3D::ReadTextureFromFile(const char* file_name) {

		int x, y, n;
		int force_channels = 4;
		unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);

		if (!image_data) {
			fprintf(stderr, "ERROR: could not load %s\n", file_name);

			
			GLuint textureID;
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);
			unsigned char white[] = { 255, 255, 255, 255 };
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			return textureID;
		}

		
		if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
			fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name);
		}

		int width_in_bytes = x * 4;
		unsigned char* top = NULL;
		unsigned char* bottom = NULL;
		unsigned char temp = 0;
		int half_height = y / 2;

		for (int row = 0; row < half_height; row++) {
			top = image_data + row * width_in_bytes;
			bottom = image_data + (y - row - 1) * width_in_bytes;

			for (int col = 0; col < width_in_bytes; col++) {
				temp = *top;
				*top = *bottom;
				*bottom = temp;
				top++;
				bottom++;
			}
		}

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_SRGB_ALPHA,
			x,
			y,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			image_data
		);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(image_data);

		return textureID;
	}

	Model3D::~Model3D() {

		for (size_t i = 0; i < loadedTextures.size(); i++) {
			glDeleteTextures(1, &loadedTextures.at(i).id);
		}

		for (size_t i = 0; i < meshes.size(); i++) {
			GLuint VBO = meshes.at(i).getBuffers().VBO;
			GLuint EBO = meshes.at(i).getBuffers().EBO;
			GLuint VAO = meshes.at(i).getBuffers().VAO;
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
			glDeleteVertexArrays(1, &VAO);
		}
	}
}