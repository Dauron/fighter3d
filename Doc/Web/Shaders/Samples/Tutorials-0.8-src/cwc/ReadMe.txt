 *************************************
 aGLSL.cpp, aGLSL.h and helper files
 (c) 2003 by Martin Christen
 
 This directory contains common source
 *************************************

  1) OpenGL Extensions Management:

        aGL_Extensions.cpp
        aGL_Extensions.h

        aGLExt_module_name.cpp
        aGLExt_module_name.h

	At this point, only a few extensions are included: 
	
          *  1.3 - GL_ARB_multitexture		
	  *  1.5 - GL_ARB_vertex_buffer_object
	    
	    OpenGL Shading Language:
	  *  ... - GL_ARB_shader_object
	  *  ... - GL_ARB_vertex_shader
	  *  ... - GL_ARB_fragment_shader

      


     Core Features OpenGL 1.3:
     -------------------------

     *Multitexture			GL_ARB_multitexture
     Transpose Matrix			GL_ARB_transpose_matrix
     Multisample			GL_ARB_multisample
     Texture Add Environment Mode	GL_ARB_texture_env
     Cube Map Textures			GL_ARB_texture_cube_map
     Compressed Textures		GL_ARB_texture_compress
     Texture Border Clamp		GL_ARB_texture_border_clamp
     Texture Combine Environment Mode	GL_ARB_texture_env_combine
     Texture Dot3 Environment Mode	GL_ARB_texture_env_dot3

     Core Features OpenGL 1.4
     ------------------------

     Point Parameters			GL_ARB_point_parameters
     Texture Crossbar Environment Mode	GL_ARB_env_crossbar
     Texture Mirrored Repeat            GL_ARB_texture_mirrored_repeat
     Depth Texture			GL_ARB_depth_texture
     Shadow				GL_ARB_shadow
     Window Raster Position		GL_ARB_window_pos

     Core Features OpenGL 1.5
     ------------------------

     *Buffer Objects			GL_ARB_vertex_buffer_object
     Occlusion Queries			GL_ARB_occlusion_query


     other Extensions:
     -----------------

     Vertex Blend			GL_ARB_vertex_blend
     Matrix Palette			GL_ARB_matrix_palette
     Shadow Ambient			GL_ARB_shadow_ambient	
     Low-level Vertex Programming	GL_ARB_vertex_program
     Low-level Fragment Programming	GL_ARB_fragment_program
     *Shader Objects			GL_ARB_shader_objects
     *High Level Vertex Programming	GL_ARB_vertex_shader
     *High Level Fragment Programming	GL_ARB_fragment_shader
     *OpenGL Shading Language		GL_ARB_shading_language_100
     Non-Power-Of_Two Textures		GL_ARB_texture_non_power_of_two
     Point Sprites			GL_ARB_point_sprite
     ...


  2) OpenGL Shading Language C++ Framework

        aGLSL.h     -> includes aGL_Extensions.h -> includes {aGLExt_module_name.h}
	aGLSL.cpp   -> includes aGL_Extensions.cpp -> includes {aGLExt_module_name.cpp}

  

  3) OpenGL SimpleTutorialTookit (STT)
        Source code to simplify developing test applications for Shaders. 
        OpenGL SimpleTutorialToolkit contains common source for quick
	OpenGL development.
        It is not recommended to use this toolkit for commercial applications.
