#ifndef GL_GRAPHICS_PREDEF
#define GL_GRAPHICS_PREDEF

#include "classes.h"
#include "base/graphics.h"
#include "base/libraries.h"

#include "rendervecs.h"

extern GLFWwindow* window;

class GLGraphicsContext : public GraphicsContext { public:
	
	GLuint block_vertexid;
	GLuint ui_vertexid;
	GLuint block_program;
	GLuint ui_program;
	
	GLuint blocktex_id;
	GLuint transblocktex_id;
	GLuint breaking_textures;
	GLuint overlay_textures;
	GLuint edges_textures;
	GLuint uitex_id;
	
	GLuint vertexbuffer;
	GLuint databuffer;
	
	AsyncGLVecs glvecs;
	AsyncGLVecs gltransvecs;
	GLUIVecs gluivecs;
	GLVecsDestination glvecsdest;
	
	GLuint uibuffer;
	
	GLuint pMatID;
	GLuint mvMatID;
	GLuint blockTextureID;
	GLuint uiTextureID;
	GLuint viewdistID;
	GLuint clearcolorID;
	GLuint sunlightID;
	GLuint suncolorID;
	GLuint breakingTexID;
	GLuint overlayTexID;
	GLuint edgesTexID;
	
	PathLib blocktex;
	PathLib transblocktex;
	PathLib uitex;
	
	vector<UIRect> ui_atlas;
	
	vec3* camera_pos;
	vec2* camera_rot;
	
	GLGraphicsContext();
	~GLGraphicsContext();
	
	virtual void set_camera(vec3* pos, vec2* rot);
	
	virtual const PathLib* block_textures() const;
	virtual const PathLib* trans_block_textures() const;
	virtual const PathLib* ui_textures() const;
	
	virtual RenderVecs* blockvecs();
	virtual RenderVecs* transvecs();
	virtual UIVecs* uivecs();
	
	void init_graphics();
	void load_textures();
	
	virtual void block_draw_call();
	virtual void ui_draw_call();
	virtual void swap();
};

#endif
