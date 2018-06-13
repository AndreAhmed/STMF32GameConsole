#ifndef __graphics_H__
#define __graphics_H__
 
#include <stdint.h>

#define MAX_POINTS_PER_POLY     4   // a maximum of four points per poly
#define MAX_POLYS_PER_OBJECT    16  // this will have to do!
#define WINDOW_WIDTH 240
#define WINDOW_HEIGHT 319
#define M_PI 3.145
#define HALF_SCREEN_WIDTH  160
#define HALF_SCREEN_HEIGHT  120
#define ASPECT_RATIO  (float)0.80
#define viewing_distance 250
#define MAX_VERTICES_PER_OBJECT 32  // this should be enough
#define MAX_OBJECTS             32  // maximum number of objects in world
 

typedef float matrix_4x4[4][4];       // the standard 4x4 homogenous matrix

typedef float matrix_1x4[4];
		
// a 2D vertex
typedef struct VERTEX2DI_TYP
{
	int x,y; // the vertex
} VERTEX2DI, *VERTEX2DI_PTR;

// a 2D vertex
typedef struct VERTEX2DF_TYP
{
	float x,y; // the vertex
} VERTEX2DF, *VERTEX2DF_PTR;



// a 2D polygon
typedef struct POLYGON2D_TYP
{
	unsigned char state;        // state of polygon
	unsigned char num_verts;    // number of vertices
	int x0,y0;        // position of center of polygon
	int xv,yv;        // initial velocity
	int color;      // could be index or PALETTENTRY
	VERTEX2DF vlist[8]; // pointer to vertex list
} POLYGON2D, *POLYGON2D_PTR;
 
// a 3D vertex
typedef struct VERTEX4DI_TYP
{
	int x, y; // the vertex
} VERTEX4DI, *VERTEX4DI_PTR;

// a 3D vertex
typedef struct VERTEX3DF_TYP
{
	float x, y, z; // the vertex
} VERTEX3DF, *VERTEX3DF_PTR;

typedef struct vector_3d_typ
{

	float x, y, z;    // a 3-D vector along with normalization factor
	// if needed

} point_3d, vector_3d, *point_3d_ptr, *vector_3d_ptr;

typedef struct polygon_typ
{
	int color;
	int num_points;   // number of points in polygon (usually 3 or 4)

	int vertex_list[MAX_POINTS_PER_POLY];  // the index number of vertices
	
	float shade;       // the final shade of color after lighting

	int shading;     // type of lighting, flat or constant shading

	int two_sided;   // flags if the polygon is two sided

	int visible;     // used to remove backfaces

	int active;      // used to turn faces on and off

	int clipped;     // flags that polygon has been clipped or removed

	float normal_length; // pre-computed magnitude of normal


} polygon, *polygon_ptr;

 
typedef struct POLYGON3D_TYP
{
	unsigned char state;        // state of polygon
	unsigned char num_verts;    // number of vertices
	int x0, y0;        // position of center of polygon
	int world_x, world_y, world_z;
	int  color;      // could be index or PALETTENTRY
	int num_polys;      // the number of polygons in the object
	polygon polys[MAX_POLYS_PER_OBJECT]; // the polygons that make up the object
	VERTEX3DF vlist[8]; // pointer to vertex list
	VERTEX3DF world_vlist[8]; // transformed vertices 
	VERTEX3DF camera_vlist[8];

} POLYGON3D, *POLYGON3D_PTR;

typedef struct object_typ
{
	int  color;
	int id;             // identification number of object

	int num_vertices;   // total number of vertices in object

	point_3d vertices_local[MAX_VERTICES_PER_OBJECT];  // local vertices

	point_3d vertices_world[MAX_VERTICES_PER_OBJECT];  // world vertices

	point_3d vertices_camera[MAX_VERTICES_PER_OBJECT]; // camera vertices

	int num_polys;      // the number of polygons in the object

	polygon polys[MAX_POLYS_PER_OBJECT]; // the polygons that make up the object

	float radius;       // the average radius of object

	int state;          // state of object

	point_3d world_pos; // position of object in world coordinates

} object, *object_ptr;

#define MAX_FRAMES 64

typedef struct 
{
	const uint16_t *data ;
	int w , h ;
}BITMAP , *BITMAP_PTR ;

typedef struct sprite
{
	float x, y , old_x , old_y ;
	float vxdelay , vydelay , xdelay , ydelay ;
	float xspeed , yspeed , yaccel ;
	int width, height;
	int curr_frame ;
	int num_frames;
	int anim_clock;
	int anim_speed;
	int move_angle , angle_vel ;
  int animdir , continue_anim ;
	const uint16_t * imgData ;
	uint16_t* frames[MAX_FRAMES];
	int alive ; 
}sprite, *sprite_ptr;




void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int color);
void DrawBottomTriangle(int x0, int y0, int x1, int y1, int x2, int y3, int color);
void DrawTopTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color);
void Draw_Object_Solid(object_ptr the_object, int color);
void DrawSolidTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color );
void Draw_Line(float x1, float y1, float x2, float y2, int color);
void Transform_LocalToWorld(object_ptr poly);
void Transform_WorldToCamera( object_ptr poly);
int Rotate_Polygon3D_YAxis(object_ptr poly, int theta);
void Remove_Backfaces_And_Shade(object_ptr the_object, int color);
void Make_Vector_3D(point_3d_ptr init,
			point_3d_ptr term,
			vector_3d_ptr result);

void Cross_Product_3D(vector_3d_ptr u,
			vector_3d_ptr v,
			vector_3d_ptr normal);


float Vector_Mag_3D(vector_3d_ptr v);

void Mat_Print_4x4(matrix_4x4 a);

void Mat_Print_1x4(matrix_1x4 a);

void Mat_Mul_4x4_4x4(matrix_4x4 a,
			matrix_4x4 b,
			matrix_4x4 result);

void Mat_Mul_1x4_4x4(matrix_1x4 a,
			matrix_4x4 b,
			matrix_1x4 result);

void Mat_Identity_4x4(matrix_4x4 a);

void Mat_Zero_4x4(matrix_4x4 a);

void Mat_Copy_4x4(matrix_4x4 source, matrix_4x4 destination);

void  blit(BITMAP_PTR SrcPm,uint8_t *fb,  int src_x, int src_y, int dst_x, int dst_y, int w, int h) ;

void InitSprite(sprite_ptr sprite, int x, int y, int width, int height, int ac, int as, int numframes);

void  Grape_frame( sprite_ptr spr , BITMAP_PTR src_bitmap  ,  int frames_col , int frame_num );
void  Update_postion(sprite_ptr spr) ;
void  Update_animation(sprite_ptr spr) ;
void  CopySDRam(int w , int h , uint32_t buffer_layer ,  uint32_t screen_layer  ) ;
void  DrawBitmap(const uint16_t*bitmap, int w, int h, int x, int y, float angle, uint32_t layer);
int   collision( sprite_ptr spr1 ,  sprite_ptr spr2) ;
void  DrawSprite(sprite_ptr sprite, uint8_t *fb ) ;
void  erase_spr(sprite_ptr spr , BITMAP_PTR src_bitmap) ;
void  BufferToScreen(uint16_t *buffer , int w , int h);

int fsin( short alpha);
int fcos( short alpha);

#endif
