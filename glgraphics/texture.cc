#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"

#include "scrolls/libraries.h"
#include "scrolls/ui.h"

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

GLuint loadBMP_image_folder(const PathLib* img_paths, bool transparency) {
	if (img_paths->size() < 0) {
		exit(1);
	}
	int width, height, nrChannels;
	unsigned char* img_data = stbi_load(string(img_paths->getpath(0)).c_str(), &width, &height, &nrChannels, 0);
	unsigned char all_data[width*height*nrChannels*img_paths->size()];
	for (int i = 0; i < width*height*nrChannels; i ++) {
		all_data[i] = img_data[i];
	}
	stbi_image_free(img_data);
	
	for (int i = 1; i < img_paths->size(); i ++) {
		int newwidth, newheight, newnrChannels;
		img_data = stbi_load(string(img_paths->getpath(i)).c_str(), &newwidth, &newheight, &newnrChannels, 0);
		if (newwidth != width or newheight != newheight or newnrChannels != nrChannels) {
			cout << "error in load_image_folder, image sizes are not the same" << endl;
			exit(2);
		}
		for (int j = 0; j < width*height*nrChannels; j ++) {
			all_data[i*width*height*nrChannels + j] = img_data[j];
		}
		stbi_image_free(img_data);
	}
	
	int num_layers = img_paths->size();
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

GLuint loadBMP_array_folder(const PathLib* img_paths, bool transparency, GLint colorformat) {
	
	if (img_paths->size() < 0) {
		exit(1);
	}
	int width, height, nrChannels;
	unsigned char* img_data = stbi_load(string(img_paths->getpath(0)).c_str(), &width, &height, &nrChannels, transparency ? 4 : 3);
	nrChannels = transparency ? 4 : 3;
	unsigned char* all_data = new unsigned char[width*height*nrChannels*img_paths->size()];
	for (int i = 0; i < width*height*nrChannels; i ++) {
		all_data[i] = img_data[i];
	}
	stbi_image_free(img_data);
	
	for (int i = 1; i < img_paths->size(); i ++) {
		int newwidth, newheight, newnrChannels;
		img_data = stbi_load(string(img_paths->getpath(i)).c_str(), &newwidth, &newheight, &newnrChannels, transparency ? 4 : 3);
		if (newwidth != width or newheight != newheight) {
			cout << "error in load_array_folder, image sizes are not the same" << endl;
			exit(2);
		}
		for (int j = 0; j < width*height*nrChannels; j ++) {
			all_data[i*width*height*nrChannels + j] = img_data[j];
		}
		stbi_image_free(img_data);
	}
	
	int num_layers = img_paths->size();
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

	// Give the image to OpenGL
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	
	if (!transparency) {
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,GL_RGB, width, height, num_layers, 0, GL_BGR, GL_UNSIGNED_BYTE, all_data);
	} else {
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,GL_RGBA, width, height, num_layers, 0, GL_BGRA, GL_UNSIGNED_BYTE, all_data);
	}
	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	
	delete[] all_data;
	
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

GLuint loadBMP_pack_folder(const PathLib* img_paths, UIRect* ui_atlas, int max_width, int max_height) {
	stbrp_context context;
	stbrp_node nodes[max_width];
	
	int num_imgs = img_paths->size();
	
	stbrp_rect rects[num_imgs];
	unsigned char* datas[num_imgs];
	
	stbrp_init_target(&context, max_width, max_height, nodes, max_width);
	
	int i = 0;
	for (string path : *img_paths) {
		int width, height, channels;
	  datas[i] = stbi_load(path.c_str(), &width, &height, &channels, 0);
		rects[i].w = width;
		rects[i].h = height;
		i ++;
	}
	
	stbrp_pack_rects(&context, rects, num_imgs);
	
	unsigned char* alldata = new unsigned char[max_width * max_height * 4];
	
	for (i = 0; i < max_width * max_height * 4; i ++) {
		alldata[i] = 255;
	}
	
	for (int i = 0; i < num_imgs; i ++) {
		int xoff = rects[i].x;
		int yoff = rects[i].y;
		cout << xoff << ' ' << yoff << endl;
		ui_atlas[i] = UIRect(0, vec2(0,0), vec2(0,0), vec2(yoff,xoff) / vec2(max_width,max_height),
				vec2(rects[i].h,rects[i].w) / vec2(max_width,max_height));
		for (int x = 0; x < rects[i].w; x ++) {
			for (int y = 0; y < rects[i].h; y ++) {
				for (int z = 0; z < 4; z ++) {
					alldata[(x+xoff) * (max_height*4) + (y+yoff) * 4 + z] = datas[i][y * (rects[i].w*4) + x * 4 + z];
				}
			}
		}
		stbi_image_free(datas[i]);
	}
	
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, max_width, max_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, alldata);
	
	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);
	
	// exit(1);
	
	// Return the ID of the texture we just created
	return textureID;
}
