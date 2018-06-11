/*
Basic 2D / 3D Graphics Engine (Software Renderer)
Ahmed Saleh Tolba
Last update: 08.09.2014
xgameprogrammer@hotmail.com 
*/
#include <stdlib.h>
#include <math.h>

#include "graphics.h"


#include "stm32f429i_discovery_lcd.h"

#define SCREEN_WIDTH     240
#define SCREEN_HEIGHT    320

float ambient_light = 0.5f;          // ambient light level
vector_3d light_source = { -0.913913f,0.389759f,-0.113369f }; // position of point light source
point_3d view_point = { 0,0,0};  // position of camera

// precomputed sin table * 256
static const short tsin[91]= { 0,4,9,13,18,22,27,31,36,40,44,49,53,58,62,66,71,75,
                        79,83,88,92,96,100,104,108,112,116,120,124,128,132,
                        136,139,143,147,150,154,158,161,165,168,171,175,178,
                        181,184,187,190,193,196,199,202,204,207,210,212,215,
                        217,219,222,224,226,228,230,232,234,236,237,239,241,
                        242,243,245,246,247,248,249,250,251,252,253,254,254,
                        255,255,255,256,256,256,256
                        };


uint16_t floatToRGB565(float input)
{
    uint8_t x = input*255;
    uint8_t r = (x&248u);//0xff-0x07, 5msb mask
    uint8_t g = (x&252u);//0xff-0x03, 6msb mask
    uint8_t b = (x&248u);//0xff-0x07, 6msb mask

    return (r<<8)|(g<<3)|(b>>3);// assuming r is at msb
}										
												
										
/**
 * @brief  Displays a line.
 * @param  Xpos: specifies the X position.
 * @param  Ypos: specifies the Y position.
 * @param  Length: line length.
 * @param  Direction: line direction.
 *   This parameter can be one of the following values: LCD_LINE_VERTICAL or Horizontal.
 * @retval None
 */
void Draw_Line(float x1, float y1, float x2, float y2, int color) {
	float slope;
	float y;
	float x;
	float xdiff = (x2 - x1);
	float ydiff = (y2 - y1);

	if (xdiff == 0.0f && ydiff == 0.0f) {
		BSP_LCD_DrawPixel((int) x1, (int) y1, color);
		return;
	}

	if (fabs(xdiff) > fabs(ydiff)) {
		float xmin, xmax;

		// set xmin to the lower x value given
		// and xmax to the higher value
		if (x1 < x2) {
			xmin = x1;
			xmax = x2;
		} else {
			xmin = x2;
			xmax = x1;
		}

		// draw line in terms of y slope
		slope = ydiff / xdiff;
		
		for (x = xmin; x <= xmax; x += 1.0f) {
			float y = y1 + ((x - x1) * slope);
			BSP_LCD_DrawPixel((int) x, (int) y, color);
		}
	} else {
		float ymin, ymax;

		// set ymin to the lower y value given
		// and ymax to the higher value
		if (y1 < y2) {
			ymin = y1;
			ymax = y2;
		} else {
			ymin = y2;
			ymax = y1;
		}

		// draw line in terms of x slope
		
	  slope		= xdiff / ydiff;
	
		for ( y = ymin; y <= ymax; y += 1.0f) {
			float x = x1 + ((y - y1) * slope);
			BSP_LCD_DrawPixel((int) x, (int) y, color);
		}
	}
}
 

void DrawBottomTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color)
{
 
  int y;
	float dxy_left = (float)(x2 - x0) / (y2 - y0);
	float dxy_right = (float)(x1 - x0) / (y1 - y0);
 
	// set starting and ending points for edge trace
	float xs = x0;
	float xe = x0;
 
	// draw each scanline
	for (y = y0; y <= y1; y++)
	{
		// draw a line from xs to xe at y in color c
		BSP_LCD_DrawLine((int)xs, (int)y,(int)xe, (int)y);
		// move down one scanline
		xs += dxy_left;
		xe += dxy_right;
	} // end for y
}


void DrawTopTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color)
{
	int temp_x;
	int y;
	// compute deltas
	float dxy_left = (float)(x2 - x0) / (y2 - y0);
	float dxy_right = (float)(x2 - x1) / (y2 - y1);
	// set starting and ending points for edge trace
	float xs = x0;
	float xe = x1;
	// draw each scanline
	for (y = y0; y <= y2; y++)
	{
		// draw a line from xs to xe at y in color c
		BSP_LCD_DrawLine((int)(xs + 0.5), (int)y,(int)(xe + 0.5), y);
		// move down one scanline
		xs += dxy_left;
		xe += dxy_right;
	} // end for y
}
void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
	int temp_x, // used for sorting
	temp_y;
	if ((x1 == x2 && x2 == x3) || (y1 == y2 && y2 == y3))
			return;
	// sort p1,p2,p3 in ascending y order
	if (y2 < y1)
	{
		temp_x = x2;
		temp_y = y2;
		x2 = x1;
		y2 = y1;
		x1 = temp_x;
		y1 = temp_y;
	} // end if
	  // now we know that p1 and p2 are in order
	if (y3 < y1)
	{
		temp_x = x3;
		temp_y = y3;
		x3 = x1;
		y3 = y1;
		x1 = temp_x;
		y1 = temp_y;
	} // end if
	  // finally test y3 against y2
	if (y3 < y2)
	{
		temp_x = x3;
		temp_y = y3;
		x3 = x2;
		y3 = y2;
		x2 = temp_x;
		y2 = temp_y;
	} // end if


	if (y2 == y3)
	{
		DrawBottomTriangle(x1, y1, x2,y2,x3,y3, color);
	}
	/* check for trivial case of top-flat triangle */
	else if (y1 == y2)
	{
		DrawTopTriangle(x1, y1, x2, y2, x3, y3, color);
	}
	else
	{
		/* general case - split the triangle in a topflat and bottom-flat one */
		int vTmpx;
		vTmpx = (int)(x1 + ((float)(y2 - y1) / (float)(y3 - y1)) * (x3 - x1));
		 
		DrawBottomTriangle(x1, y1, vTmpx, y2, x2, y2, color);
		DrawTopTriangle(x2, y2, vTmpx, y2, x3, y3, color);
	}

}
 
 
void Draw_Object_Solid(object_ptr the_object, int color)
{

	// this function draws an object shaded solid and can perform
	// simple z entent clipping

	int curr_poly,      // the current polygon
		vertex_1,       // vertex index numbers
		vertex_2,
		vertex_3,
		vertex_4,
		is_quad = 0;      // quadrilateral flag

	float x1, y1, z1,     // working variables
		x2, y2, z2,
		x3, y3, z3,
		x4, y4, z4;

	// compute position of object in world

	for (curr_poly = 0; curr_poly < the_object->num_polys; curr_poly++)
	{

		if (the_object->polys[curr_poly].visible == 0)
			continue;
		// extract the vertex numbers

		vertex_1 = the_object->polys[curr_poly].vertex_list[0];
		vertex_2 = the_object->polys[curr_poly].vertex_list[1];
		vertex_3 = the_object->polys[curr_poly].vertex_list[2];


		// do Z clipping first before projection

		z1 = the_object->vertices_camera[vertex_1].z;
		z2 = the_object->vertices_camera[vertex_2].z;
		z3 = the_object->vertices_camera[vertex_3].z;

		// test if this is a quad

		if (the_object->polys[curr_poly].num_points == 4)
		{

			// extract vertex number and z component for clipping and projection

			vertex_4 = the_object->polys[curr_poly].vertex_list[3];
			z4 = the_object->vertices_camera[vertex_4].z;

			// set quad flag

			is_quad = 1;

		} // end if quad
		else
			z4 = z3;
 

		// extract points of polygon

		x1 = the_object->vertices_camera[vertex_1].x;
		y1 = the_object->vertices_camera[vertex_1].y;

		x2 = the_object->vertices_camera[vertex_2].x;
		y2 = the_object->vertices_camera[vertex_2].y;

		x3 = the_object->vertices_camera[vertex_3].x;
		y3 = the_object->vertices_camera[vertex_3].y;


		// compute screen position of points

		x1 = (HALF_SCREEN_WIDTH + x1*viewing_distance / z1);
		y1 = (HALF_SCREEN_HEIGHT - ASPECT_RATIO*y1*viewing_distance / z1);

		x2 = (HALF_SCREEN_WIDTH + x2*viewing_distance / z2);
		y2 = (HALF_SCREEN_HEIGHT - ASPECT_RATIO*y2*viewing_distance / z2);

		x3 = (HALF_SCREEN_WIDTH + x3*viewing_distance / z3);
		y3 = (HALF_SCREEN_HEIGHT - ASPECT_RATIO*y3*viewing_distance / z3);

		// draw triangle
  
		DrawTriangle((int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3, the_object->polys[curr_poly].shade );

		// draw second poly if this is a quad

		if (is_quad)
		{
			// extract the point

			x4 = the_object->vertices_camera[vertex_4].x;
			y4 = the_object->vertices_camera[vertex_4].y;

			// poject to screen

			x4 = (HALF_SCREEN_WIDTH + x4*viewing_distance / z4);
			y4 = (HALF_SCREEN_HEIGHT - ASPECT_RATIO*y4*viewing_distance / z4);

			// draw triangle
			 
			DrawTriangle((int)x1, (int)y1, (int)x3, (int)y3, (int)x4, (int)y4, the_object->polys[curr_poly].shade);

		} // end if quad

	} // end for curr_poly

} // end Draw_Object_Solid
void swap(int *i, int *j) {
	int t = *i;
	*i = *j;
	*j = t;
}
 
float Dot_Product_3D(vector_3d_ptr u, vector_3d_ptr v)
{
	// this function computes the dot product of two vectors

	return((u->x * v->x) + (u->y * v->y) + (u->z * v->z));

} // end Dot_Product

  //////////////////////////////////////////////////////////////////////////////

void  Make_Vector_3D(point_3d_ptr init,
	point_3d_ptr term,
	vector_3d_ptr result)
{
	// this function creates a vector from two points in 3D space

	result->x = term->x - init->x;
	result->y = term->y - init->y;
	result->z = term->z - init->z;

} // end Make_Vector

  //////////////////////////////////////////////////////////////////////////////

void  Cross_Product_3D(vector_3d_ptr u,
	vector_3d_ptr v,
	vector_3d_ptr normal)
{
	// this function computes the cross product between two vectors

	normal->x = (u->y*v->z - u->z*v->y);
	normal->y = -(u->x*v->z - u->z*v->x);
	normal->z = (u->x*v->y - u->y*v->x);

} // end Cross_Product_3D

  ///////////////////////////////////////////////////////////////////////////////
float Vector_Mag_3D(vector_3d_ptr v)
{
	// computes the magnitude of a vector

	return((float)sqrt(v->x*v->x + v->y*v->y + v->z*v->z));

} // end Vector_Mag_3D
 
  //////////////////////////////////////////////////////////////////////////////

void  Mat_Mul_4x4_4x4(matrix_4x4 a,
	matrix_4x4 b,
	matrix_4x4 result)
{
	// this function multiplies a 4x4 by a 4x4 and stores the result in a 4x4

	int index_i,  // row and column looping vars
		index_j,
		index_k;

	float sum;    // temp used to hold sum of products

				  // loop thru rows of a

	for (index_i = 0; index_i < 4; index_i++)
	{
		// loop thru columns of b

		for (index_j = 0; index_j < 4; index_j++)
		{

			// multiply ith row of a by jth column of b and store the sum
			// of products in the position i,j of result

			sum = 0;

			for (index_k = 0; index_k < 4; index_k++)
				sum += a[index_i][index_k] * b[index_k][index_j];

			// store result

			result[index_i][index_j] = sum;

		} // end for index_j

	} // end for index_i

} // end Mat_Mul_4x4_4x4

  //////////////////////////////////////////////////////////////////////////////

void Mat_Mul_1x4_4x4(matrix_1x4 a,
	matrix_4x4 b,
	matrix_1x4 result)
{
	// this function multiplies a 1x4 by a 4x4 and stores the result in a 1x4

	int index_j,    // column index
		index_k;    // row index

	float sum;    // temp used to hold sum of products

				  // loop thru columns of b

	for (index_j = 0; index_j < 4; index_j++)
	{

		// multiply ith row of a by jth column of b and store the sum
		// of products in the position i,j of result

		sum = 0;

		for (index_k = 0; index_k < 4; index_k++)
			sum += a[index_k] * b[index_k][index_j];

		// store result

		result[index_j] = sum;

	} // end for index_j

} // end Mat_Mul_1x4_4x4

  //////////////////////////////////////////////////////////////////////////////

void  Mat_Identity_4x4(matrix_4x4 a)
{
	// this function creates a 4x4 identity matrix

	a[0][1] = a[0][2] = a[0][3] = 0;
	a[1][0] = a[1][2] = a[1][3] = 0;
	a[2][0] = a[2][1] = a[2][3] = 0;
	a[3][0] = a[3][1] = a[3][2] = 0;

	// set main diagonal to 1's

	a[0][0] = a[1][1] = a[2][2] = a[3][3] = 1;

} // end Mat_Identity_4x4

  /////////////////////////////////////////////////////////////////////////////

void Mat_Zero_4x4(matrix_4x4 a)
{
	// this function zero's out a 4x4 matrix

	a[0][0] = a[0][1] = a[0][2] = a[0][3] = 0;
	a[1][0] = a[1][1] = a[1][2] = a[1][3] = 0;
	a[2][0] = a[2][1] = a[2][2] = a[2][3] = 0;
	a[3][0] = a[3][1] = a[3][2] = a[3][3] = 0;

} // end Mat_Zero_4x4

  /////////////////////////////////////////////////////////////////////////////

void  Mat_Copy_4x4(matrix_4x4 source, matrix_4x4 destination)
{
	// this function copies one 4x4 matrix to another

	int index_i,
		index_j; // looping vars

				 // copy the matrix row by row

	for (index_i = 0; index_i < 4; index_i++)
		for (index_j = 0; index_j < 4; index_j++)
			destination[index_i][index_j] = source[index_i][index_j];

} // end Mat_Copy_4x4

 
void Transform_LocalToWorld(object_ptr poly)
{
	int curr_vert;
	if (!poly) return;
	

	for (curr_vert = 0; curr_vert < poly->num_vertices; curr_vert++)
	{
		poly->vertices_world[curr_vert].x = poly->vertices_local[curr_vert].x + poly->world_pos.x;
		poly->vertices_world[curr_vert].y = poly->vertices_local[curr_vert].y + poly->world_pos.y;
		poly->vertices_world[curr_vert].z = poly->vertices_local[curr_vert].z + poly->world_pos.z;
	}
}

void Transform_WorldToCamera( object_ptr poly)
{
	int curr_vert;
	// assume camera at 0,0,0 with angles 0,0,0; 
	for (curr_vert = 0; curr_vert < poly->num_vertices; curr_vert++)
	{
		poly->vertices_camera[curr_vert].x = poly->vertices_local[curr_vert].x + poly->world_pos.x;
		poly->vertices_camera[curr_vert].y = poly->vertices_local[curr_vert].y + poly->world_pos.y;
		poly->vertices_camera[curr_vert].z = poly->vertices_local[curr_vert].z + poly->world_pos.z;
	}
}

void Remove_Backfaces_And_Shade(object_ptr the_object, int color)
{
	// this function removes all the backfaces of an object by setting the removed
	// flag. This function assumes that the object has been transformed into
	// camera coordinates. Also, the function computes the flat shading of the
	// object

	int vertex_0,         // vertex indices
		vertex_1,
		vertex_2,
		curr_poly;        // current polygon

	float dp,             // the result of the dot product
	intensity;      // the final intensity of the surface

	vector_3d u, v,           // general working vectors
		normal,        // the normal to the surface begin processed
		sight;         // line of sight vector

					   // for each polygon in the object determine if it is pointing away from the
					   // viewpoint and direction

	for (curr_poly = 0; curr_poly < the_object->num_polys; curr_poly++)
	{

		// compute two vectors on polygon that have the same intial points

		vertex_0 = the_object->polys[curr_poly].vertex_list[0];
		vertex_1 = the_object->polys[curr_poly].vertex_list[1];
		vertex_2 = the_object->polys[curr_poly].vertex_list[2];

		// the vector u = vo->v1

		Make_Vector_3D((point_3d_ptr)&the_object->vertices_world[vertex_0],
			(point_3d_ptr)&the_object->vertices_world[vertex_1],
			(vector_3d_ptr)&u);

		// the vector v = vo-v2

		Make_Vector_3D((point_3d_ptr)&the_object->vertices_world[vertex_0],
			(point_3d_ptr)&the_object->vertices_world[vertex_2],
			(vector_3d_ptr)&v);

		// compute the normal to polygon v x u
		 


		Cross_Product_3D((vector_3d_ptr)&v,
			(vector_3d_ptr)&u,
			(vector_3d_ptr)&normal);

		// compute the line of sight vector, since all coordinates are world all
		// object vertices are already relative to (0,0,0), thus

		sight.x = view_point.x - the_object->vertices_world[vertex_0].x;
		sight.y = view_point.y - the_object->vertices_world[vertex_0].y;
		sight.z = view_point.z - the_object->vertices_world[vertex_0].z;

		sight.x = sight.x / Vector_Mag_3D((vector_3d_ptr)&sight);
		sight.y = sight.y / Vector_Mag_3D((vector_3d_ptr)&sight);
		sight.z = sight.z / Vector_Mag_3D((vector_3d_ptr)&sight);
		// compute the dot product between line of sight vector and normal to surface

		dp = Dot_Product_3D((vector_3d_ptr)&normal, (vector_3d_ptr)&sight);

		 
		if (dp > 0)
		{
			// set visibility

			the_object->polys[curr_poly].visible = 1;

			// compute light intensity if needed


			if (the_object->polys[curr_poly].shading == 1)
			{ 
				   float mag;
				 
					float normx = normal.x;
					float normy = normal.y;
					float normz = normal.z;
				 	mag = Vector_Mag_3D((vector_3d_ptr)&normal);
					// compute the dot product between the light source vector
					// and normal vector to surface
					normal.x = normx / mag;
					normal.y = normy / mag;
					normal.z = normz / mag;
				 
				 
				dp = Dot_Product_3D((vector_3d_ptr)&normal,
					(vector_3d_ptr)&light_source);

				// test if light ray is reflecting off surface
				 

				if (dp > 0)
				{
					 
					float intens ;
					uint16_t colorMod;
					int r,g,b;
					float Red, Green, Blue, result;
					intensity = ambient_light + dp;
				
					if (intensity > 1)
						intensity = 1;
					if (intensity < 0)
						intensity = 0;
					 r = the_object->polys[curr_poly].color >> 11;
					 g = (the_object->polys[curr_poly].color >>5)&0x0F;
					 b = the_object->polys[curr_poly].color & 0x0000FF;
					 Red = r*intensity;
					 Green = g*intensity;
					 Blue  = b*intensity;
					 result =  (int)(Red) << 11 | (int)(Green ) << 5 | (int)(Blue);
					 the_object->polys[curr_poly].shade = result;
					// printf("\nintensity of polygon %d is %f",curr_poly,intensity);

				} // end if light is reflecting off surface
				else{
					int r,g,b;
					float Red, Green, Blue, result;
					float colorModulate;
					uint16_t colorMod;
					 r = the_object->polys[curr_poly].color >> 11;
					 g = (the_object->polys[curr_poly].color >>5)&0x0F;
					 b = the_object->polys[curr_poly].color & 0x0000FF;
					 Red = r*ambient_light;
					 Green = g*ambient_light;
					 Blue  = b*ambient_light;
					 result =  (int)(Red) << 11 | (int)(Green ) << 5 | (int)(Blue);
					 the_object->polys[curr_poly].shade = result;
				}
			} // end if use flat shading
			else
			{
				// assume constant shading and simply assign color to shade

				the_object->polys[curr_poly].shade = color;

			} // end else constat shading

		} // end if face is visible
		else
			the_object->polys[curr_poly].visible = 0; // set invisible flag
 

	} // end for curr_poly

}

int Rotate_Polygon3D_YAxis(object_ptr poly, int theta)
{
	float theta_Rad;
	unsigned curr_vert;

	if (!poly)
		return 0;
	
  theta_Rad	= (float)theta*3.14159 / 180; // trig functions work in rad


	// loop and rotate each point, very crude, no lookup!!!
	for (curr_vert = 0; curr_vert < poly->num_vertices; curr_vert++)
	{
		// perform rotation
		float xr = (float)poly->vertices_local[curr_vert].z*sin(theta_Rad) + (float)poly->vertices_local[curr_vert].x*cos(theta_Rad);
		float yr = (float)poly->vertices_local[curr_vert].y;
		float zr = (float)poly->vertices_local[curr_vert].z*cos(theta_Rad) - (float)poly->vertices_local[curr_vert].x*sin(theta_Rad);

		// store result back
		poly->vertices_local[curr_vert].x = xr;
		poly->vertices_local[curr_vert].y = yr;
		poly->vertices_local[curr_vert].z = zr;


	} // end for curr_vert
	return 1;
}

/*************************************
Sprite Library
*/

void blit(BITMAP_PTR SrcPm,  int src_x, int src_y, int dst_x, int dst_y, int w, int h)
{
	for (int i = 0; i <  w ; i++)
        for (int j = 0 ; j < h ; j++) {
            uint16_t color = SrcPm->data[(src_y+j)*SrcPm->w + (src_x+i)];
            if (color != 0)
            {
							BSP_LCD_DrawPixel( i + dst_x, j + dst_y , color) ;
            }
        }
}

void  erase_spr(sprite_ptr spr , BITMAP_PTR src_bitmap)
{
	int x = spr->x , y = spr->y  ; 
     if(y < 0 ) y = 0;
     if(y > SCREEN_HEIGHT) y = SCREEN_HEIGHT ;
     if(x < 0 ) x = 0;
     if(x > SCREEN_WIDTH) x = SCREEN_HEIGHT ;	
     	
         blit( src_bitmap ,  x , y ,x , y, spr->width , spr->height) ;
}


void InitSprite(sprite_ptr sprite, int x, int y, int width, int height, int ac, int as, int numframes)
{
	if (!sprite)
		return;
	sprite->x = x;
	sprite->y = y;
	sprite->width = width;
	sprite->height = height;
	sprite->curr_frame = 0;
	sprite->anim_clock = ac;
	sprite->anim_speed = as;
	sprite->num_frames = numframes;
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		sprite->frames[i] = NULL;
	}

}

void Grape_frame( sprite_ptr spr , BITMAP_PTR src_bitmap  ,  int frames_col , int frame_num )
{
    int w = spr->width ;
    int h = spr->height ;
    int x0 =  (frame_num % frames_col)*w ;
    int y0 =  (frame_num / frames_col)*h ;

    //spr->frames[frame_num] =  malloc(sizeof(BITMAP));
	  //spr->frames[frame_num]->w = w ;
	  //spr->frames[frame_num]->h = h ;
    spr->frames[frame_num] = malloc(w*h*sizeof(uint16_t)) ;
    uint16_t *des_data = spr->frames[frame_num] ;
	  if(!des_data) return ;

      for(int y_src = y0 ,y_des = 0 ; y_des < h ; y_src++ , y_des++ )
    {
         for(int x_src = x0 , x_des = 0 ; x_des < w ; x_src++ , x_des++ )
        {
            des_data[x_des+y_des*w] = src_bitmap->data[x_src + y_src*src_bitmap->w] ;
        }
    }
}
 

void Update_postion(sprite_ptr spr)
{
	if(++spr->vxdelay > spr->xdelay )
    {
        spr->x += spr->xspeed ;
        spr->vxdelay = 0 ;
    }
		if(++spr->vydelay > spr->ydelay )
    {
        spr->y += spr->yspeed ;
        spr->vydelay = 0 ;
    }
		spr->yspeed += spr->yaccel ;
		
}

void Update_animation(sprite_ptr spr)
{
	if (++spr->anim_clock > spr->anim_speed)
		{
			spr->anim_clock = 0;
			spr->curr_frame++;

			if (spr->curr_frame >= (spr->num_frames -1) )
				spr->curr_frame = 0;
		}
}


void  DrawSprite(sprite_ptr spr, uint8_t *fb)
{
	  const uint16_t* data = (spr->num_frames > 1 ) ? spr->frames[spr->curr_frame] : spr->imgData  ; 
		
    for (int i = 0; i < spr->width; i++)
        for (int j = 0; j < spr->height; j++) {
            uint16_t color = data[j*spr->width + i];
            if (color != 0)
            {
							LCD_DrawPixel(fb,  i + spr->x, j + spr->y, color) ;

            }
        }
}

int collision( sprite_ptr spr1 ,  sprite_ptr spr2)
{
    if(spr1->x  > (spr2->x + spr2->width  ) ||
      spr1->y  > (spr2->y + spr2->height  ) ||
      spr2->x > (spr1->x + spr1->width  ) ||
      spr2->y > (spr1->y + spr1->height  ) )
   {
       return 0 ;
   }
   return 1 ;
}

void CopySDRam(int w , int h , uint32_t buffer_layer ,  uint32_t screen_layer  )
{
	for (int i = 0 ; i< w ; i++)
	{
		for (int j = 0 ; j < h ; j++)
		{
//			LCD_DrawPixel(screen_layer , i , j  , LCD_ReadPixel(buffer_layer , i , j)) ;
		}
	}
}

void BufferToScreen(uint16_t *buffer , int w , int h)
{
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
             {
                 BSP_LCD_DrawPixel(x , y , ((uint16_t (*)[w])buffer)[y][x] );
             }
}

 
void  DrawBitmap(const uint16_t*bitmap, int w, int h, int x, int y, float angle, uint32_t layer)
{  
	int i, j;
   
  	for (i = 0; i < w; i++)
		for (j = 0; j < h; j++) {
		// LCD_DrawPixel(layer, i + x,j + y  , bitmap[(w*j + i )]);
 			 
		}
	   
}

int fsin( short alpha)
{
    int i = 1;

    // normalize the angle
    if ( alpha < 0)
        alpha = 360 - ((-alpha) % 360);
    else
        alpha %= 360;

	if ( alpha >= 180)
	{   // sin(a+180) == - sin(a)
		alpha -= 180;
        i = -1;
    }
    if ( alpha > 90)
    { // sin(180-a) == sin (a); when a<180
        alpha = 180 - alpha;
    }

    // use the table to approximate the angle
    return ( i * tsin[ alpha]);
}

int fcos( short alpha)
{   // cos(a ) == sin (a+90)
    return fsin( alpha + 90);
}
 
