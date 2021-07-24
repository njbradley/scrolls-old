#ifndef TEXTURE
#define TEXTURE

#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <map>


#include "cross-platform.h"

GLuint loadBMP_custom(const char * imagepath, bool transparency){
	
	unsigned char * data;
	
	int width, height, nrChannels;
    data = stbi_load(imagepath, &width, &height, &nrChannels, (transparency) ? 4 : 3);
    //SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	if (transparency) {
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	}
	stbi_image_free(data);
	// OpenGL has now copied the data. Free our own version
	//delete [] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}

GLuint loadBMP_image_folder(string dirpath, bool transparency) {
	vector<string> img_paths;
	get_files_folder(dirpath, &img_paths);
	if (img_paths.size() < 0) {
		exit(1);
	}
	int width, height, nrChannels;
	unsigned char* img_data = stbi_load((dirpath + "/" + img_paths[0]).c_str(), &width, &height, &nrChannels, 0);
	unsigned char all_data[width*height*nrChannels*img_paths.size()];
	for (int i = 0; i < width*height*nrChannels; i ++) {
		all_data[i] = img_data[i];
	}
	stbi_image_free(img_data);
	
	for (int i = 1; i < img_paths.size(); i ++) {
		int newwidth, newheight, newnrChannels;
		img_data = stbi_load((dirpath + "/" + img_paths[i]).c_str(), &newwidth, &newheight, &newnrChannels, 0);
		if (newwidth != width or newheight != newheight or newnrChannels != nrChannels) {
			cout << "error in load_image_folder, image sizes are not the same" << endl;
			exit(2);
		}
		for (int j = 0; j < width*height*nrChannels; j ++) {
			all_data[i*width*height*nrChannels + j] = img_data[j];
		}
		stbi_image_free(img_data);
	}
	
	int num_layers = img_paths.size();
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	if (transparency) {
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height*num_layers, 0, GL_BGRA, GL_UNSIGNED_BYTE, all_data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height*num_layers, 0, GL_BGR, GL_UNSIGNED_BYTE, all_data);
	}
	// OpenGL has now copied the data. Free our own version
	//delete [] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}

GLuint loadBMP_array_folder(string dirpath, bool transparency = false, GLint colorformat = 0) {
	vector<string> img_paths;
	get_files_folder(dirpath, &img_paths);
	if (img_paths.size() < 0) {
		exit(1);
	}
	int width, height, nrChannels;
	unsigned char* img_data = stbi_load((dirpath + "/" + img_paths[0]).c_str(), &width, &height, &nrChannels, transparency ? 4 : 3);
	nrChannels = transparency ? 4 : 3;
	unsigned char* all_data = new unsigned char[width*height*nrChannels*img_paths.size()];
	for (int i = 0; i < width*height*nrChannels; i ++) {
		all_data[i] = img_data[i];
	}
	stbi_image_free(img_data);
	
	for (int i = 1; i < img_paths.size(); i ++) {
		int newwidth, newheight, newnrChannels;
		img_data = stbi_load((dirpath + "/" + img_paths[i]).c_str(), &newwidth, &newheight, &newnrChannels, transparency ? 4 : 3);
		if (newwidth != width or newheight != newheight) {
			cout << "error in load_array_folder, image sizes are not the same" << endl;
			exit(2);
		}
		for (int j = 0; j < width*height*nrChannels; j ++) {
			all_data[i*width*height*nrChannels + j] = img_data[j];
		}
		stbi_image_free(img_data);
	}
	
	int num_layers = img_paths.size();
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

	// Give the image to OpenGL
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	// for (int i = 0; i < width*height*3*num_layers; i ++) {
	// 	cout << int(all_data[i]) << ' ';
	// }
	// cout << endl;
	// cout << width << ' ' << height << ' ' << num_layers << endl;
	if (!transparency) {
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,GL_RGB, width, height, num_layers, 0, GL_BGR, GL_UNSIGNED_BYTE, all_data);
	} else {
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,GL_RGBA, width, height, num_layers, 0, GL_BGRA, GL_UNSIGNED_BYTE, all_data);
	}
	// OpenGL has now copied the data. Free our own version
	//delete [] data;
	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	// ... which requires mipmaps. Generate them automatically.
	
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	
	delete[] all_data;
	// Return the ID of the texture we just created
	return textureID;
}




GLuint loadBMP_edge_combos(string dirpath, int* num_edges) {
	vector<string> img_paths;
	get_files_folder(dirpath, &img_paths);
	if (img_paths.size() < 0) {
		cout << "no files in folder " << endl;
		std::terminate();
	}
	
	
	
	int width, height, nrChannels;
	unsigned char* img_data = stbi_load((dirpath + "/" + img_paths[0]).c_str(), &width, &height, &nrChannels, 0);
	unsigned char* all_data = new unsigned char[width*height*nrChannels*img_paths.size()];
	for (int i = 0; i < width*height*nrChannels; i ++) {
		all_data[i] = img_data[i];
	}
	stbi_image_free(img_data);
	
	for (int i = 1; i < img_paths.size(); i ++) {
		int newwidth, newheight, newnrChannels;
		img_data = stbi_load((dirpath + "/" + img_paths[i]).c_str(), &newwidth, &newheight, &newnrChannels, 0);
		if (newwidth != width or newheight != newheight) {
			cout << "error in load_array_folder, image sizes are not the same" << endl;
			exit(2);
		}
		for (int j = 0; j < width*height*nrChannels; j ++) {
			all_data[i*width*height*nrChannels + j] = img_data[j];
		}
		stbi_image_free(img_data);
	}
	
	int num_layers = img_paths.size();
	
	int img_total_size = width*height*nrChannels*img_paths.size();
	
	unsigned char* rotated = new unsigned char[img_total_size*4];
	for (int i = 0; i < img_total_size; i++) {
		rotated[i] = all_data[i];
	}
	
	for (int i = 1; i < 4; i ++) {
		for (int pic = 0; pic < num_layers; pic ++) {
			for (int x = 0; x < width; x ++) {
				for (int y = 0; y < height; y ++) {
					int orig_pos = pic * width * height * nrChannels + x * height * nrChannels + y * nrChannels;
					int rot_pos = pic * width * height * nrChannels + (height - y - 1) * height * nrChannels + x * nrChannels;
					
					for (int j = 0; j < nrChannels; j ++) {
						rotated[i*img_total_size + rot_pos + j] = rotated[(i-1)*img_total_size + orig_pos + j];
					}
				}
			}
		}
	}
	
	unsigned char* final_data = new unsigned char[(num_layers + 1) * (num_layers + 1) * (num_layers + 1) * (num_layers + 1) * width*height*nrChannels];
	int img_bytes = width*height*nrChannels;
	int off = 0;
	for (int i = 0; i <= num_layers; i ++) {
		for (int j = 0; j <= num_layers; j ++) {
			for (int k = 0; k <= num_layers; k ++) {
				for (int l = 0; l <= num_layers; l ++) {
					int arr[] = {l,j,k,i};
					int index = 0;
					for (int val : arr) {
						if (val > 0) {
							for (int x = 0; x < width; x ++) {
								for (int y = 0; y < height; y ++) {
									for (int c = 0; c < nrChannels; c ++) {
										unsigned char currentdata = final_data[off*img_bytes + x*height*nrChannels + y*nrChannels + c];
										unsigned char indata = rotated[index*img_total_size + (val-1)*img_bytes + x*height*nrChannels + y*nrChannels + c];
										currentdata = indata;
										if (currentdata > 255) {
											currentdata = 255;
										}
										final_data[off*img_bytes + x*height*nrChannels + y*nrChannels + c] = currentdata;
									}
								}
							}
						}
						index ++;
					}
					off ++;
				}
			}
		}
	}
	
	
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

	// Give the image to OpenGL
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	// for (int i = 0; i < width*height*3*num_layers; i ++) {
	// 	cout << int(all_data[i]) << ' ';
	// }
	// cout << endl;
	// cout << width << ' ' << height << ' ' << num_layers << endl;
	
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,GL_RGBA, width, height, (num_layers + 1) * (num_layers + 1) * (num_layers + 1) * (num_layers + 1), 0, GL_BGRA, GL_UNSIGNED_BYTE, final_data);
	
	// OpenGL has now copied the data. Free our own version
	//delete [] data;
	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	// ... which requires mipmaps. Generate them automatically.
	
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	
	delete[] all_data;
	delete[] rotated;
	delete[] final_data;
	// Return the ID of the texture we just created
	return textureID;
}



GLuint loadBMP_array_custom(const char * imagepath){
	
	unsigned char * data;
	
	int width, height, nrChannels;
  data = stbi_load(imagepath, &width, &height, &nrChannels, 3);
  //SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  
	int num_layers = height / width;
	
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

	// Give the image to OpenGL
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,GL_RGB, width, width, num_layers, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	
	stbi_image_free(data);
	// OpenGL has now copied the data. Free our own version
	//delete [] data;
	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	// ... which requires mipmaps. Generate them automatically.
	
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	
	// Return the ID of the texture we just created
	return textureID;
}

#endif
