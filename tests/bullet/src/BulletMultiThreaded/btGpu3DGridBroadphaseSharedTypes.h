/*
Bullet Continuous Collision Detection and Physics Library, http://bulletphysics.org
Copyright (C) 2006, 2009 Sony Computer Entertainment Inc. 

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

//----------------------------------------------------------------------------------------

// Shared definitions for GPU-based 3D Grid collision detection broadphase

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  Keep this file free from Bullet headers
//  it is included into both CUDA and CPU code
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//----------------------------------------------------------------------------------------

#ifndef BTGPU3DGRIDBROADPHASESHAREDTYPES_H
#define BTGPU3DGRIDBROADPHASESHAREDTYPES_H

//----------------------------------------------------------------------------------------

#define BT_3DGRID_PAIR_FOUND_FLG (0x40000000)
#define BT_3DGRID_PAIR_NEW_FLG   (0x20000000)
#define BT_3DGRID_PAIR_ANY_FLG   (BT_3DGRID_PAIR_FOUND_FLG | BT_3DGRID_PAIR_NEW_FLG)

//----------------------------------------------------------------------------------------

struct bt3DGridBroadphaseParams 
{
	unsigned int	m_gridSizeX;
	unsigned int	m_gridSizeY;
	unsigned int	m_gridSizeZ;
	unsigned int	m_numCells;
	float			m_worldOriginX;
	float			m_worldOriginY;
	float			m_worldOriginZ;
	float			m_cellSizeX;
	float			m_cellSizeY;
	float			m_cellSizeZ;
	unsigned int	m_numBodies;
	unsigned int	m_maxBodiesPerCell;
};

//----------------------------------------------------------------------------------------

struct bt3DGrid3F1U
{
	float			fx;
	float			fy;
	float			fz;
	unsigned int	uw;
};

//----------------------------------------------------------------------------------------

#endif // BTGPU3DGRIDBROADPHASESHAREDTYPES_H

