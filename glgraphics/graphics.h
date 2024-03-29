#ifndef GL_GRAPHICS_PREDEF
#define GL_GRAPHICS_PREDEF

#include "classes.h"
#include "scrolls/graphics.h"
#include "scrolls/libraries.h"
#include "scrolls/debug.h"

#include "rendervecs.h"

extern GLFWwindow* window;

class GLGraphicsContext : public GraphicsContext { public:
	PLUGIN_HEAD(GLGraphicsContext);
	// PluginUpCast<DebugLines, GLDebugLines> debuglines {&::debuglines};
	PLUGIN_REQUIRES_RUNTIME(::debuglines, debuglines, GLDebugLines);
	PLUGIN_REQUIRES(GraphicsContext::, blockvecs, AsyncGLVecs);
	PLUGIN_REQUIRES(GraphicsContext::, transvecs, AsyncGLVecs);
	PLUGIN_REQUIRES(GraphicsContext::, uivecs, GLUIVecs);
	
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
	
	vec3* camera_pos = nullptr;
	vec2* camera_rot = nullptr;
	
	GLGraphicsContext();
	~GLGraphicsContext();
	
	virtual void set_camera(vec3* pos, vec2* rot);
	
	virtual const PathLib* block_textures() const;
	virtual const PathLib* trans_block_textures() const;
	virtual const PathLib* ui_textures() const;
	
	void init_graphics();
	void load_textures();
	
	virtual void block_draw_call();
	virtual void ui_draw_call();
	virtual void swap();
};

#endif
